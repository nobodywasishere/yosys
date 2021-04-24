Yosys by example – Advanced Synthesis
=====================================

.. container:: frame

.. container:: frame

   Overview This section contains 4 subsections:

   -  Using selections

   -  Advanced uses of techmap

   -  Coarse-grain synthesis

   -  Automatic design changes

Using selections
----------------

.. container:: frame

Simple selections
~~~~~~~~~~~~~~~~~

.. container:: frame

   Most Yosys commands make use of the “selection framework” of Yosys.
   It can be used to apply commands only to part of the design. For
   example:

   .. code:: ys

      delete                # will delete the whole design, but

      delete foobar         # will only delete the module foobar.

   The ``select`` command can be used to create a selection for
   subsequent commands. For example:

   .. code:: ys

      select foobar         # select the module foobar
      delete                # delete selected objects
      select -clear         # reset selection (select whole design)

Selection by object name
~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   The easiest way to select objects is by object name. This is usually
   only done in synthesis scripts that are hand-tailored for a specific
   design.

   .. code:: ys

      select foobar         # select module foobar
      select foo*           # select all modules whose names start with foo
      select foo*/bar*      # select all objects matching bar* from modules matching foo*
      select */clk          # select objects named clk from all modules

Module and design context
~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Commands can be executed in *module* or *design* context. Until now
   all commands have been executed in design context. The ``cd`` command
   can be used to switch to module context.

   In module context all commands only effect the active module. Objects
   in the module are selected without the ``<module_name>/`` prefix. For
   example:

   .. code:: ys

      cd foo                # switch to module foo
      delete bar            # delete object foo/bar

      cd mycpu              # switch to module mycpu
      dump reg_*            # print details on all objects whose names start with reg_

      cd ..                 # switch back to design

   Note: Most synthesis scripts never switch to module context. But it
   is a very powerful tool for interactive design investigation.

Selecting by object property or type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Special patterns can be used to select by object property or type.
   For example:

   .. code:: ys

      select w:reg_*        # select all wires whose names start with reg_
      select a:foobar       # select all objects with the attribute foobar set
      select a:foobar=42    # select all objects with the attribute foobar set to 42
      select A:blabla       # select all modules with the attribute blabla set
      select foo/t:$add     # select all $add cells from the module foo

   A complete list of this pattern expressions can be found in the
   command reference to the ``select`` command.

Combining selection
~~~~~~~~~~~~~~~~~~~

.. container:: frame

   When more than one selection expression is used in one statement,
   then they are pushed on a stack. The final elements on the stack are
   combined into a union:

   .. code:: ys

      select t:$dff r:WIDTH>1     # all cells of type $dff and/or with a parameter WIDTH > 1

   Special %-commands can be used to combine the elements on the stack:

   .. code:: ys

      select t:$dff r:WIDTH>1 %i  # all cells of type $dff *AND* with a parameter WIDTH > 1

   .. container:: block

      | Examples for ``%``-codes (see ``help select`` for full list)
        ``%u`` union of top two elements on stack – pop 2, push 1
      | ``%d`` difference of top two elements on stack – pop 2, push 1
      | ``%i`` intersection of top two elements on stack – pop 2, push 1
      | ``%n`` inverse of top element on stack – pop 1, push 1

Expanding selections
~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Selections of cells and wires can be expanded along connections using
   ``%``-codes for selecting input cones (``%ci``), output cones
   (``%co``), or both (``%x``).

   .. code:: ys

      # select all wires that are inputs to $add cells
      select t:$add %ci w:* %i

   Additional constraints such as port names can be specified.

   .. code:: ys

      # select all wires that connect a "Q" output with a "D" input
      select c:* %co:+[Q] w:* %i c:* %ci:+[D] w:* %i %i

      # select the multiplexer tree that drives the signal 'state'
      select state %ci*:+$mux,$pmux[A,B,Y]

   See ``help select`` for full documentation of this expressions.

