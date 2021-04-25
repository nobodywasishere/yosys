Introduction to Yosys
=====================

.. container:: frame

Representations of (digital) Circuits
-------------------------------------

.. container:: frame

   -  Graphical

      -  Schematic Diagram

      -  Physical Layout

   -  Non-graphical

      -  Netlists

      -  Hardware Description Languages (HDLs)

   .. container:: block

      Definition:

Levels of Abstraction for Digital Circuits
------------------------------------------

.. container:: frame

   -  System Level

   -  High Level

   -  Behavioral Level

   -  Register-Transfer Level (RTL)

   -  Logical Gate Level

   -  Physical Gate Level

   -  Switch Level

   .. container:: block

      Definition:

Digital Circuit Synthesis
-------------------------

.. container:: frame

   Synthesis Tools (such as Yosys) can transform HDL code to circuits:

   .. container:: center

What Yosys can and can’t do
---------------------------

.. container:: frame

   Things Yosys can do:

   -  Read and process (most of) modern Verilog-2005 code.

   -  Perform all kinds of operations on netlist (RTL, Logic, Gate).

   -  Perform logic optimizations and gate mapping with ABC [1]_.

   Things Yosys can’t do:

   -  Process high-level languages such as C/C++/SystemC.

   -  Create physical layouts (place&route).

   A typical flow combines Yosys with with a low-level implementation
   tool, such as Qflow [2]_ for ASIC designs.

Yosys Data- and Control-Flow
----------------------------

.. container:: frame

   A (usually short) synthesis script controls Yosys.

   This scripts contain three types of commands:

   -  **Frontends**, that read input files (usually Verilog).

   -  **Passes**, that perform transformations on the design in memory.

   -  **Backends**, that write the design in memory to a file (various
      formats are available: Verilog, BLIF, EDIF, SPICE, BTOR, …).

   .. container:: center

Program Components and Data Formats
-----------------------------------

.. container:: frame

   .. container:: center

Example Project
---------------

.. container:: frame

   The following slides cover an example project. This project contains
   three files:

   -  A simple ASIC synthesis script

   -  A digital design written in Verilog

   -  A simple CMOS cell library

   | Direct link to the files:
   | https://github.com/cliffordwolf/yosys/tree/master/manual/PRESENTATION_Intro

.. container:: frame

   – Synthesis Script

   ``# read design``

   | # the high-level stuff
   | ; ; ; ; ;

   | # mapping to internal cell library
   | ;

   ``# mapping flip-flops to mycells.lib``

   | # mapping logic to mycells.lib

   | # cleanup

   | # write synthesized design

   .. container:: block

      Command: ````

.. container:: frame

   – Verilog Source: ``counter.v``

   .. code:: verilog

      module counter (clk, rst, en, count);

      	input clk, rst, en;
      	output reg [1:0] count;

      	always @(posedge clk)
      		if (rst)
      			count <= 2'd0;
      		else if (en)
      			count <= count + 2'd1;

      endmodule

.. container:: frame

   – Cell Library: ``mycells.lib``

   .. container:: columns

      .. code:: liberty

         library(demo) {
           cell(BUF) {
             area: 6;
             pin(A) { direction: input; }
             pin(Y) { direction: output;
                       function: "A"; }
           }
           cell(NOT) {
             area: 3;
             pin(A) { direction: input; }
             pin(Y) { direction: output;
                       function: "A'"; }
           }
           cell(NAND) {
             area: 4;
             pin(A) { direction: input; }
             pin(B) { direction: input; }
             pin(Y) { direction: output;
                      function: "(A*B)'"; }
           }

      .. code:: liberty

           cell(NOR) {
             area: 4;
             pin(A) { direction: input; }
             pin(B) { direction: input; }
             pin(Y) { direction: output;
                      function: "(A+B)'"; }
           }
           cell(DFF) {
             area: 18;
             ff(IQ, IQN) { clocked_on: C;
                           next_state: D; }
             pin(C) { direction: input;
                          clock: true; }
             pin(D) { direction: input; }
             pin(Q) { direction: output;
                       function: "IQ"; }
           }
         }

Running the Synthesis Script
----------------------------

.. container:: frame

   – Step 1/4

   ::

      read_verilog counter.v
      hierarchy -check -top counter

   .. image:: PRESENTATION_Intro/counter_00.pdf
      :alt: image

.. container:: frame

   – Step 2/4

   ::

      proc; opt; fsm; opt; memory; opt

   .. image:: PRESENTATION_Intro/counter_01.pdf
      :alt: image

.. container:: frame

   – Step 3/4

   ::

      techmap; opt

   .. image:: PRESENTATION_Intro/counter_02.pdf
      :alt: image

