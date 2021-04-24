Yosys by example – Synthesis
============================

.. container:: frame

Typical Phases of a Synthesis Flow
----------------------------------

.. container:: frame

   -  Reading and elaborating the design

   -  Higher-level synthesis and optimization

      -  Converting ``always``-blocks to logic and registers

      -  Perform coarse-grain optimizations (resource sharing, const
         folding, ...)

      -  Handling of memories and other coarse-grain blocks

      -  Extracting and optimizing finite state machines

   -  Convert remaining logic to bit-level logic functions

   -  Perform optimizations on bit-level logic functions

   -  Map bit-level logic gates and registers to cell library

   -  Write results to output file

Reading the design
------------------

.. container:: frame

   .. code:: ys

      read_verilog file1.v
      read_verilog -I include_dir -D enable_foo -D WIDTH=12 file2.v
      read_verilog -lib cell_library.v

      verilog_defaults -add -I include_dir
      read_verilog file3.v
      read_verilog file4.v
      verilog_defaults -clear

      verilog_defaults -push
      verilog_defaults -add -I include_dir
      read_verilog file5.v
      read_verilog file6.v
      verilog_defaults -pop

Design elaboration
------------------

.. container:: frame

   During design elaboration Yosys figures out how the modules are
   hierarchically connected. It also re-runs the AST parts of the
   Verilog frontend to create all needed variations of parametric
   modules.

   .. code:: ys

      # simplest form. at least this version should be used after reading all input files
      #
      hierarchy

      # recommended form. fails if parts of the design hierarchy are missing, removes
      # everything that is unreachable from the top module, and marks the top module.
      #
      hierarchy -check -top top_module

The ``proc`` command
--------------------

.. container:: frame

   The Verilog frontend converts ``always``-blocks to RTL netlists for
   the expressions and “processes” for the control- and memory elements.

   The ``proc`` command transforms this “processes” to netlists of RTL
   multiplexer and register cells.

   The ``proc`` command is actually a macro-command that calls the
   following other commands:

   .. code:: ys

      proc_clean      # remove empty branches and processes
      proc_rmdead     # remove unreachable branches
      proc_init       # special handling of "initial" blocks
      proc_arst       # identify modeling of async resets
      proc_mux        # convert decision trees to multiplexer networks
      proc_dff        # extract registers from processes
      proc_clean      # if all went fine, this should remove all the processes

   Many commands can not operate on modules with “processes” in them.
   Usually a call to ``proc`` is the first command in the actual
   synthesis procedure after design elaboration.

.. container:: frame

   – Example 1/3

   .. container:: columns

      .. code:: verilog

         module test(input D, C, R, output reg Q);
             always @(posedge C, posedge R)
                 if (R)
         	    Q <= 0;
         	else
         	    Q <= D;
         endmodule

      .. code:: ys

         read_verilog proc_01.v
         hierarchy -check -top test
         proc;;

   .. image:: PRESENTATION_ExSyn/proc_01.pdf
      :alt: image
      :width: 8cm

.. container:: frame

   – Example 2/3 to 0cm\ |image|

   .. container:: columns

      .. code:: verilog

         module test(input D, C, R, RV,
                     output reg Q);
             always @(posedge C, posedge R)
                 if (R)
         	    Q <= RV;
         	else
         	    Q <= D;
         endmodule

      .. code:: ys

         read_verilog proc_02.v
         hierarchy -check -top test
         proc;;

.. container:: frame

   – Example 3/3 to 0cm\ |image1|

   .. container:: columns

      .. code:: ys

         read_verilog proc_03.v
         hierarchy -check -top test
         proc;;

      .. code:: verilog

         module test(input A, B, C, D, E,
                     output reg Y);
             always @* begin
         	Y <= A;
         	if (B)
         	    Y <= C;
         	if (D)
         	    Y <= E;
             end
         endmodule

The ``opt`` command
-------------------

.. container:: frame

   The ``opt`` command implements a series of simple optimizations. It
   also is a macro command that calls other commands:

   .. code:: ys

      opt_expr                # const folding and simple expression rewriting
      opt_merge -nomux        # merging identical cells

      do
          opt_muxtree         # remove never-active branches from multiplexer tree
          opt_reduce          # consolidate trees of boolean ops to reduce functions
          opt_merge           # merging identical cells
          opt_rmdff           # remove/simplify registers with constant inputs
          opt_clean           # remove unused objects (cells, wires) from design
          opt_expr            # const folding and simple expression rewriting
      while [changed design]

   The command ``clean`` can be used as alias for ``opt_clean``. And
   ``;;`` can be used as shortcut for ``clean``. For example:

   .. code:: ys

      proc; opt; memory; opt_expr;; fsm;;