Incremental selection
~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Sometimes a selection can most easily be described by a series of
   add/delete operations. The commands ``select -add`` and
   ``select -del`` respectively add or remove objects from the current
   selection instead of overwriting it.

   .. code:: ys

      select -none            # start with an empty selection
      select -add reg_*       # select a bunch of objects
      select -del reg_42      # but not this one
      select -add state %ci   # and add mor stuff

   Within a select expression the token ``%`` can be used to push the
   previous selection on the stack.

   .. code:: ys

      select t:$add t:$sub    # select all $add and $sub cells
      select % %ci % %d       # select only the input wires to those cells

Creating selection variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Selections can be stored under a name with the ``select -set <name>``
   command. The stored selections can be used in later select
   expressions using the syntax ``@<name>``.

   .. code:: ys

      select -set cone_a state_a %ci*:-$dff  # set @cone_a to the input cone of state_a
      select -set cone_b state_b %ci*:-$dff  # set @cone_b to the input cone of state_b
      select @cone_a @cone_b %i              # select the objects that are in both cones

   Remember that select expressions can also be used directly as
   arguments to most commands. Some commands also except a single select
   argument to some options. In those cases selection variables must be
   used to capture more complex selections.

   .. code:: ys

      dump @cone_a @cone_b

      select -set cone_ab @cone_a @cone_b %i
      show -color red @cone_ab -color magenta @cone_a -color blue @cone_b

.. container:: frame

   – Example

   .. container:: columns

      .. code:: verilog

         module test(clk, s, a, y);
             input clk, s;
             input [15:0] a;
             output [15:0] y;
             reg [15:0] b, c;

             always @(posedge clk) begin
                 b <= a;
                 c <= b;
             end

             wire [15:0] state_a = (a ^ b) + c;
             wire [15:0] state_b = (a ^ b) - c;
             assign y = !s ? state_a : state_b;
         endmodule

      .. code:: ys

         read_verilog select.v
         hierarchy -check -top test
         proc; opt
         cd test
         select -set cone_a state_a %ci*:-$dff
         select -set cone_b state_b %ci*:-$dff
         select -set cone_ab @cone_a @cone_b %i
         show -prefix select -format pdf -notitle \
              -color red @cone_ab -color magenta @cone_a \
              -color blue @cone_b

   .. image:: PRESENTATION_ExAdv/select.pdf
      :alt: image

Advanced uses of techmap
------------------------

.. container:: frame

Introduction to techmap
~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  The ``techmap`` command replaces cells in the design with
      implementations given as Verilog code (called “map files”). It can
      replace Yosys’ internal cell types (such as ``$or``) as well as
      user-defined cell types.

   -  Verilog parameters are used extensively to customize the internal
      cell types.

   -  Additional special parameters are used by techmap to communicate
      meta-data to the map files.

   -  Special wires are used to instruct techmap how to handle a module
      in the map file.

   -  Generate blocks and recursion are powerful tools for writing map
      files.

