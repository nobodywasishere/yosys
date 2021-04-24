.. _chapter:prog:

Programming Yosys Extensions
============================

This chapter contains some bits and pieces of information about
programming yosys extensions. Also consult the section on programming in
the “Yosys Presentation” (can be downloaded from the Yosys website as
PDF) and don’t be afraid to ask questions on the YosysHQ Slack.

Guidelines
----------

The ``guidelines`` directory contains notes on various aspects of Yosys
development. The files ``GettingStarted`` and ``CodingStyle`` may be of
particular interest, and are reproduced here.

::

   Getting Started
   ===============


   Outline of a Yosys command
   --------------------------

   Here is a the C++ code for a "hello_world" Yosys command (hello.cc):

   	#include "kernel/yosys.h"

   	USING_YOSYS_NAMESPACE
   	PRIVATE_NAMESPACE_BEGIN

   	struct HelloWorldPass : public Pass {
   		HelloWorldPass() : Pass("hello_world") { }
   		void execute(vector<string>, Design*) override {
   			log("Hello World!\n");
   		}
   	} HelloWorldPass;

   	PRIVATE_NAMESPACE_END

   This can be built into a Yosys module using the following command:

   	yosys-config --exec --cxx --cxxflags --ldflags -o hello.so -shared hello.cc --ldlibs

   Or short:

   	yosys-config --build hello.so hello.cc

   And then executed using the following command:

   	yosys -m hello.so -p hello_world


   Yosys Data Structures
   ---------------------

   Here is a short list of data structures that you should make yourself familiar
   with before you write C++ code for Yosys. The following data structures are all
   defined when "kernel/yosys.h" is included and USING_YOSYS_NAMESPACE is used.

     1. Yosys Container Classes

   Yosys uses dict<K, T> and pool<T> as main container classes. dict<K, T> is
   essentially a replacement for std::unordered_map<K, T> and pool<T> is a
   replacement for std::unordered_set<T>. The main characteristics are:

   	- dict<K, T> and pool<T> are about 2x faster than the std containers

   	- references to elements in a dict<K, T> or pool<T> are invalidated by
   	  insert and remove operations (similar to std::vector<T> on push_back()).

   	- some iterators are invalidated by erase(). specifically, iterators
   	  that have not passed the erased element yet are invalidated. (erase()
   	  itself returns valid iterator to the next element.)

   	- no iterators are invalidated by insert(). elements are inserted at
   	  begin(). i.e. only a new iterator that starts at begin() will see the
   	  inserted elements.

   	- the method .count(key, iterator) is like .count(key) but only
   	  considers elements that can be reached via the iterator.

   	- iterators can be compared. it1 < it2 means that the position of t2
   	  can be reached via t1 but not vice versa.

   	- the method .sort() can be used to sort the elements in the container
   	  the container stays sorted until elements are added or removed.

   	- dict<K, T> and pool<T> will have the same order of iteration across
   	  all compilers, standard libraries and architectures.

   In addition to dict<K, T> and pool<T> there is also an idict<K> that
   creates a bijective map from K to the integers. For example:

   	idict<string, 42> si;
   	log("%d\n", si("hello"));      // will print 42
   	log("%d\n", si("world"));      // will print 43
   	log("%d\n", si.at("world"));   // will print 43
   	log("%d\n", si.at("dummy"));   // will throw exception
   	log("%s\n", si[42].c_str()));  // will print hello
   	log("%s\n", si[43].c_str()));  // will print world
   	log("%s\n", si[44].c_str()));  // will throw exception

   It is not possible to remove elements from an idict.

   Finally mfp<K> implements a merge-find set data structure (aka. disjoint-set or
   union-find) over the type K ("mfp" = merge-find-promote).

     2. Standard STL data types

   In Yosys we use std::vector<T> and std::string whenever applicable. When
   dict<K, T> and pool<T> are not suitable then std::map<K, T> and std::set<T>
   are used instead.

   The types std::vector<T> and std::string are also available as vector<T>
   and string in the Yosys namespace.

     3. RTLIL objects

   The current design (essentially a collection of modules, each defined by a
   netlist) is stored in memory using RTLIL object (declared in kernel/rtlil.h,
   automatically included by kernel/yosys.h). You should glance over at least
   the declarations for the following types in kernel/rtlil.h:

   	RTLIL::IdString
   		This is a handle for an identifier (e.g. cell or wire name).
   		It feels a lot like a std::string, but is only a single int
   		in size. (The actual string is stored in a global lookup
   		table.)

   	RTLIL::SigBit
   		A single signal bit. I.e. either a constant state (0, 1,
   		x, z) or a single bit from a wire.

   	RTLIL::SigSpec
   		Essentially a vector of SigBits.

   	RTLIL::Wire
   	RTLIL::Cell
   		The building blocks of the netlist in a module.

   	RTLIL::Module
   	RTLIL::Design
   		The module is a container with connected cells and wires
   		in it. The design is a container with modules in it.

   All this types are also available without the RTLIL:: prefix in the Yosys
   namespace.

     4. SigMap and other Helper Classes

   There are a couple of additional helper classes that are in wide use
   in Yosys. Most importantly there is SigMap (declared in kernel/sigtools.h).

   When a design has many wires in it that are connected to each other, then a
   single signal bit can have multiple valid names. The SigMap object can be used
   to map SigSpecs or SigBits to unique SigSpecs and SigBits that consistently
   only use one wire from such a group of connected wires. For example:

   	SigBit a = module->addWire(NEW_ID);
   	SigBit b = module->addWire(NEW_ID);
   	module->connect(a, b);

   	log("%d\n", a == b); // will print 0

   	SigMap sigmap(module);
   	log("%d\n", sigmap(a) == sigmap(b)); // will print 1


   Using the RTLIL Netlist Format
   ------------------------------

   In the RTLIL netlist format the cell ports contain SigSpecs that point to the
   Wires. There are no references in the other direction. This has two direct
   consequences:

   (1) It is very easy to go from cells to wires but hard to go in the other way.

   (2) There is no danger in removing cells from the netlists, but removing wires
   can break the netlist format when there are still references to the wire
   somewhere in the netlist.

   The solution to (1) is easy: Create custom indexes that allow you to make fast
   lookups for the wire-to-cell direction. You can either use existing generic
   index structures to do that (such as the ModIndex class) or write your own
   index. For many application it is simplest to construct a custom index. For
   example:

   	SigMap sigmap(module);
   	dict<SigBit, Cell*> sigbit_to_driver_index;

   	for (auto cell : module->cells())
   		for (auto &conn : cell->connections())
   			if (cell->output(conn.first))
   				for (auto bit : sigmap(conn.second))
   					sigbit_to_driver_index[bit] = cell;

   Regarding (2): There is a general theme in Yosys that you don't remove wires
   from the design. You can rename them, unconnect them, but you do not actually remove
   the Wire object from the module. Instead you let the "clean" command take care
   of the dangling wires. On the other hand it is safe to remove cells (as long as
   you make sure this does not invalidate a custom index you are using in your code).


   Example Code
   ------------

   The following yosys commands are a good starting point if you are looking for examples
   of how to use the Yosys API:

   	manual/CHAPTER_Prog/stubnets.cc
   	manual/PRESENTATION_Prog/my_cmd.cc


   Script Passes
   -------------

   The ScriptPass base class can be used to implement passes that just call other passes,
   like a script. Examples for such passes are:

   	techlibs/common/prep.cc
   	techlibs/common/synth.cc

   In some cases it is easier to implement such a pass as regular pass, for example when
   ScriptPass doesn't provide the type of flow control desired. (But many of the
   script passes in Yosys that don't use ScriptPass simply predate the ScriptPass base
   class.) Examples for such passes are:

   	passes/opt/opt.cc
   	passes/proc/proc.cc

   Whether they use the ScriptPass base-class or not, a pass should always either
   call other passes without doing any non-trivial work itself, or should implement
   a non-trivial algorithm but not call any other passes. The reason for this is that
   this helps containing complexity in individual passes and simplifies debugging the
   entire system.

   Exceptions to this rule should be rare and limited to cases where calling other
   passes is optional and only happens when requested by the user (such as for
   example `techmap -autoproc`), or where it is about commands that are "top-level
   commands" in their own right, not components to be used in regular synthesis
   flows (such as the `bugpoint` command).

   A pass that would "naturally" call other passes and also do some work itself
   should be re-written in one of two ways:

   1) It could be re-written as script pass with the parts that are not calls
   to other passes factored out into individual new passes. Usually in those
   cases the new sub passes share the same prefix as the top-level script pass.

   2) It could be re-written so that it already expects the design in a certain
   state, expecting the calling script to set up this state before calling the
   pass in questions.

   Many back-ends are examples for the 2nd approach. For example, `write_aiger`
   does not convert the design into AIG representation, but expects the design
   to be already in this form, and prints an `Unsupported cell type` error
   message otherwise.


   Notes on the existing codebase
   ------------------------------

   For historical reasons not all parts of Yosys adhere to the current coding
   style. When adding code to existing parts of the system, adhere to this guide
   for the new code instead of trying to mimic the style of the surrounding code.