.. container:: frame

   – Example 1/4 to 0cm\ |image2|

   .. container:: columns

      .. code:: ys

         read_verilog opt_01.v
         hierarchy -check -top test
         opt

      .. code:: verilog

         module test(input A, B, output Y);
         assign Y = A ? A ? B : 1'b1 : B;
         endmodule

.. container:: frame

   – Example 2/4 to 0cm\ |image3|

   .. container:: columns

      .. code:: ys

         read_verilog opt_02.v
         hierarchy -check -top test
         opt

      .. code:: verilog

         module test(input A, output Y, Z);
         assign Y = A == A, Z = A != A;
         endmodule

.. container:: frame

   – Example 3/4 to 0cm\ |image4|

   .. container:: columns

      .. code:: ys

         read_verilog opt_03.v
         hierarchy -check -top test
         opt

      .. code:: verilog

         module test(input  [3:0] A, B,
                     output [3:0] Y, Z);
         assign Y = A + B, Z = B + A;
         endmodule

.. container:: frame

   – Example 4/4 to 0cm\ |image5|

   .. container:: columns

      .. code:: verilog

         module test(input CLK, ARST,
                     output [7:0] Q1, Q2, Q3);

         wire NO_CLK = 0;

         always @(posedge CLK, posedge ARST)
         	if (ARST)
         		Q1 <= 42;

         always @(posedge NO_CLK, posedge ARST)
         	if (ARST)
         		Q2 <= 42;
         	else
         		Q2 <= 23;

         always @(posedge CLK)
         	Q3 <= 42;

         endmodule

      .. code:: ys

         read_verilog opt_04.v
         hierarchy -check -top test
         proc; opt

When to use ``opt`` or ``clean``
--------------------------------

.. container:: frame

   Usually it does not hurt to call ``opt`` after each regular command
   in the synthesis script. But it increases the synthesis time, so it
   is favourable to only call ``opt`` when an improvement can be
   achieved.

   The designs in ``yosys-bigsim`` are a good playground for
   experimenting with the effects of calling ``opt`` in various places
   of the flow.

   It generally is a good idea to call ``opt`` before inherently
   expensive commands such as ``sat`` or ``freduce``, as the possible
   gain is much higher in this cases as the possible loss.

   The ``clean`` command on the other hand is very fast and many
   commands leave a mess (dangling signal wires, etc). For example, most
   commands do not remove any wires or cells. They just change the
   connections and depend on a later call to clean to get rid of the now
   unused objects. So the occasional ``;;`` is a good idea in every
   synthesis script.

The ``memory`` command
----------------------

.. container:: frame

   In the RTL netlist, memory reads and writes are individual cells.
   This makes consolidating the number of ports for a memory easier. The
   ``memory`` transforms memories to an implementation. Per default that
   is logic for address decoders and registers. It also is a macro
   command that calls other commands:

   .. code:: ys

      # this merges registers into the memory read- and write cells.
      memory_dff

      # this collects all read and write cells for a memory and transforms them
      # into one multi-port memory cell.
      memory_collect

      # this takes the multi-port memory cell and transforms it to address decoder
      # logic and registers. This step is skipped if "memory" is called with -nomap.
      memory_map

   Usually it is preferred to use architecture-specific RAM resources
   for memory. For example:

   .. code:: ys

      memory -nomap; techmap -map my_memory_map.v; memory_map

.. container:: frame

   – Example 1/2 to 0cm\ |image6|

   .. container:: columns

      .. code:: ys

         read_verilog memory_01.v
         hierarchy -check -top test
         proc;; memory; opt

      .. code:: verilog

         module test(input      CLK, ADDR,
                     input      [7:0] DIN,
         	    output reg [7:0] DOUT);
             reg [7:0] mem [0:1];
             always @(posedge CLK) begin
                 mem[ADDR] <= DIN;
         	DOUT <= mem[ADDR];
             end
         endmodule