.. container:: frame

   – Example 1/2 To map the Verilog OR-reduction operator to 3-input OR
   gates:

   .. container:: columns

      .. code:: verilog

         module \$reduce_or (A, Y);

             parameter A_SIGNED = 0;
             parameter A_WIDTH = 0;
             parameter Y_WIDTH = 0;

             input [A_WIDTH-1:0] A;
             output [Y_WIDTH-1:0] Y;

             function integer min;
                 input integer a, b;
                 begin
                     if (a < b)
                         min = a;
                     else
                         min = b;
                 end
             endfunction

             genvar i;
             generate begin
                 if (A_WIDTH == 0) begin
                     assign Y = 0;
                 end

      .. code:: verilog

                 if (A_WIDTH == 1) begin
                     assign Y = A;
                 end
                 if (A_WIDTH == 2) begin
                     wire ybuf;
                     OR3X1 g (.A(A[0]), .B(A[1]), .C(1'b0), .Y(ybuf));
                     assign Y = ybuf;
                 end
                 if (A_WIDTH == 3) begin
                     wire ybuf;
                     OR3X1 g (.A(A[0]), .B(A[1]), .C(A[2]), .Y(ybuf));
                     assign Y = ybuf;
                 end
                 if (A_WIDTH > 3) begin
                     localparam next_stage_sz = (A_WIDTH+2) / 3;
                     wire [next_stage_sz-1:0] next_stage;
                     for (i = 0; i < next_stage_sz; i = i+1) begin
                         localparam bits = min(A_WIDTH - 3*i, 3);
                         assign next_stage[i] = |A[3*i +: bits];
                     end
                     assign Y = |next_stage;
                 end
             end endgenerate
         endmodule

.. container:: frame

   – Example 2/2 to 0cm\ |image|

   .. container:: columns

      .. code:: ys

         techmap -map red_or3x1_map.v;;

      .. code:: verilog

         module test (A, Y);
             input [6:0] A;
             output Y;
             assign Y = |A;
         endmodule

Conditional techmap
~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  In some cases only cells with certain properties should be
      substituted.

   -  The special wire ``_TECHMAP_FAIL_`` can be used to disable a
      module in the map file for a certain set of parameters.

   -  The wire ``_TECHMAP_FAIL_`` must be set to a constant value. If it
      is non-zero then the module is disabled for this set of
      parameters.

   -  Example use-cases:

      -  coarse-grain cell types that only operate on certain bit widths

      -  memory resources for different memory geometries (width, depth,
         ports, etc.)

.. container:: frame

   – Example to 0cm\ |image1|

   .. code:: verilog

      module \$mul (A, B, Y);
          parameter A_SIGNED = 0;
          parameter B_SIGNED = 0;
          parameter A_WIDTH = 1;
          parameter B_WIDTH = 1;
          parameter Y_WIDTH = 1;

          input [A_WIDTH-1:0] A;
          input [B_WIDTH-1:0] B;
          output [Y_WIDTH-1:0] Y;

          wire _TECHMAP_FAIL_ = A_WIDTH != B_WIDTH || B_WIDTH != Y_WIDTH;

          MYMUL #( .WIDTH(Y_WIDTH) ) g ( .A(A), .B(B), .Y(Y) );
      endmodule

   .. container:: columns

      .. code:: verilog

         module test(A, B, C, Y1, Y2);
             input   [7:0] A, B, C;
             output  [7:0] Y1 = A * B;
             output [15:0] Y2 = A * C;
         endmodule

      .. code:: ys

         read_verilog sym_mul_test.v
         hierarchy -check -top test

         techmap -map sym_mul_map.v;;

Scripting in map modules
~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  The special wires ``_TECHMAP_DO_`` can be used to run Yosys
      scripts in the context of the replacement module.

   -  The wire that comes first in alphabetical oder is interpreted as
      string (must be connected to constants) that is executed as
      script. Then the wire is removed. Repeat.

   -  You can even call techmap recursively!

   -  Example use-cases:

      -  Using always blocks in map module: call ``proc``

      -  Perform expensive optimizations (such as ``freduce``) on cells
         where this is known to work well.

      -  Interacting with custom commands.

   PROTIP: Commands such as ``shell``, ``show -pause``, and ``dump`` can
   be use in the ``_TECHMAP_DO_`` scripts for debugging map modules.

.. container:: frame

   – Example to 0cm\ |image2|

   .. container:: columns

      .. code:: verilog

         module MYMUL(A, B, Y);
             parameter WIDTH = 1;
             input [WIDTH-1:0] A, B;
             output reg [WIDTH-1:0] Y;

             wire [1023:0] _TECHMAP_DO_ = "proc; clean";

             integer i;
             always @* begin
                 Y = 0;
                 for (i = 0; i < WIDTH; i=i+1)
                     if (A[i])
                         Y = Y + (B << i);
             end
         endmodule

      .. code:: verilog

         module test(A, B, Y);
             input  [1:0] A, B;
             output [1:0] Y = A * B;
         endmodule

      .. code:: ys

         read_verilog mymul_test.v
         hierarchy -check -top test

         techmap -map sym_mul_map.v \
                 -map mymul_map.v;;

      .. code:: ys

         rename test test_mapped
         read_verilog mymul_test.v
         miter -equiv test test_mapped miter
         flatten miter

         sat -verify -prove trigger 0 miter

Handling constant inputs
~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  The special parameters ``_TECHMAP_CONSTMSK__`` and
      ``_TECHMAP_CONSTVAL__`` can be used to handle constant input
      values to cells.

   -  The former contains 1-bits for all constant input bits on the
      port.

   -  The latter contains the constant bits or undef (x) for
      non-constant bits.

   -  Example use-cases:

      -  Converting arithmetic (for example multiply to shift)

      -  Identify constant addresses or enable bits in memory
         interfaces.

.. container:: frame

   – Example to 0cm\ |image3|

   .. container:: columns

      .. code:: verilog

         module MYMUL(A, B, Y);
             parameter WIDTH = 1;
             input [WIDTH-1:0] A, B;
             output reg [WIDTH-1:0] Y;

             parameter _TECHMAP_CONSTVAL_A_ = WIDTH'bx;
             parameter _TECHMAP_CONSTVAL_B_ = WIDTH'bx;

             reg _TECHMAP_FAIL_;
             wire [1023:0] _TECHMAP_DO_ = "proc; clean";

             integer i;
             always @* begin
             	_TECHMAP_FAIL_ <= 1;
                 for (i = 0; i < WIDTH; i=i+1) begin
                     if (_TECHMAP_CONSTVAL_A_ === WIDTH'd1 << i) begin
         	        _TECHMAP_FAIL_ <= 0;
                         Y <= B << i;
         	    end
                     if (_TECHMAP_CONSTVAL_B_ === WIDTH'd1 << i) begin
         	        _TECHMAP_FAIL_ <= 0;
                         Y <= A << i;
         	    end
         	end
             end
         endmodule

      .. code:: verilog

         module test (A, X, Y);
         input [7:0] A;
         output [7:0] X = A * 8'd 6;
         output [7:0] Y = A * 8'd 8;
         endmodule

      .. code:: ys

         read_verilog mulshift_test.v
         hierarchy -check -top test

         techmap -map sym_mul_map.v \
                 -map mulshift_map.v;;

Handling shorted inputs
~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  The special parameters ``_TECHMAP_BITS_CONNMAP_`` and
      ``_TECHMAP_CONNMAP__`` can be used to handle shorted inputs.

   -  Each bit of the port correlates to an ``_TECHMAP_BITS_CONNMAP_``
      bits wide number in ``_TECHMAP_CONNMAP__``.

   -  Each unique signal bit is assigned its own number. Identical
      fields in the ``_TECHMAP_CONNMAP__`` parameters mean shorted
      signal bits.

   -  The numbers 0-3 are reserved for ``0``, ``1``, ``x``, and ``z``
      respectively.

   -  Example use-cases:

      -  Detecting shared clock or control signals in memory interfaces.

      -  In some cases this can be used for for optimization.

.. container:: frame

   – Example to 0cm\ |image4|

   .. container:: columns

      .. code:: verilog

         module \$add (A, B, Y);
           parameter A_SIGNED = 0;
           parameter B_SIGNED = 0;
           parameter A_WIDTH = 1;
           parameter B_WIDTH = 1;
           parameter Y_WIDTH = 1;

           input [A_WIDTH-1:0] A;
           input [B_WIDTH-1:0] B;
           output [Y_WIDTH-1:0] Y;

           parameter _TECHMAP_BITS_CONNMAP_ = 0;
           parameter _TECHMAP_CONNMAP_A_ = 0;
           parameter _TECHMAP_CONNMAP_B_ = 0;

           wire _TECHMAP_FAIL_ = A_WIDTH != B_WIDTH || B_WIDTH < Y_WIDTH ||
                                 _TECHMAP_CONNMAP_A_ != _TECHMAP_CONNMAP_B_;

           assign Y = A << 1;
         endmodule

      .. code:: verilog

         module test (A, B, X, Y);
         input [7:0] A, B;
         output [7:0] X = A + B;
         output [7:0] Y = A + A;
         endmodule

      .. code:: ys

         read_verilog addshift_test.v
         hierarchy -check -top test

         techmap -map addshift_map.v;;

Notes on using techmap
~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   -  Don’t use positional cell parameters in map modules.

   -  | Don’t try to implement basic logic optimization with techmap.
      | (So the OR-reduce using OR3X1 cells map was actually a bad
        example.)

   -  You can use the ``$_ _``-prefix for internal cell types to avoid
      collisions with the user-namespace. But always use two underscores
      or the internal consistency checker will trigger on this cells.

   -  Techmap has two major use cases:

      -  | Creating good logic-level representation of arithmetic
           functions.
         | This also means using dedicated hardware resources such as
           half- and full-adder cells in ASICS or dedicated carry logic
           in FPGAs.

      -  Mapping of coarse-grain resources such as block memory or DSP
         cells.

Coarse-grain synthesis
----------------------

.. container:: frame

Intro to coarse-grain synthesis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   In coarse-grain synthesis the target architecture has cells of the
   same complexity or larger complexity than the internal RTL
   representation.

   For example:

   .. code:: verilog

      wire [15:0] a, b;
          wire [31:0] c, y;
          assign y = a * b + c;

   This circuit contains two cells in the RTL representation: one
   multiplier and one adder. In some architectures this circuit can be
   implemented using a single circuit element, for example an FPGA DSP
   core. Coarse grain synthesis is this mapping of groups of circuit
   elements to larger components.

   Fine-grain synthesis would be matching the circuit elements to
   smaller components, such as LUTs, gates, or half- and full-adders.

The extract pass
~~~~~~~~~~~~~~~~

.. container:: frame

   -  Like the ``techmap`` pass, the ``extract`` pass is called with a
      map file. It compares the circuits inside the modules of the map
      file with the design and looks for sub-circuits in the design that
      match any of the modules in the map file.

   -  If a match is found, the ``extract`` pass will replace the
      matching subcircuit with an instance of the module from the map
      file.

   -  In a way the ``extract`` pass is the inverse of the techmap pass.

.. container:: frame

   – Example 1/2 to 0cm

   .. container:: columns

      .. code:: verilog

         module test(a, b, c, d, y);
         input [15:0] a, b;
         input [31:0] c, d;
         output [31:0] y;
         assign y = a * b + c + d;
         endmodule

      .. code:: verilog

         module macc_16_16_32(a, b, c, y);
         input [15:0] a, b;
         input [31:0] c;
         output [31:0] y;
         assign y = a*b + c;
         endmodule

      .. code:: ys

         read_verilog macc_simple_test.v
         hierarchy -check -top test

         extract -map macc_simple_xmap.v;;

.. container:: frame

   – Example 2/2

   ================== ==================
   \                  
   :math:`\downarrow` :math:`\downarrow`
   \                  
   :math:`\downarrow` :math:`\downarrow`
   \                  
   ================== ==================

The wrap-extract-unwrap method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Often a coarse-grain element has a constant bit-width, but can be
   used to implement operations with a smaller bit-width. For example, a
   18x25-bit multiplier can also be used to implement 16x20-bit
   multiplication.

   A way of mapping such elements in coarse grain synthesis is the
   wrap-extract-unwrap method:

   -  | **wrap**
      | Identify candidate-cells in the circuit and wrap them in a cell
        with a constant wider bit-width using ``techmap``. The wrappers
        use the same parameters as the original cell, so the information
        about the original width of the ports is preserved.
      | Then use the ``connwrappers`` command to connect up the
        bit-extended in- and outputs of the wrapper cells.

   -  | **extract**
      | Now all operations are encoded using the same bit-width as the
        coarse grain element. The ``extract`` command can be used to
        replace circuits with cells of the target architecture.

   -  | **unwrap**
      | The remaining wrapper cell can be unwrapped using ``techmap``.

   The following sides detail an example that shows how to map MACC
   operations of arbitrary size to MACC cells with a 18x25-bit
   multiplier and a 48-bit adder (such as the Xilinx DSP48 cells).

Example: DSP48_MACC
~~~~~~~~~~~~~~~~~~~

.. container:: frame

   | – 1/13 Preconditioning: ``macc_xilinx_swap_map.v``
   | Make sure ``A`` is the smaller port on all multipliers

   .. container:: columns

      .. code:: verilog

         (* techmap_celltype = "$mul" *)
         module mul_swap_ports (A, B, Y);

         parameter A_SIGNED = 0;
         parameter B_SIGNED = 0;
         parameter A_WIDTH = 1;
         parameter B_WIDTH = 1;
         parameter Y_WIDTH = 1;

         input [A_WIDTH-1:0] A;
         input [B_WIDTH-1:0] B;
         output [Y_WIDTH-1:0] Y;

         wire _TECHMAP_FAIL_ = A_WIDTH <= B_WIDTH;

      .. code:: verilog

         \$mul #(
         	.A_SIGNED(B_SIGNED),
         	.B_SIGNED(A_SIGNED),
         	.A_WIDTH(B_WIDTH),
         	.B_WIDTH(A_WIDTH),
         	.Y_WIDTH(Y_WIDTH)
         ) _TECHMAP_REPLACE_ (
         	.A(B),
         	.B(A),
         	.Y(Y)
         );

         endmodule

.. container:: frame

   – 2/13 Wrapping multipliers: ``macc_xilinx_wrap_map.v``

   .. container:: columns

      .. code:: verilog

         (* techmap_celltype = "$mul" *)
         module mul_wrap (A, B, Y);

         parameter A_SIGNED = 0;
         parameter B_SIGNED = 0;
         parameter A_WIDTH = 1;
         parameter B_WIDTH = 1;
         parameter Y_WIDTH = 1;

         input [A_WIDTH-1:0] A;
         input [B_WIDTH-1:0] B;
         output [Y_WIDTH-1:0] Y;

         wire [17:0] A_18 = A;
         wire [24:0] B_25 = B;
         wire [47:0] Y_48;
         assign Y = Y_48;

         wire [1023:0] _TECHMAP_DO_ = "proc; clean";

         reg _TECHMAP_FAIL_;
         initial begin
         	_TECHMAP_FAIL_ <= 0;

      .. code:: verilog

         	if (A_SIGNED || B_SIGNED)
         		_TECHMAP_FAIL_ <= 1;
         	if (A_WIDTH < 4 || B_WIDTH < 4)
         		_TECHMAP_FAIL_ <= 1;
         	if (A_WIDTH > 18 || B_WIDTH > 25)
         		_TECHMAP_FAIL_ <= 1;
         	if (A_WIDTH*B_WIDTH < 100)
         		_TECHMAP_FAIL_ <= 1;
         end

         \$__mul_wrapper #(
         	.A_SIGNED(A_SIGNED),
         	.B_SIGNED(B_SIGNED),
         	.A_WIDTH(A_WIDTH),
         	.B_WIDTH(B_WIDTH),
         	.Y_WIDTH(Y_WIDTH)
         ) _TECHMAP_REPLACE_ (
         	.A(A_18),
         	.B(B_25),
         	.Y(Y_48)
         );

         endmodule

.. container:: frame

   – 3/13 Wrapping adders: ``macc_xilinx_wrap_map.v``

   .. container:: columns

      .. code:: verilog

         (* techmap_celltype = "$add" *)
         module add_wrap (A, B, Y);

         parameter A_SIGNED = 0;
         parameter B_SIGNED = 0;
         parameter A_WIDTH = 1;
         parameter B_WIDTH = 1;
         parameter Y_WIDTH = 1;

         input [A_WIDTH-1:0] A;
         input [B_WIDTH-1:0] B;
         output [Y_WIDTH-1:0] Y;

         wire [47:0] A_48 = A;
         wire [47:0] B_48 = B;
         wire [47:0] Y_48;
         assign Y = Y_48;

         wire [1023:0] _TECHMAP_DO_ = "proc; clean";

      .. code:: verilog

         reg _TECHMAP_FAIL_;
         initial begin
         	_TECHMAP_FAIL_ <= 0;
         	if (A_SIGNED || B_SIGNED)
         		_TECHMAP_FAIL_ <= 1;
         	if (A_WIDTH < 10 && B_WIDTH < 10)
         		_TECHMAP_FAIL_ <= 1;
         end

         \$__add_wrapper #(
         	.A_SIGNED(A_SIGNED),
         	.B_SIGNED(B_SIGNED),
         	.A_WIDTH(A_WIDTH),
         	.B_WIDTH(B_WIDTH),
         	.Y_WIDTH(Y_WIDTH)
         ) _TECHMAP_REPLACE_ (
         	.A(A_48),
         	.B(B_48),
         	.Y(Y_48)
         );

         endmodule

.. container:: frame

   – 4/13 Extract: ``macc_xilinx_xmap.v``

   .. code:: verilog

      module DSP48_MACC (a, b, c, y);

      input [17:0] a;
      input [24:0] b;
      input [47:0] c;
      output [47:0] y;

      assign y = a*b + c;

      endmodule

   .. simply use the same wrapping commands on this module as on the
   design to create a template for the ``extract`` command.

.. container:: frame

   – 5/13 Unwrapping multipliers: ``macc_xilinx_unwrap_map.v``

   .. container:: columns

      .. code:: verilog

         module \$__mul_wrapper (A, B, Y);

         parameter A_SIGNED = 0;
         parameter B_SIGNED = 0;
         parameter A_WIDTH = 1;
         parameter B_WIDTH = 1;
         parameter Y_WIDTH = 1;

         input [17:0] A;
         input [24:0] B;
         output [47:0] Y;

         wire [A_WIDTH-1:0] A_ORIG = A;
         wire [B_WIDTH-1:0] B_ORIG = B;
         wire [Y_WIDTH-1:0] Y_ORIG;
         assign Y = Y_ORIG;

      .. code:: verilog

         \$mul #(
         	.A_SIGNED(A_SIGNED),
         	.B_SIGNED(B_SIGNED),
         	.A_WIDTH(A_WIDTH),
         	.B_WIDTH(B_WIDTH),
         	.Y_WIDTH(Y_WIDTH)
         ) _TECHMAP_REPLACE_ (
         	.A(A_ORIG),
         	.B(B_ORIG),
         	.Y(Y_ORIG)
         );

         endmodule

