Yosys by example – Beyond Synthesis
===================================

.. container:: frame

.. container:: frame

   Overview This section contains 2 subsections:

   -  Interactive Design Investigation

   -  Symbolic Model Checking

Interactive Design Investigation
--------------------------------

.. container:: frame

.. container:: frame

   Yosys can also be used to investigate designs (or netlists created
   from other tools).

   -  The selection mechanism (see slides “Using Selections”),
      especially patterns such as ``%ci`` and ``%co``, can be used to
      figure out how parts of the design are connected.

   -  Commands such as ``submod``, ``expose``, ``splice``, …can be used
      to transform the design into an equivalent design that is easier
      to analyse.

   -  Commands such as ``eval`` and ``sat`` can be used to investigate
      the behavior of the circuit.

.. container:: frame

   Example: Reorganizing a module

   .. container:: columns

      .. code:: verilog

         module scrambler(
                 input clk, rst, in_bit,
                 output reg out_bit
         );
             reg [31:0] xs;
             always @(posedge clk) begin
             	if (rst)
         	    xs = 1;
                 xs = xs ^ (xs << 13);
                 xs = xs ^ (xs >> 17);
                 xs = xs ^ (xs << 5);
                 out_bit <= in_bit ^ xs[0];
             end
         endmodule

      .. code:: ys

         read_verilog scrambler.v

         hierarchy; proc;;

         cd scrambler
         submod -name xorshift32 \
                    xs %c %ci %D %c %ci:+[D] %D \
                    %ci*:-$dff xs %co %ci %d

   .. image:: PRESENTATION_ExOth/scrambler_p01.pdf
      :alt: image
      :width: 11cm

   .. image:: PRESENTATION_ExOth/scrambler_p02.pdf
      :alt: image
      :width: 11cm

.. container:: frame

   Example: Analysis of circuit behavior

   .. code:: ys

      > read_verilog scrambler.v
      > hierarchy; proc;; cd scrambler
      > submod -name xorshift32 xs %c %ci %D %c %ci:+[D] %D %ci*:-$dff xs %co %ci %d

      > cd xorshift32
      > rename n2 in
      > rename n1 out

      > eval -set in 1 -show out
      Eval result: \out = 270369.

      > eval -set in 270369 -show out
      Eval result: \out = 67634689.

      > sat -set out 632435482
      Signal Name                 Dec        Hex                                   Bin
      -------------------- ---------- ---------- -------------------------------------
      \in                   745495504   2c6f5bd0      00101100011011110101101111010000
      \out                  632435482   25b2331a      00100101101100100011001100011010

Symbolic Model Checking
-----------------------

.. container:: frame

.. container:: frame

   Symbolic Model Checking (SMC) is used to formally prove that a
   circuit has (or has not) a given property.

   One application is Formal Equivalence Checking: Proving that two
   circuits are identical. For example this is a very useful feature
   when debugging custom passes in Yosys.

   Other applications include checking if a module conforms to interface
   standards.

   The ``sat`` command in Yosys can be used to perform Symbolic Model
   Checking.

.. container:: frame

   Example: Formal Equivalence Checking (1/2) Remember the following
   example? 1em

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

   Lets see if it is correct..

.. container:: frame

   Example: Formal Equivalence Checking (2/2)

   .. code:: ys

      # read test design
      read_verilog techmap_01.v
      hierarchy -top test

      # create two version of the design: test_orig and test_mapped
      copy test test_orig
      rename test test_mapped

      # apply the techmap only to test_mapped
      techmap -map techmap_01_map.v test_mapped

      # create a miter circuit to test equivalence
      miter -equiv -make_assert -make_outputs test_orig test_mapped miter
      flatten miter

      # run equivalence check
      sat -verify -prove-asserts -show-inputs -show-outputs miter

   …

   ::

      Solving problem with 945 variables and 2505 clauses..
      SAT proof finished - no model found: SUCCESS!

.. container:: frame

   Example: Symbolic Model Checking (1/2) The following AXI4 Stream
   Master has a bug. But the bug is not exposed if the slave keeps
   ``tready`` asserted all the time. (Something a test bench might do.)

   Symbolic Model Checking can be used to expose the bug and find a
   sequence of values for ``tready`` that yield the incorrect behavior.

   -1em

   .. container:: columns

      .. code:: verilog

         module axis_master(aclk, aresetn, tvalid, tready, tdata);
             input aclk, aresetn, tready;
             output reg tvalid;
             output reg [7:0] tdata;

             reg [31:0] state;
             always @(posedge aclk) begin
                 if (!aresetn) begin
         	    state <= 314159265;
         	    tvalid <= 0;
         	    tdata <= 'bx;
         	end else begin
         	    if (tvalid && tready)
         	    	tvalid <= 0;
         	    if (!tvalid || !tready) begin
         	    //             ^- should not be inverted!
                         state = state ^ state << 13;
                         state = state ^ state >> 7;
                         state = state ^ state << 17;
         		if (state[9:8] == 0) begin
         		    tvalid <= 1;
         		    tdata <= state;
         		end
         	    end
         	end
             end
         endmodule

      .. code:: verilog

         module axis_test(aclk, tready);
             input aclk, tready;
             wire aresetn, tvalid;
             wire [7:0] tdata;

             integer counter = 0;
             reg aresetn = 0;

             axis_master uut (aclk, aresetn, tvalid, tready, tdata);

             always @(posedge aclk) begin
             	if (aresetn && tready && tvalid) begin
         	    if (counter == 0) assert(tdata ==  19);
         	    if (counter == 1) assert(tdata ==  99);
         	    if (counter == 2) assert(tdata ==   1);
         	    if (counter == 3) assert(tdata == 244);
         	    if (counter == 4) assert(tdata == 133);
         	    if (counter == 5) assert(tdata == 209);
         	    if (counter == 6) assert(tdata == 241);
         	    if (counter == 7) assert(tdata == 137);
         	    if (counter == 8) assert(tdata == 176);
         	    if (counter == 9) assert(tdata ==   6);
         	    counter <= counter + 1;
         	end
         	aresetn <= 1;
             end
         endmodule

.. container:: frame

   Example: Symbolic Model Checking (2/2)

   .. code:: ys

      read_verilog -sv axis_master.v axis_test.v
      hierarchy -top axis_test

      proc; flatten;;
      sat -seq 50 -prove-asserts

   …with unmodified ``axis_master.v``:

   ::

      Solving problem with 159344 variables and 442126 clauses..
      SAT proof finished - model found: FAIL!

   …with fixed ``axis_master.v``:

   ::

      Solving problem with 159144 variables and 441626 clauses..
      SAT proof finished - no model found: SUCCESS!

Summary
-------

.. container:: frame

   -  Yosys provides useful features beyond synthesis.

   -  The commands ``sat`` and ``eval`` can be used to analyse the
      behavior of a circuit.

   -  The ``sat`` command can also be used for symbolic model checking.

   -  This can be useful for debugging and testing designs and Yosys
      extensions alike.

   .. container:: center

      Questions?

   .. container:: center

      http://www.clifford.at/yosys/