.. container:: frame

   – Step 4/4

   ::

      dfflibmap -liberty mycells.lib
      abc -liberty mycells.lib
      clean

   .. image:: PRESENTATION_Intro/counter_03.pdf
      :alt: image
      :width: 10cm

The synth command
-----------------

.. container:: frame

   Yosys contains a default (recommended example) synthesis script in
   form of the ``synth`` command. The following commands are executed by
   this synthesis command:

   .. container:: columns

      .. code:: ys

         begin:
             hierarchy -check [-top <top>]

         coarse:
             proc
             opt
             wreduce
             alumacc
             share
             opt
             fsm
             opt -fast
             memory -nomap
             opt_clean

      .. code:: ys

         fine:
             opt -fast -full
             memory_map
             opt -full
             techmap
             opt -fast

         abc:
             abc -fast
             opt -fast

Yosys Commands
--------------

.. container:: frame

   1/3 (excerpt) Command reference:

   -  Use “``help``” for a command list and “``help command``” for
      details.

   -  Or run “``yosys -H``” or “``yosys -h command``”.

   -  Or go to http://www.clifford.at/yosys/documentation.html.

   Commands for design navigation and investigation:

   .. code:: ys

      cd                   # a shortcut for 'select -module <name>'
          ls                   # list modules or objects in modules
          dump                 # print parts of the design in RTLIL format
          show                 # generate schematics using graphviz
          select               # modify and view the list of selected objects

   Commands for executing scripts or entering interactive mode:

   .. code:: ys

      shell                # enter interactive command mode
          history              # show last interactive commands
          script               # execute commands from script file
          tcl                  # execute a TCL script file

.. container:: frame

   2/3 (excerpt) Commands for reading and elaborating the design:

   .. code:: ys

      read_rtlil           # read modules from RTLIL file
          read_verilog         # read modules from Verilog file
          hierarchy            # check, expand and clean up design hierarchy

   Commands for high-level synthesis:

   .. code:: ys

      proc                 # translate processes to netlists
          fsm                  # extract and optimize finite state machines
          memory               # translate memories to basic cells
          opt                  # perform simple optimizations

   Commands for technology mapping:

   .. code:: ys

      techmap              # generic technology mapper
          abc                  # use ABC for technology mapping
          dfflibmap            # technology mapping of flip-flops
          hilomap              # technology mapping of constant hi- and/or lo-drivers
          iopadmap             # technology mapping of i/o pads (or buffers)
          flatten              # flatten design

.. container:: frame

   3/3 (excerpt) Commands for writing the results:

   .. code:: ys

      write_blif           # write design to BLIF file
          write_btor           # write design to BTOR file
          write_edif           # write design to EDIF netlist file
          write_rtlil          # write design to RTLIL file
          write_spice          # write design to SPICE netlist file
          write_verilog        # write design to Verilog file

   Script-Commands for standard synthesis tasks:

   .. code:: ys

      synth                # generic synthesis script
          synth_xilinx         # synthesis for Xilinx FPGAs

   Commands for model checking:

   .. code:: ys

      sat                  # solve a SAT problem in the circuit
          miter                # automatically create a miter circuit
          scc                  # detect strongly connected components (logic loops)

   ... and many many more.

More Verilog Examples
---------------------

.. container:: frame

   1/3

   .. code:: verilog

      module detectprime(a, y);
          input [4:0] a;
          output y;

          integer i, j;
          reg [31:0] lut;

          initial begin
              for (i = 0; i < 32; i = i+1) begin
                  lut[i] = i > 1;
                  for (j = 2; j*j <= i; j = j+1)
                      if (i % j == 0)
                          lut[i] = 0;
              end
          end

          assign y = lut[a];
      endmodule

.. container:: frame

   2/3

   .. code:: verilog

      module carryadd(a, b, y);
          parameter WIDTH = 8;
          input [WIDTH-1:0] a, b;
          output [WIDTH-1:0] y;

          genvar i;
          generate
              for (i = 0; i < WIDTH; i = i+1) begin:STAGE
                  wire IN1 = a[i], IN2 = b[i];
                  wire C, Y;
                  if (i == 0)
                      assign C = IN1 & IN2, Y = IN1 ^ IN2;
                  else
                      assign C = (IN1 & IN2) | ((IN1 | IN2) & STAGE[i-1].C),
                             Y = IN1 ^ IN2 ^ STAGE[i-1].C;
                  assign y[i] = Y;
              end
          endgenerate
      endmodule