.. container:: frame

   – 6/13 Unwrapping adders: ``macc_xilinx_unwrap_map.v``

   .. container:: columns

      .. code:: verilog

         module \$__add_wrapper (A, B, Y);

         parameter A_SIGNED = 0;
         parameter B_SIGNED = 0;
         parameter A_WIDTH = 1;
         parameter B_WIDTH = 1;
         parameter Y_WIDTH = 1;

         input [47:0] A;
         input [47:0] B;
         output [47:0] Y;

         wire [A_WIDTH-1:0] A_ORIG = A;
         wire [B_WIDTH-1:0] B_ORIG = B;
         wire [Y_WIDTH-1:0] Y_ORIG;
         assign Y = Y_ORIG;

      .. code:: verilog

         \$add #(
         	.A_SIGNED(A_SIGNED),
         	.B_SIGNED(B_SIGNED),
         	.A_WIDTH(A_WIDTH),
         	.B_WIDTH(B_WIDTH),
         	.Y_WIDTH(Y_WIDTH)
         ) _TECHMAP_REPLACE_ (
         	.A(A_ORIG),
         	.B(B_ORIG),
         	.Y(Y_ORIG)
         );

         endmodule

.. container:: frame

   – 7/13

   ================== ==================
   ``test1``          ``test2``
   \                  
   :math:`\downarrow` :math:`\downarrow`
   ================== ==================

   .. code:: ys

      read_verilog macc_xilinx_test.v
                                  hierarchy -check

   ================== ==================
   :math:`\downarrow` :math:`\downarrow`
   \                  
   ================== ==================