.. container:: frame

   – Example 2/2 to 0cm\ |image7|

   .. container:: columns

      .. code:: verilog

         module test(
             input             WR1_CLK,  WR2_CLK,
             input             WR1_WEN,  WR2_WEN,
             input      [7:0]  WR1_ADDR, WR2_ADDR,
             input      [7:0]  WR1_DATA, WR2_DATA,
             input             RD1_CLK,  RD2_CLK,
             input      [7:0]  RD1_ADDR, RD2_ADDR,
             output reg [7:0]  RD1_DATA, RD2_DATA
         );

         reg [7:0] memory [0:255];

         always @(posedge WR1_CLK)
             if (WR1_WEN)
                 memory[WR1_ADDR] <= WR1_DATA;

         always @(posedge WR2_CLK)
             if (WR2_WEN)
                 memory[WR2_ADDR] <= WR2_DATA;

         always @(posedge RD1_CLK)
             RD1_DATA <= memory[RD1_ADDR];

         always @(posedge RD2_CLK)
             RD2_DATA <= memory[RD2_ADDR];

         endmodule

      .. code:: ys

         read_verilog memory_02.v
         hierarchy -check -top test
         proc;; memory -nomap
         opt -mux_undef -mux_bool

The ``fsm`` command
-------------------

.. container:: frame

   The ``fsm`` command identifies, extracts, optimizes (re-encodes), and
   re-synthesizes finite state machines. It again is a macro that calls
   a series of other commands:

   .. code:: ys

      fsm_detect          # unless got option -nodetect
      fsm_extract

      fsm_opt
      clean
      fsm_opt

      fsm_expand          # if got option -expand
      clean               # if got option -expand
      fsm_opt             # if got option -expand

      fsm_recode          # unless got option -norecode

      fsm_info

      fsm_export          # if got option -export
      fsm_map             # unless got option -nomap

.. container:: frame

   – details Some details on the most important commands from the
   ``fsm_`` group:

   The ``fsm_detect`` command identifies FSM state registers and marks
   them with the ``(* fsm_encoding = "auto" *)`` attribute, if they do
   not have the ``fsm_encoding`` set already. Mark registers with
   ``(* fsm_encoding = "none" *)`` to disable FSM optimization for a
   register.

   The ``fsm_extract`` command replaces the entire FSM (logic and state
   registers) with a ``$fsm`` cell.

   The commands ``fsm_opt`` and ``fsm_recode`` can be used to optimize
   the FSM.

   Finally the ``fsm_map`` command can be used to convert the
   (optimized) ``$fsm`` cell back to logic and registers.

The ``techmap`` command
-----------------------

.. container:: frame

   to 0cm\ |image8| The ``techmap`` command replaces cells with
   implementations given as verilog source. For example implementing a
   32 bit adder using 16 bit adders:

   to 0cm

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

      generate
        if ((A_WIDTH == 32) && (B_WIDTH == 32))
          begin
            wire [16:0] S1 = A[15:0] + B[15:0];
            wire [15:0] S2 = A[31:16] + B[31:16] + S1[16];
            assign Y = {S2[15:0], S1[15:0]};
          end
        else
          wire _TECHMAP_FAIL_ = 1;
      endgenerate

      endmodule

   to 0cm

   .. code:: verilog

      module test(input [31:0]  a, b,
                  output [31:0] y);
      assign y = a + b;
      endmodule

   .. code:: ys

      read_verilog techmap_01.v
      hierarchy -check -top test
      techmap -map techmap_01_map.v;;

.. container:: frame

   – stdcell mapping When ``techmap`` is used without a map file, it
   uses a built-in map file to map all RTL cell types to a generic
   library of built-in logic gates and registers.

   .. container:: block

      The built-in logic gate types are:
      ``$_NOT_ $_AND_ $_OR_ $_XOR_ $_MUX_``

   .. container:: block

      The register types are:
      ``$_SR_NN_ $_SR_NP_ $_SR_PN_ $_SR_PP_ $_DFF_N_ $_DFF_P_ $_DFF_NN0_ $_DFF_NN1_ $_DFF_NP0_ $_DFF_NP1_ $_DFF_PN0_ $_DFF_PN1_ $_DFF_PP0_ $_DFF_PP1_ $_DFFSR_NNN_ $_DFFSR_NNP_ $_DFFSR_NPN_ $_DFFSR_NPP_ $_DFFSR_PNN_ $_DFFSR_PNP_ $_DFFSR_PPN_ $_DFFSR_PPP_ $_DLATCH_N_ $_DLATCH_P_``