::

   Coding Style
   ============


   Formatting of code
   ------------------

   - Yosys code is using tabs for indentation. Tabs are 8 characters.

   - A continuation of a statement in the following line is indented by
     two additional tabs.

   - Lines are as long as you want them to be. A good rule of thumb is
     to break lines at about column 150.

   - Opening braces can be put on the same or next line as the statement
     opening the block (if, switch, for, while, do). Put the opening brace
     on its own line for larger blocks, especially blocks that contains
     blank lines.

   - Otherwise stick to the Linux Kernel Coding Style:
       https://www.kernel.org/doc/Documentation/CodingStyle


   C++ Language
   -------------

   Yosys is written in C++11. At the moment only constructs supported by
   gcc 4.8 are allowed in Yosys code. This will change in future releases.

   In general Yosys uses "int" instead of "size_t". To avoid compiler
   warnings for implicit type casts, always use "GetSize(foobar)" instead
   of "foobar.size()". (GetSize() is defined in kernel/yosys.h)

   Use range-based for loops whenever applicable.

The “stubsnets” Example Module
------------------------------

The following is the complete code of the “stubsnets” example module. It
is included in the Yosys source distribution as
``manual/CHAPTER_Prog/stubnets.cc``.

.. code:: c++
   :number-lines:

   // This is free and unencumbered software released into the public domain.
   //
   // Anyone is free to copy, modify, publish, use, compile, sell, or
   // distribute this software, either in source code form or as a compiled
   // binary, for any purpose, commercial or non-commercial, and by any
   // means.

   #include "kernel/yosys.h"
   #include "kernel/sigtools.h"

   #include <string>
   #include <map>
   #include <set>

   USING_YOSYS_NAMESPACE
   PRIVATE_NAMESPACE_BEGIN

   // this function is called for each module in the design
   static void find_stub_nets(RTLIL::Design *design, RTLIL::Module *module, bool report_bits)
   {
   	// use a SigMap to convert nets to a unique representation
   	SigMap sigmap(module);

   	// count how many times a single-bit signal is used
   	std::map<RTLIL::SigBit, int> bit_usage_count;

   	// count output lines for this module (needed only for summary output at the end)
   	int line_count = 0;

   	log("Looking for stub wires in module %s:\n", RTLIL::id2cstr(module->name));

   	// For all ports on all cells
   	for (auto &cell_iter : module->cells_)
   	for (auto &conn : cell_iter.second->connections())
   	{
   		// Get the signals on the port
   		// (use sigmap to get a uniqe signal name)
   		RTLIL::SigSpec sig = sigmap(conn.second);

   		// add each bit to bit_usage_count, unless it is a constant
   		for (auto &bit : sig)
   			if (bit.wire != NULL)
   				bit_usage_count[bit]++;
   	}

   	// for each wire in the module
   	for (auto &wire_iter : module->wires_)
   	{
   		RTLIL::Wire *wire = wire_iter.second;

   		// .. but only selected wires
   		if (!design->selected(module, wire))
   			continue;

   		// add +1 usage if this wire actually is a port
   		int usage_offset = wire->port_id > 0 ? 1 : 0;

   		// we will record which bits of the (possibly multi-bit) wire are stub signals
   		std::set<int> stub_bits;

   		// get a signal description for this wire and split it into separate bits
   		RTLIL::SigSpec sig = sigmap(wire);

   		// for each bit (unless it is a constant):
   		// check if it is used at least two times and add to stub_bits otherwise
   		for (int i = 0; i < GetSize(sig); i++)
   			if (sig[i].wire != NULL && (bit_usage_count[sig[i]] + usage_offset) < 2)
   				stub_bits.insert(i);

   		// continue if no stub bits found
   		if (stub_bits.size() == 0)
   			continue;

   		// report stub bits and/or stub wires, don't report single bits
   		// if called with report_bits set to false.
   		if (GetSize(stub_bits) == GetSize(sig)) {
   			log("  found stub wire: %s\n", RTLIL::id2cstr(wire->name));
   		} else {
   			if (!report_bits)
   				continue;
   			log("  found wire with stub bits: %s [", RTLIL::id2cstr(wire->name));
   			for (int bit : stub_bits)
   				log("%s%d", bit == *stub_bits.begin() ? "" : ", ", bit);
   			log("]\n");
   		}

   		// we have outputted a line, increment summary counter
   		line_count++;
   	}

   	// report summary
   	if (report_bits)
   		log("  found %d stub wires or wires with stub bits.\n", line_count);
   	else
   		log("  found %d stub wires.\n", line_count);
   }

   // each pass contains a singleton object that is derived from Pass
   struct StubnetsPass : public Pass {
   	StubnetsPass() : Pass("stubnets") { }
   	void execute(std::vector<std::string> args, RTLIL::Design *design) override
   	{
   		// variables to mirror information from passed options
   		bool report_bits = 0;

   		log_header(design, "Executing STUBNETS pass (find stub nets).\n");

   		// parse options
   		size_t argidx;
   		for (argidx = 1; argidx < args.size(); argidx++) {
   			std::string arg = args[argidx];
   			if (arg == "-report_bits") {
   				report_bits = true;
   				continue;
   			}
   			break;
   		}

   		// handle extra options (e.g. selection)
   		extra_args(args, argidx, design);

   		// call find_stub_nets() for each module that is either
   		// selected as a whole or contains selected objects.
   		for (auto &it : design->modules_)
   			if (design->selected_module(it.first))
   				find_stub_nets(design, it.second, report_bits);
   	}
   } StubnetsPass;

   PRIVATE_NAMESPACE_END

.. code:: makefile
   :number-lines:

   test: stubnets.so
   	yosys -ql test1.log -m ./stubnets.so test.v -p "stubnets"
   	yosys -ql test2.log -m ./stubnets.so test.v -p "opt; stubnets"
   	yosys -ql test3.log -m ./stubnets.so test.v -p "techmap; opt; stubnets -report_bits"
   	tail test1.log test2.log test3.log

   stubnets.so: stubnets.cc
   	yosys-config --exec --cxx --cxxflags --ldflags -o $@ -shared $^ --ldlibs

   clean:
   	rm -f test1.log test2.log test3.log
   	rm -f stubnets.so stubnets.d

.. code:: verilog
   :number-lines:

   module uut(in1, in2, in3, out1, out2);

   input [8:0] in1, in2, in3;
   output [8:0] out1, out2;

   assign out1 = in1 + in2 + (in3 >> 4);

   endmodule