.. container:: frame

   – 8/13

   ================== ==================
   ``test1``          ``test2``
   \                  
   :math:`\downarrow` :math:`\downarrow`
   ================== ==================

   .. code:: ys

      techmap -map macc_xilinx_swap_map.v ;;

   ================== ==================
   :math:`\downarrow` :math:`\downarrow`
   \                  
   ================== ==================

.. container:: frame

   – 9/13 Wrapping in ``test1``:

   .. container:: columns

      to 0cm

      .. code:: ys

         techmap -map macc_xilinx_wrap_map.v

         connwrappers -unsigned $__mul_wrapper \
                                     Y Y_WIDTH \
                      -unsigned $__add_wrapper \
                                     Y Y_WIDTH ;;

   .. image:: PRESENTATION_ExAdv/macc_xilinx_test1c.pdf
      :alt: image

.. container:: frame

   – 10/13 Wrapping in ``test2``:

   .. container:: columns

      to 0cm

      .. code:: ys

         techmap -map macc_xilinx_wrap_map.v

         connwrappers -unsigned $__mul_wrapper \
                                     Y Y_WIDTH \
                      -unsigned $__add_wrapper \
                                     Y Y_WIDTH ;;

   .. image:: PRESENTATION_ExAdv/macc_xilinx_test2c.pdf
      :alt: image