The ``abc`` command
-------------------

.. container:: frame

   The ``abc`` command provides an interface to ABC [1]_, an open source
   tool for low-level logic synthesis.

   The ``abc`` command processes a netlist of internal gate types and
   can perform:

   -  logic minimization (optimization)

   -  mapping of logic to standard cell library (liberty format)

   -  mapping of logic to k-LUTs (for FPGA synthesis)

   Optionally ``abc`` can process registers from one clock domain and
   perform sequential optimization (such as register balancing).

   ABC is also controlled using scripts. An ABC script can be specified
   to use more advanced ABC features. It is also possible to write the
   design with ``write_blif`` and load the output file into ABC outside
   of Yosys.

.. container:: frame

   – Example

   .. container:: columns

      .. code:: verilog

         module test(input clk, a, b, c,
                     output reg y);

         	reg [2:0] q1, q2;
         	always @(posedge clk) begin
         		q1 <= { a, b, c };
         		q2 <= q1;
         		y <= ^q2;
         	end
         endmodule

      .. code:: ys

         read_verilog abc_01.v
         read_verilog -lib abc_01_cells.v
         hierarchy -check -top test
         proc; opt; techmap
         abc -dff -liberty abc_01_cells.lib;;

   .. image:: PRESENTATION_ExSyn/abc_01.pdf
      :alt: image

Other special-purpose mapping commands
--------------------------------------

.. container:: frame

   .. container:: block

      ``dfflibmap`` This command maps the internal register cell types
      to the register types described in a liberty file.

   .. container:: block

      ``hilomap`` Some architectures require special driver cells for
      driving a constant hi or lo value. This command replaces simple
      constants with instances of such driver cells.

   .. container:: block

      ``iopadmap`` Top-level input/outputs must usually be implemented
      using special I/O-pad cells. This command inserts this cells to
      the design.

Example Synthesis Script
------------------------

.. container:: frame

   .. container:: columns

      .. code:: ys

         # read and elaborate design
         read_verilog cpu_top.v cpu_ctrl.v cpu_regs.v
         read_verilog -D WITH_MULT cpu_alu.v
         hierarchy -check -top cpu_top

         # high-level synthesis
         proc; opt; fsm;; memory -nomap; opt

         # substitute block rams
         techmap -map map_rams.v

         # map remaining memories
         memory_map

         # low-level synthesis
         techmap; opt; flatten;; abc -lut6
         techmap -map map_xl_cells.v

         # add clock buffers
         select -set xl_clocks t:FDRE %x:+FDRE[C] t:FDRE %d
         iopadmap -inpad BUFGP O:I @xl_clocks

         # add io buffers
         select -set xl_nonclocks w:* t:BUFGP %x:+BUFGP[I] %d
         iopadmap -outpad OBUF I:O -inpad IBUF O:I @xl_nonclocks

         # write synthesis results
         write_edif synth.edif

      .. container:: block

         Teaser / Outlook

         The weird ``select`` expressions at the end of this script are
         discussed in the next part (Section 3, “Advanced Synthesis”) of
         this presentation.

Summary
-------

.. container:: frame

   -  Yosys provides commands for each phase of the synthesis.

   -  Each command solves a (more or less) simple problem.

   -  Complex commands are often only front-ends to simple commands.

   -  ``proc; opt; fsm; opt; memory; opt; techmap; opt; abc;;``

   .. container:: center

      Questions?

   .. container:: center

      http://www.clifford.at/yosys/

.. [1]
   http://www.eecs.berkeley.edu/~alanmi/abc/

.. |image| image:: PRESENTATION_ExSyn/proc_02.pdf
.. |image1| image:: PRESENTATION_ExSyn/proc_03.pdf
.. |image2| image:: PRESENTATION_ExSyn/opt_01.pdf
.. |image3| image:: PRESENTATION_ExSyn/opt_02.pdf
.. |image4| image:: PRESENTATION_ExSyn/opt_03.pdf
.. |image5| image:: PRESENTATION_ExSyn/opt_04.pdf
   :width: 6cm
.. |image6| image:: PRESENTATION_ExSyn/memory_01.pdf
.. |image7| image:: PRESENTATION_ExSyn/memory_02.pdf
   :width: 7.5cm
.. |image8| image:: PRESENTATION_ExSyn/techmap_01.pdf
   :width: 12cm