.. container:: frame

   3/3

   .. code:: verilog

      module cam(clk, wr_enable, wr_addr, wr_data, rd_data, rd_addr, rd_match);
          parameter WIDTH = 8;
          parameter DEPTH = 16;
          localparam ADDR_BITS = $clog2(DEPTH-1);

          input clk, wr_enable;
          input [ADDR_BITS-1:0] wr_addr;
          input [WIDTH-1:0] wr_data, rd_data;
          output reg [ADDR_BITS-1:0] rd_addr;
          output reg rd_match;

          integer i;
          reg [WIDTH-1:0] mem [0:DEPTH-1];

          always @(posedge clk) begin
              rd_addr <= 'bx;
              rd_match <= 0;
              for (i = 0; i < DEPTH; i = i+1)
                  if (mem[i] == rd_data) begin
                      rd_addr <= i;
                      rd_match <= 1;
                  end
              if (wr_enable)
                  mem[wr_addr] <= wr_data;
          end
      endmodule

Currently unsupported Verilog-2005 language features
----------------------------------------------------

.. container:: frame

   -  Tri-state logic

   -  The wor/wand wire types (maybe for 0.5)

   -  Latched logic (is synthesized as logic with feedback loops)

   -  Some non-synthesizable features that should be ignored in
      synthesis are not supported by the parser and cause a parser error
      (file a bug report if you encounter this problem)

Verification of Yosys
---------------------

.. container:: frame

   Continuously checking the correctness of Yosys and making sure that
   new features do not break old ones is a high priority in Yosys.

   Two external test suites have been built for Yosys: VlogHammer and
   yosys-bigsim (see next slides)

   In addition to that, yosys comes with :math:`\approx\!200` test cases
   used in “``make test``”.

   A debug build of Yosys also contains a lot of asserts and checks the
   integrity of the internal state after each command.