.. container:: frame

   – 11/13 Extract in ``test1``:

   .. container:: columns

      to 0cm

      .. code:: ys

         design -push
         read_verilog macc_xilinx_xmap.v
         techmap -map macc_xilinx_swap_map.v
         techmap -map macc_xilinx_wrap_map.v;;
         design -save __macc_xilinx_xmap
         design -pop

      .. code:: ys

         extract -constports -ignore_parameters \
                 -map %__macc_xilinx_xmap       \
                 -swap $__add_wrapper A,B ;;

      to 0cm

   .. image:: PRESENTATION_ExAdv/macc_xilinx_test1d.pdf
      :alt: image
      :width: 11cm

.. container:: frame

   – 12/13 Extract in ``test2``:

   .. container:: columns

      to 0cm

      .. code:: ys

         design -push
         read_verilog macc_xilinx_xmap.v
         techmap -map macc_xilinx_swap_map.v
         techmap -map macc_xilinx_wrap_map.v;;
         design -save __macc_xilinx_xmap
         design -pop

      .. code:: ys

         extract -constports -ignore_parameters \
                 -map %__macc_xilinx_xmap       \
                 -swap $__add_wrapper A,B ;;

      to 0cm

   .. image:: PRESENTATION_ExAdv/macc_xilinx_test2d.pdf
      :alt: image
      :width: 11cm

