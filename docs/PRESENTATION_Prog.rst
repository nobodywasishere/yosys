Writing Yosys extensions in C++
===============================

.. container:: frame

Program Components and Data Formats
-----------------------------------

.. container:: frame

   .. container:: center

Simplified RTLIL Entity-Relationship Diagram
--------------------------------------------

.. container:: frame

   Between passses and frontends/backends the design is stored in Yosys’
   internal RTLIL (RTL Intermediate Language) format. For writing Yosys
   extensions it is key to understand this format.

   .. container:: center

RTLIL without memories and processes
------------------------------------

.. container:: frame

   After the commands ``proc`` and ``memory`` (or ``memory -nomap``), we
   are left with a much simpler version of RTLIL:

   .. container:: center

   Many commands simply choose to only work on this simpler version:

   ::

      for (RTLIL::Module *module : design->selected_modules() {
          if (module->has_memories_warn() || module->has_processes_warn())
              continue;
          ....
      }

   For simplicity we only discuss this version of RTLIL in this
   presentation.

Using dump and show commands
----------------------------

.. container:: frame

   -  The ``dump`` command prints the design (or parts of it) in the
      text representation of RTLIL.

   -  The ``show`` command visualizes how the components in the design
      are connected.

   When trying to understand what a command does, create a small test
   case and look at the output of ``dump`` and ``show`` before and after
   the command has been executed.

The RTLIL Data Structures
-------------------------

.. container:: frame

   The RTLIL data structures are simple structs utilizing ``pool<>`` and
   ``dict<>`` containers (drop-in replacements for
   ``std::unordered_set<>`` and ``std::unordered_map<>``).

   -  Most operations are performed directly on the RTLIL structs
      without setter or getter functions.

   -  In debug builds a consistency checker is run over the in-memory
      design between commands to make sure that the RTLIL representation
      is intact.

   -  Most RTLIL structs have helper methods that perform the most
      common operations.

   See ``yosys/kernel/rtlil.h`` for details.

RTLIL::IdString
~~~~~~~~~~~~~~~

.. container:: frame

   ``RTLIL::IdString`` in many ways behave like a ``std::string``. It is
   used for names of RTLIL objects. Internally a RTLIL::IdString object
   is only a single integer.

   The first character of a ``RTLIL::IdString`` specifies if the name is
   *public* or *private*:

   -  | ``RTLIL::IdString[0] == ’\\’``:
      | This is a public name. Usually this means it is a name that was
        declared in a Verilog file.

   -  | ``RTLIL::IdString[0] == ’$’``:
      | This is a private name. It was assigned by Yosys.

   Use the ``NEW_ID`` macro to create a new unique private name.

RTLIL::Design and RTLIL::Module
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   The ``RTLIL::Design`` and ``RTLIL::Module`` structs are the top-level
   RTLIL data structures. Yosys always operates on one active design,
   but can hold many designs in memory.

   .. code:: c++

      struct RTLIL::Design {
          dict<RTLIL::IdString, RTLIL::Module*> modules_;
          ...
      };

      struct RTLIL::Module {
          RTLIL::IdString name;
          dict<RTLIL::IdString, RTLIL::Wire*> wires_;
          dict<RTLIL::IdString, RTLIL::Cell*> cells_;
          std::vector<RTLIL::SigSig> connections_;
          ...
      };

   (Use the various accessor functions instead of directly working with
   the ``_`` members.)

The RTLIL::Wire Structure
~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Each wire in the design is represented by a ``RTLIL::Wire`` struct:

   .. code:: c++

      struct RTLIL::Wire {
          RTLIL::IdString name;
          int width, start_offset, port_id;
          bool port_input, port_output;
          ...
      };

   ================ ================================================
   ``width``        The total number of bits. E.g. 10 for ``[9:0]``.
   ``start_offset`` The lowest bit index. E.g. 3 for ``[5:3]``.
   ``port_id``      Zero for non-ports. Positive index for ports.
   ``port_input``   True for ``input`` and ``inout`` ports.
   ``port_output``  True for ``output`` and ``inout`` ports.
   ================ ================================================

RTLIL::State and RTLIL::Const
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   The ``RTLIL::State`` enum represents a simple 1-bit logic level:

   .. code:: c++

      enum RTLIL::State {
          S0 = 0,
          S1 = 1,
          Sx = 2, // undefined value or conflict
          Sz = 3, // high-impedance / not-connected
          Sa = 4, // don't care (used only in cases)
          Sm = 5  // marker (used internally by some passes)
      };

   The ``RTLIL::Const`` struct represents a constant multi-bit value:

   .. code:: c++

      struct RTLIL::Const {
          std::vector<RTLIL::State> bits;
          ...
      };

   Notice that Yosys is not using special ``VCC`` or ``GND`` driver
   cells to represent constants. Instead constants are part of the RTLIL
   representation itself.

The RTLIL::SigSpec Structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   The ``RTLIL::SigSpec`` struct represents a signal vector. Each bit
   can either be a bit from a wire or a constant value.

   .. code:: c++

      struct RTLIL::SigBit
      {
          RTLIL::Wire *wire;
          union {
              RTLIL::State data; // used if wire == NULL
              int offset;        // used if wire != NULL
          };
          ...
      };

      struct RTLIL::SigSpec {
          std::vector<RTLIL::SigBit> bits_; // LSB at index 0
          ...
      };

   The ``RTLIL::SigSpec`` struct has a ton of additional helper methods
   to compare, analyze, and manipulate instances of ``RTLIL::SigSpec``.

The RTLIL::Cell Structure
~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   (1/2) The ``RTLIL::Cell`` struct represents an instance of a module
   or library cell.

   The ports of the cell are associated with ``RTLIL::SigSpec``
   instances and the parameters are associated with ``RTLIL::Const``
   instances:

   .. code:: c++

      struct RTLIL::Cell {
          RTLIL::IdString name, type;
          dict<RTLIL::IdString, RTLIL::SigSpec> connections_;
          dict<RTLIL::IdString, RTLIL::Const> parameters;
          ...
      };

   The ``type`` may refer to another module in the same design, a cell
   name from a cell library, or a cell name from the internal cell
   library:

   ::

      $not $pos $neg $and $or $xor $xnor $reduce_and $reduce_or $reduce_xor $reduce_xnor
      $reduce_bool $shl $shr $sshl $sshr $lt $le $eq $ne $eqx $nex $ge $gt $add $sub $mul $div $mod
      $divfloor $modfloor $pow $logic_not $logic_and $logic_or $mux $pmux $slice $concat $lut $assert $sr $dff
      $dffsr $adff $dlatch $dlatchsr $memrd $memwr $mem $fsm $_NOT_ $_AND_ $_OR_ $_XOR_ $_MUX_ $_SR_NN_
      $_SR_NP_ $_SR_PN_ $_SR_PP_ $_DFF_N_ $_DFF_P_ $_DFF_NN0_ $_DFF_NN1_ $_DFF_NP0_ $_DFF_NP1_ $_DFF_PN0_
      $_DFF_PN1_ $_DFF_PP0_ $_DFF_PP1_ $_DFFSR_NNN_ $_DFFSR_NNP_ $_DFFSR_NPN_ $_DFFSR_NPP_ $_DFFSR_PNN_
      $_DFFSR_PNP_ $_DFFSR_PPN_ $_DFFSR_PPP_ $_DLATCH_N_ $_DLATCH_P_ $_DLATCHSR_NNN_ $_DLATCHSR_NNP_
      $_DLATCHSR_NPN_ $_DLATCHSR_NPP_ $_DLATCHSR_PNN_ $_DLATCHSR_PNP_ $_DLATCHSR_PPN_ $_DLATCHSR_PPP_

.. container:: frame

   (2/2) Simulation models (i.e. *documentation* :-) for the internal
   cell library:

   | 2em ``yosys/techlibs/common/simlib.v`` and
   | 2em ``yosys/techlibs/common/simcells.v``

   The lower-case cell types (such as ``$and``) are parameterized cells
   of variable width. This so-called *RTL Cells* are the cells described
   in ``simlib.v``.

   The upper-case cell types (such as ``$_AND_``) are single-bit cells
   that are not parameterized. This so-called *Internal Logic Gates* are
   the cells described in ``simcells.v``.

   The consistency checker also checks the interfaces to the internal
   cell library. If you want to use private cell types for your own
   purposes, use the ``$__``-prefix to avoid name collisions.

Connecting wires or constant drivers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Additional connections between wires or between wires and constants
   are modelled using ``RTLIL::Module::connections``:

   .. code:: c++

      typedef std::pair<RTLIL::SigSpec, RTLIL::SigSpec> RTLIL::SigSig;

      struct RTLIL::Module {
          ...
          std::vector<RTLIL::SigSig> connections_;
          ...
      };

   ``RTLIL::SigSig::first`` is the driven signal and
   ``RTLIL::SigSig::second`` is the driving signal. Example usage
   (setting wire ``foo`` to value ``42``):

   .. code:: c++

      module->connect(module->wire("\\foo"),
                      RTLIL::SigSpec(42, module->wire("\\foo")->width));

Creating modules from scratch
-----------------------------

.. container:: frame

   Let’s create the following module using the RTLIL API:

   .. code:: verilog

      module absval(input signed [3:0] a, output [3:0] y);
          assign y = a[3] ? -a : a;
      endmodule

   .. code:: c++

      RTLIL::Module *module = new RTLIL::Module;
      module->name = "\\absval";

      RTLIL::Wire *a = module->addWire("\\a", 4);
      a->port_input = true;
      a->port_id = 1;

      RTLIL::Wire *y = module->addWire("\\y", 4);
      y->port_output = true;
      y->port_id = 2;

      RTLIL::Wire *a_inv = module->addWire(NEW_ID, 4);
      module->addNeg(NEW_ID, a, a_inv, true);
      module->addMux(NEW_ID, a, a_inv, RTLIL::SigSpec(a, 1, 3), y);

      module->fixup_ports();

Modifying modules
-----------------

.. container:: frame

   Most commands modify existing modules, not create new ones.

   When modifying existing modules, stick to the following DOs and
   DON’Ts:

   -  Do not remove wires. Simply disconnect them and let a successive
      ``clean`` command worry about removing it.

   -  Use ``module->fixup_ports()`` after changing the ``port_``
      properties of wires.

   -  You can safely remove cells or change the ``connections`` property
      of a cell, but be careful when changing the size of the
      ``SigSpec`` connected to a cell port.

   -  Use the ``SigMap`` helper class (see next slide) when you need a
      unique handle for each signal bit.

Using the SigMap helper class
-----------------------------

.. container:: frame

   Consider the following module:

   .. code:: verilog

      module test(input a, output x, y);
          assign x = a, y = a;
      endmodule

   In this case ``a``, ``x``, and ``y`` are all different names for the
   same signal. However:

   .. code:: c++

      RTLIL::SigSpec a(module->wire("\\a")), x(module->wire("\\x")),
                                             y(module->wire("\\y"));
      log("%d %d %d\n", a == x, x == y, y == a); // will print "0 0 0"

   The ``SigMap`` helper class can be used to map all such aliasing
   signals to a unique signal from the group (usually the wire that is
   directly driven by a cell or port).

   .. code:: c++

      SigMap sigmap(module);
      log("%d %d %d\n", sigmap(a) == sigmap(x), sigmap(x) == sigmap(y),
                        sigmap(y) == sigmap(a)); // will print "1 1 1"

Printing log messages
---------------------

.. container:: frame

   The ``log()`` function is a ``printf()``-like function that can be
   used to create log messages.

   Use ``log_signal()`` to create a C-string for a SigSpec object [1]_:

   .. code:: c++

      log("Mapped signal x: %s\n", log_signal(sigmap(x)));

   Use ``log_id()`` to create a C-string for an ``RTLIL::IdString``:

   .. code:: c++

      log("Name of this module: %s\n", log_id(module->name));

   Use ``log_header()`` and ``log_push()``/``log_pop()`` to structure
   log messages:

   .. code:: c++

      log_header(design, "Doing important stuff!\n");
      log_push();
      for (int i = 0; i < 10; i++)
          log("Log message #%d.\n", i);
      log_pop();

Error handling
--------------

.. container:: frame

   Use ``log_error()`` to report a non-recoverable error:

   .. code:: c++

      if (design->modules.count(module->name) != 0)
          log_error("A module with the name %s already exists!\n",
                  RTLIL::id2cstr(module->name));

   Use ``log_cmd_error()`` to report a recoverable error:

   .. code:: c++

      if (design->selection_stack.back().empty())
          log_cmd_error("This command can't operator on an empty selection!\n");

   Use ``log_assert()`` and ``log_abort()`` instead of ``assert()`` and
   ``abort()``.

Creating a command
------------------

.. container:: frame

   Simply create a global instance of a class derived from ``Pass`` to
   create a new yosys command:

   .. code:: c++

      #include "kernel/yosys.h"
      USING_YOSYS_NAMESPACE

      struct MyPass : public Pass {
          MyPass() : Pass("my_cmd", "just a simple test") { }
          virtual void execute(std::vector<std::string> args, RTLIL::Design *design)
          {
              log("Arguments to my_cmd:\n");
              for (auto &arg : args)
                  log("  %s\n", arg.c_str());

              log("Modules in current design:\n");
              for (auto mod : design->modules())
                  log("  %s (%d wires, %d cells)\n", log_id(mod),
                          GetSize(mod->wires()), GetSize(mod->cells()));
          }
      } MyPass;

Creating a plugin
-----------------

.. container:: frame

   Yosys can be extended by adding additional C++ code to the Yosys code
   base, or by loading plugins into Yosys.

   Use the following command to compile a Yosys plugin:

   ::

      yosys-config --exec --cxx --cxxflags --ldflags \
                   -o my_cmd.so -shared my_cmd.cc --ldlibs

   Or shorter:

   ::

      yosys-config --build my_cmd.so my_cmd.cc

   Load the plugin using the yosys ``-m`` option:

   ::

      yosys -m ./my_cmd.so -p 'my_cmd foo bar'

Summary
-------

.. container:: frame

   -  Writing Yosys extensions is very straight-forward.

   -  …and even simpler if you don’t need RTLIL::Memory or
      RTLIL::Process objects.

   -  Writing synthesis software? Consider learning the Yosys API and
      make your work part of the Yosys framework.

   .. container:: center

      Questions?

   .. container:: center

      http://www.clifford.at/yosys/

.. [1]
   The pointer returned by ``log_signal()`` is automatically freed by
   the log framework at a later time.