.. container:: frame

   – VlogHammer VlogHammer is a Verilog regression test suite developed
   to test the different subsystems in Yosys by comparing them to each
   other and to the output created by some other tools (Xilinx Vivado,
   Xilinx XST, Altera Quartus II, ...).

   Yosys Subsystems tested: Verilog frontend, const folding, const eval,
   technology mapping, simulation models, SAT models.

   Thousands of auto-generated test cases containing code such as:

   .. code:: verilog

      assign y9 = $signed(((+$signed((^(6'd2 ** a2))))<$unsigned($unsigned(((+a3))))));
      assign y10 = (-((+((+{2{(~^p13)}})))^~(!{{b5,b1,a0},(a1&p12),(a4+a3)})));
      assign y11 = (~&(-{(-3'sd3),($unsigned($signed($unsigned({p0,b4,b1}))))}));

   Some bugs in Yosys where found and fixed thanks to VlogHammer. Over
   50 bugs in the other tools used as external reference where found and
   reported so far.

.. container:: frame

   – yosys-bigsim yosys-bigsim is a collection of real-world open-source
   Verilog designs and test benches. yosys-bigsim compares the testbench
   outputs of simulations of the original Verilog code and synthesis
   results.

   The following designs are included in yosys-bigsim (excerpt):

   -  ``openmsp430`` – an MSP430 compatible 16 bit CPU

   -  ``aes_5cycle_2stage`` – an AES encryption core

   -  ``softusb_navre`` – an AVR compatible 8 bit CPU

   -  ``amber23`` – an ARMv2 compatible 32 bit CPU

   -  ``lm32`` – another 32 bit CPU from Lattice Semiconductor

   -  ``verilog-pong`` – a hardware pong game with VGA output

   -  ``elliptic_curve_group`` – ECG point-add and point-scalar-mul core

   -  ``reed_solomon_decoder`` – a Reed-Solomon Error Correction Decoder

Benefits of Open Source HDL Synthesis
-------------------------------------

.. container:: frame

   -  Cost (also applies to “free as in free beer” solutions)

   -  Availability and Reproducibility

   -  Framework- and all-in-one-aspects

   -  Educational Tool

   Yosys is open source under the ISC license.

.. container:: frame

   – 1/3

   -  Cost (also applies to “free as in free beer” solutions):

      Today the cost for a mask set in :math:`\unit[180]{nm}` technology
      is far less than the cost for the design tools needed to design
      the mask layouts. Open Source ASIC flows are an important enabler
      for ASIC-level Open Source Hardware.

   -  Availability and Reproducibility:

      If you are a researcher who is publishing, you want to use tools
      that everyone else can also use. Even if most universities have
      access to all major commercial tools, you usually do not have easy
      access to the version that was used in a research project a couple
      of years ago. With Open Source tools you can even release the
      source code of the tool you have used alongside your data.

.. container:: frame

   – 2/3

   -  Framework:

      Yosys is not only a tool. It is a framework that can be used as
      basis for other developments, so researchers and hackers alike do
      not need to re-invent the basic functionality. Extensibility was
      one of Yosys’ design goals.

   -  All-in-one:

      Because of the framework characteristics of Yosys, an increasing
      number of features become available in one tool. Yosys not only
      can be used for circuit synthesis but also for formal equivalence
      checking, SAT solving, and for circuit analysis, to name just a
      few other application domains. With proprietary software one needs
      to learn a new tool for each of these applications.

.. container:: frame

   – 3/3

   -  Educational Tool:

      Proprietary synthesis tools are at times very secretive about
      their inner workings. They often are “black boxes”. Yosys is very
      open about its internals and it is easy to observe the different
      steps of synthesis.

   .. container:: block

      Yosys is licensed under the ISC license: Permission to use, copy,
      modify, and/or distribute this software for any purpose with or
      without fee is hereby granted, provided that the above copyright
      notice and this permission notice appear in all copies.

Typical Applications for Yosys
------------------------------

.. container:: frame

   -  Synthesis of final production designs

   -  Pre-production synthesis (trial runs before investing in other
      tools)

   -  Conversion of full-featured Verilog to simple Verilog

   -  Conversion of Verilog to other formats (BLIF, BTOR, etc)

   -  Demonstrating synthesis algorithms (e.g. for educational purposes)

   -  Framework for experimenting with new algorithms

   -  Framework for building custom flows [3]_

Projects (that I know of) using Yosys
-------------------------------------

.. container:: frame

   – (1/2)

   -  | Ongoing PhD project on coarse grain synthesis
      | Johann Glaser and Clifford Wolf. Methodology and Example-Driven
        Interconnect Synthesis for Designing Heterogeneous Coarse-Grain
        Reconfigurable Architectures. In Jan Haase, editor, *Models,
        Methods, and Tools for Complex Chip Design. Lecture Notes in
        Electrical Engineering. Volume 265, 2014, pp 201-221. Springer,
        2013.*

   -  I know several people that use Yosys simply as Verilog frontend
      for other flows (using either the BLIF and BTOR backends).

   -  I know some analog chip designers that use Yosys for small digital
      control logic because it is simpler than setting up a commercial
      flow.

.. container:: frame

   – (2/2)

   -  Efabless

      -  Not much information on the website (http://efabless.com) yet.

      -  Very cheap 180nm prototyping process (partnering with various
         fabs)

      -  A semiconductor company, NOT an EDA company

      -  Web-based design environment

      -  HDL Synthesis using Yosys

      -  Custom place&route tool

      -  | efabless is building an Open Source IC as reference design.
         | (to be announced soon: http://www.openic.io)

Supported Platforms
-------------------

.. container:: frame

   -  Main development OS: Kubuntu 14.04

   -  There is a PPA for ubuntu (not maintained by me)

   -  Any current Debian-based system should work out of the box

   -  When building on other Linux distributions:

      -  Needs compiler with some C++11 support

      -  See README file for build instructions

      -  Post to the subreddit if you get stuck

   -  Ported to OS X (Darwin) and OpenBSD

   -  Native win32 build with VisualStudio

   -  Cross win32 build with MXE

Other Open Source Tools
-----------------------

.. container:: frame

   -  | Icarus Verilog
      | Verilog Simulation (and also a good syntax checker)
      | http://iverilog.icarus.com/

   -  | Qflow (incl. TimberWolf, qrouter and Magic)
      | A complete ASIC synthesis flow, using Yosys and ABC
      | http://opencircuitdesign.com/qflow/

   -  | ABC
      | Logic optimization, technology mapping, and more
      | http://www.eecs.berkeley.edu/~alanmi/abc/

Yosys needs you
---------------

.. container:: frame

   …as an active user:

   -  Use Yosys for on your own projects

   -  .. even if you are not using it as final synthesis tool

   -  Join the discussion on the Subreddit

   -  Report bugs and send in feature requests

   …as a developer:

   -  Use Yosys as environment for your (research) work

   -  .. you might also want to look into ABC for logic-level stuff

   -  Fork the project on github or create loadable plugins

   -  We need a VHDL frontend or a good VHDL-to-Verilog converter

Documentation, Downloads, Contacts
----------------------------------

.. container:: frame

   -  | Website:
      | http://www.clifford.at/yosys/

   -  | Manual, Command Reference, Application Notes:
      | http://www.clifford.at/yosys/documentation.html

   -  | Instead of a mailing list we have a SubReddit:
      | http://www.reddit.com/r/yosys/

   -  | Direct link to the source code:
      | https://github.com/cliffordwolf/yosys

Summary
-------

.. container:: frame

   -  Yosys is a powerful tool and framework for Verilog synthesis.

   -  It uses a command-based interface and can be controlled by
      scripts.

   -  By combining existing commands and implementing new commands Yosys
      can be used in a wide range of application far beyond simple
      synthesis.

   .. container:: center

      Questions?

   .. container:: center

      http://www.clifford.at/yosys/

.. [1]
   http://www.eecs.berkeley.edu/~alanmi/abc/

.. [2]
   http://opencircuitdesign.com/qflow/

.. [3]
   Not limited to synthesis but also formal verification, reverse
   engineering, ...