.. container:: frame

   – 13/13 Unwrap in ``test2``:

Automatic design changes
------------------------

.. container:: frame

Changing the design from Yosys
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   Yosys commands can be used to change the design in memory. Examples
   of this are:

   -  | **Changes in design hierarchy**
      | Commands such as ``flatten`` and ``submod`` can be used to
        change the design hierarchy, i.e. flatten the hierarchy or
        moving parts of a module to a submodule. This has applications
        in synthesis scripts as well as in reverse engineering and
        analysis.

   -  | **Behavioral changes**
      | Commands such as ``techmap`` can be used to make behavioral
        changes to the design, for example changing asynchronous resets
        to synchronous resets. This has applications in design space
        exploration (evaluation of various architectures for one
        circuit).

Example: Async reset to sync reset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: frame

   The following techmap map file replaces all positive-edge async reset
   flip-flops with positive-edge sync reset flip-flops. The code is
   taken from the example Yosys script for ASIC synthesis of the Amber
   ARMv2 CPU.

   .. container:: columns

      to 0cm

      .. code:: verilog

         (* techmap_celltype = "$adff" *)
         module adff2dff (CLK, ARST, D, Q);

             parameter WIDTH = 1;
             parameter CLK_POLARITY = 1;
             parameter ARST_POLARITY = 1;
             parameter ARST_VALUE = 0;

             input CLK, ARST;
             input [WIDTH-1:0] D;
             output reg [WIDTH-1:0] Q;

             wire [1023:0] _TECHMAP_DO_ = "proc";

             wire _TECHMAP_FAIL_ = !CLK_POLARITY || !ARST_POLARITY;

      .. code:: verilog

         // ..continued..


             always @(posedge CLK)
                 if (ARST)
                     Q <= ARST_VALUE;
                 else
                      <= D;

         endmodule

Summary
-------

.. container:: frame

   -  A lot can be achieved in Yosys just with the standard set of
      commands.

   -  The commands ``techmap`` and ``extract`` can be used to prototype
      many complex synthesis tasks.

   .. container:: center

      Questions?

   .. container:: center

      http://www.clifford.at/yosys/

.. |image| image:: PRESENTATION_ExAdv/red_or3x1.pdf
   :width: 10cm
.. |image1| image:: PRESENTATION_ExAdv/sym_mul.pdf
   :width: 6cm
.. |image2| image:: PRESENTATION_ExAdv/mymul.pdf
   :width: 10cm
.. |image3| image:: PRESENTATION_ExAdv/mulshift.pdf
   :width: 5cm
.. |image4| image:: PRESENTATION_ExAdv/addshift.pdf
   :width: 5cm
