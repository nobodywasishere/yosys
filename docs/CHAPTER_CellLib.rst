.. _chapter:celllib:

Internal Cell Library
=====================

Most of the passes in Yosys operate on netlists, i.e. they only care
about the RTLIL::Wire and RTLIL::Cell objects in an RTLIL::Module. This
chapter discusses the cell types used by Yosys to represent a
behavioural design internally.

This chapter is split in two parts. In the first part the internal RTL
cells are covered. These cells are used to represent the design on a
coarse grain level. Like in the original HDL code on this level the
cells operate on vectors of signals and complex cells like adders exist.
In the second part the internal gate cells are covered. These cells are
used to represent the design on a fine-grain gate-level. All cells from
this category operate on single bit signals.

RTL Cells
---------

Most of the RTL cells closely resemble the operators available in HDLs
such as Verilog or VHDL. Therefore Verilog operators are used in the
following sections to define the behaviour of the RTL cells.

Note that all RTL cells have parameters indicating the size of inputs
and outputs. When passes modify RTL cells they must always keep the
values of these parameters in sync with the size of the signals
connected to the inputs and outputs.

Simulation models for the RTL cells can be found in the file
``techlibs/common/simlib.v`` in the Yosys source tree.

Unary Operators
~~~~~~~~~~~~~~~

All unary RTL cells have one input port and one output port . They also
have the following parameters:

-  | 
   | Set to a non-zero value if the input is signed and therefore should
     be sign-extended when needed.

-  | 
   | The width of the input port .

-  | 
   | The width of the output port .

Table `[tab:CellLib_unary] <#tab:CellLib_unary>`__ lists all cells for
unary RTL operators.

.. container:: tabular

   | ll Verilog & Cell Type
   | ``Y =  ~A`` & ``$not``
   | ``Y =  +A`` & ``$pos``
   | ``Y =  -A`` & ``$neg``
   | ``Y =  &A`` & ``$reduce_and``
   | ``Y =  |A`` & ``$reduce_or``
   | ``Y =  ^A`` & ``$reduce_xor``
   | ``Y = ~^A`` & ``$reduce_xnor``
   | ``Y =  |A`` & ``$reduce_bool``
   | ``Y =  !A`` & ``$logic_not``

For the unary cells that output a logical value (``$reduce_and``,
``$reduce_or``, ``$reduce_xor``, ``$reduce_xnor``, ``$reduce_bool``,
``$logic_not``), when the parameter is greater than 1, the output is
zero-extended, and only the least significant bit varies.

Note that ``$reduce_or`` and ``$reduce_bool`` actually represent the
same logic function. But the HDL frontends generate them in different
situations. A ``$reduce_or`` cell is generated when the prefix ``|``
operator is being used. A ``$reduce_bool`` cell is generated when a bit
vector is used as a condition in an ``if``-statement or
``?:``-expression.

Binary Operators
~~~~~~~~~~~~~~~~

All binary RTL cells have two input ports and and one output port . They
also have the following parameters:

-  | 
   | Set to a non-zero value if the input is signed and therefore should
     be sign-extended when needed.

-  | 
   | The width of the input port .

-  | 
   | Set to a non-zero value if the input is signed and therefore should
     be sign-extended when needed.

-  | 
   | The width of the input port .

-  | 
   | The width of the output port .

Table `1.1 <#tab:CellLib_binary>`__ lists all cells for binary RTL
operators.

.. container:: tabular

   | ll Verilog & Cell Type
   | ``Y = A  & B`` & ``$and``
   | ``Y = A  | B`` & ``$or``
   | ``Y = A  ^ B`` & ``$xor``
   | ``Y = A ~^ B`` & ``$xnor``
   | ``Y = A << B`` & ``$shl``
   | ``Y = A >> B`` & ``$shr``
   | ``Y = A <<< B`` & ``$sshl``
   | ``Y = A >>> B`` & ``$sshr``
   | ``Y = A && B`` & ``$logic_and``
   | ``Y = A || B`` & ``$logic_or``
   | ``Y = A === B`` & ``$eqx``
   | ``Y = A !== B`` & ``$nex``

.. container::
   :name: tab:CellLib_binary

   .. table:: Cell types for binary operators with their corresponding
   Verilog expressions.

      ========================= =============
      Verilog                   Cell Type
      ========================= =============
      ``Y = A <  B``            ``$lt``
      ``Y = A <= B``            ``$le``
      ``Y = A == B``            ``$eq``
      ``Y = A != B``            ``$ne``
      ``Y = A >= B``            ``$ge``
      ``Y = A >  B``            ``$gt``
      ``Y = A  + B``            ``$add``
      ``Y = A  - B``            ``$sub``
      ``Y = A  * B``            ``$mul``
      ``Y = A  / B``            ``$div``
      ``Y = A  % B`` & ``$mod`` ``$divfloor``
      ``[N/A]``                 ``$modfoor``
      ``Y = A ** B``            ``$pow``
      ========================= =============

The ``$shl`` and ``$shr`` cells implement logical shifts, whereas the
``$sshl`` and ``$sshr`` cells implement arithmetic shifts. The ``$shl``
and ``$sshl`` cells implement the same operation. All four of these
cells interpret the second operand as unsigned, and require to be zero.

Two additional shift operator cells are available that do not directly
correspond to any operator in Verilog, ``$shift`` and ``$shiftx``. The
``$shift`` cell performs a right logical shift if the second operand is
positive (or unsigned), and a left logical shift if it is negative. The
``$shiftx`` cell performs the same operation as the ``$shift`` cell, but
the vacated bit positions are filled with undef (x) bits, and
corresponds to the Verilog indexed part-select expression.

For the binary cells that output a logical value (``$logic_and``,
``$logic_or``, ``$eqx``, ``$nex``, ``$lt``, ``$le``, ``$eq``, ``$ne``,
``$ge``, ``$gt``), when the parameter is greater than 1, the output is
zero-extended, and only the least significant bit varies.

Division and modulo cells are available in two rounding modes. The
original ``$div`` and ``$mod`` cells are based on truncating division,
and correspond to the semantics of the verilog ``/`` and ``%``
operators. The ``$divfloor`` and ``$modfloor`` cells represent flooring
division and flooring modulo, the latter of which is also known as
“remainder” in several languages. See
table `1.2 <#tab:CellLib_divmod>`__ for a side-by-side comparison
between the different semantics.

.. container::
   :name: tab:CellLib_divmod

   .. table:: Comparison between different rounding modes for division
   and modulo cells.

      ============ ======== ========== ======== ============= =============
      Division     Result   Truncating          Flooring      
      \                     ``$div``   ``$mod`` ``$divfloor`` ``$modfloor``
      ``-10 / 3``  ``-3.3`` ``-3``     ``-1``   ``-4``        ``2``
      ``10 / -3``  ``-3.3`` ``-3``     ``1``    ``-4``        ``-2``
      ``-10 / -3`` ``3.3``  ``3``      ``-1``   ``3``         ``-1``
      ``10 / 3``   ``3.3``  ``3``      ``1``    ``3``         ``1``
      ============ ======== ========== ======== ============= =============

Multiplexers
~~~~~~~~~~~~

Multiplexers are generated by the Verilog HDL frontend for
``?:``-expressions. Multiplexers are also generated by the ``proc`` pass
to map the decision trees from RTLIL::Process objects to logic.

The simplest multiplexer cell type is ``$mux``. Cells of this type have
a parameter and data inputs and and a data output , all of the specified
width. This cell also has a single bit control input . If is 0 the value
from the input is sent to the output, if it is 1 the value from the
input is sent to the output. So the ``$mux`` cell implements the
function ``Y = S ? B : A``.

The ``$pmux`` cell is used to multiplex between many inputs using a
one-hot select signal. Cells of this type have a and a parameter and
inputs , , and and an output . The input is bits wide. The input and the
output are both bits wide and the input is \* bits wide. When all bits
of are zero, the value from input is sent to the output. If the
:math:`n`\ ’th bit from is set, the value :math:`n`\ ’th bits wide slice
of the input is sent to the output. When more than one bit from is set
the output is undefined. Cells of this type are used to model “parallel
cases” (defined by using the ``parallel_case`` attribute or detected by
an optimization).

The ``$tribuf`` cell is used to implement tristate logic. Cells of this
type have a parameter and inputs and and an output . The input and
output are bits wide, and the input is one bit wide. When is 0, the
output is not driven. When is 1, the value from input is sent to the
output. Therefore, the ``$tribuf`` cell implements the function
``Y = EN ? A : 'bz``.

Behavioural code with cascaded ``if-then-else``- and ``case``-statements
usually results in trees of multiplexer cells. Many passes (from various
optimizations to FSM extraction) heavily depend on these multiplexer
trees to understand dependencies between signals. Therefore
optimizations should not break these multiplexer trees (e.g. by
replacing a multiplexer between a calculated signal and a constant zero
with an ``$and`` gate).

Registers
~~~~~~~~~

SR-type latches are represented by ``$sr`` cells. These cells have input
ports and and an output port . They have the following parameters:

-  | 
   | The width of inputs and and output .

-  | 
   | The set input bits are active-high if this parameter has the value
     ``1’b1`` and active-low if this parameter is ``1’b0``.

-  | 
   | The reset input bits are active-high if this parameter has the
     value ``1’b1`` and active-low if this parameter is ``1’b0``.

Both set and reset inputs have separate bits for every output bit. When
both the set and reset inputs of an ``$sr`` cell are active for a given
bit index, the reset input takes precedence.

D-type flip-flops are represented by ``$dff`` cells. These cells have a
clock port , an input port and an output port . The following parameters
are available for ``$dff`` cells:

-  | 
   | The width of input and output .

-  | 
   | Clock is active on the positive edge if this parameter has the
     value ``1’b1`` and on the negative edge if this parameter is
     ``1’b0``.

D-type flip-flops with asynchronous reset are represented by ``$adff``
cells. As the ``$dff`` cells they have , and ports. In addition they
also have a single-bit input port for the reset pin and the following
additional two parameters:

-  | 
   | The asynchronous reset is active-high if this parameter has the
     value ``1’b1`` and active-low if this parameter is ``1’b0``.

-  | 
   | The state of will be set to this value when the reset is active.

Usually these cells are generated by the ``proc`` pass using the
information in the designs RTLIL::Process objects.

D-type flip-flops with synchronous reset are represented by ``$sdff``
cells. As the ``$dff`` cells they have , and ports. In addition they
also have a single-bit input port for the reset pin and the following
additional two parameters:

-  | 
   | The synchronous reset is active-high if this parameter has the
     value ``1’b1`` and active-low if this parameter is ``1’b0``.

-  | 
   | The state of will be set to this value when the reset is active.

Note that the ``$adff`` and ``$sdff`` cells can only be used when the
reset value is constant.

D-type flip-flops with asynchronous set and reset are represented by
``$dffsr`` cells. As the ``$dff`` cells they have , and ports. In
addition they also have multi-bit and input ports and the corresponding
polarity parameters, like ``$sr`` cells.

D-type flip-flops with enable are represented by ``$dffe``, ``$adffe``,
``$dffsre``, ``$sdffe``, and ``$sdffce`` cells, which are enhanced
variants of ``$dff``, ``$adff``, ``$dffsr``, ``$sdff`` (with reset over
enable) and ``$sdff`` (with enable over reset) cells, respectively. They
have the same ports and parameters as their base cell. In addition they
also have a single-bit input port for the enable pin and the following
parameter:

-  | 
   | The enable input is active-high if this parameter has the value
     ``1’b1`` and active-low if this parameter is ``1’b0``.

D-type latches are represented by ``$dlatch`` cells. These cells have an
enable port , an input port , and an output port . The following
parameters are available for ``$dlatch`` cells:

-  | 
   | The width of input and output .

-  | 
   | The enable input is active-high if this parameter has the value
     ``1’b1`` and active-low if this parameter is ``1’b0``.

The latch is transparent when the input is active.

D-type latches with reset are represented by ``$adlatch`` cells. In
addition to ``$dlatch`` ports and parameters, they also have a
single-bit input port for the reset pin and the following additional
parameters:

-  | 
   | The asynchronous reset is active-high if this parameter has the
     value ``1’b1`` and active-low if this parameter is ``1’b0``.

-  | 
   | The state of will be set to this value when the reset is active.

D-type latches with set and reset are represented by ``$dlatchsr``
cells. In addition to ``$dlatch`` ports and parameters, they also have
multi-bit and input ports and the corresponding polarity parameters,
like ``$sr`` cells.

.. _sec:memcells:

Memories
~~~~~~~~

Memories are either represented using RTLIL::Memory objects, ``$memrd``,
``$memwr``, and ``$meminit`` cells, or by ``$mem`` cells alone.

In the first alternative the RTLIL::Memory objects hold the general
metadata for the memory (bit width, size in number of words, etc.) and
for each port a ``$memrd`` (read port) or ``$memwr`` (write port) cell
is created. Having individual cells for read and write ports has the
advantage that they can be consolidated using resource sharing passes.
In some cases this drastically reduces the number of required ports on
the memory cell. In this alternative, memory initialization data is
represented by ``$meminit`` cells, which allow delaying constant folding
for initialization addresses and data until after the frontend finishes.

The ``$memrd`` cells have a clock input , an enable input , an address
input , and a data output . They also have the following parameters:

-  | 
   | The name of the RTLIL::Memory object that is associated with this
     read port.

-  | 
   | The number of address bits (width of the input port).

-  | 
   | The number of data bits (width of the output port).

-  | 
   | When this parameter is non-zero, the clock is used. Otherwise this
     read port is asynchronous and the input is not used.

-  | 
   | Clock is active on the positive edge if this parameter has the
     value ``1’b1`` and on the negative edge if this parameter is
     ``1’b0``.

-  | 
   | If this parameter is set to ``1’b1``, a read and write to the same
     address in the same cycle will return the new value. Otherwise the
     old value is returned.

The ``$memwr`` cells have a clock input , an enable input (one enable
bit for each data bit), an address input and a data input . They also
have the following parameters:

-  | 
   | The name of the RTLIL::Memory object that is associated with this
     write port.

-  | 
   | The number of address bits (width of the input port).

-  | 
   | The number of data bits (width of the output port).

-  | 
   | When this parameter is non-zero, the clock is used. Otherwise this
     write port is asynchronous and the input is not used.

-  | 
   | Clock is active on positive edge if this parameter has the value
     ``1’b1`` and on the negative edge if this parameter is ``1’b0``.

-  | 
   | The cell with the higher integer value in this parameter wins a
     write conflict.

The ``$meminit`` cells have an address input and a data input , with the
width of the port equal to parameter times parameter. Both of the inputs
must resolve to a constant for synthesis to succeed.

-  | 
   | The name of the RTLIL::Memory object that is associated with this
     initialization cell.

-  | 
   | The number of address bits (width of the input port).

-  | 
   | The number of data bits per memory location.

-  | 
   | The number of consecutive memory locations initialized by this
     cell.

-  | 
   | The cell with the higher integer value in this parameter wins an
     initialization conflict.

The HDL frontend models a memory using RTLIL::Memory objects and
asynchronous ``$memrd`` and ``$memwr`` cells. The ``memory`` pass
(i.e. its various sub-passes) migrates ``$dff`` cells into the
``$memrd`` and ``$memwr`` cells making them synchronous, then converts
them to a single ``$mem`` cell and (optionally) maps this cell type to
``$dff`` cells for the individual words and multiplexer-based address
decoders for the read and write interfaces. When the last step is
disabled or not possible, a ``$mem`` cell is left in the design.

The ``$mem`` cell provides the following parameters:

-  | 
   | The name of the original RTLIL::Memory object that became this
     ``$mem`` cell.

-  | 
   | The number of words in the memory.

-  | 
   | The number of address bits.

-  | 
   | The number of data bits per word.

-  | 
   | The initial memory contents.

-  | 
   | The number of read ports on this memory cell.

-  | 
   | This parameter is bits wide, containing a clock enable bit for each
     read port.

-  | 
   | This parameter is bits wide, containing a clock polarity bit for
     each read port.

-  | 
   | This parameter is bits wide, containing a transparent bit for each
     read port.

-  | 
   | The number of write ports on this memory cell.

-  | 
   | This parameter is bits wide, containing a clock enable bit for each
     write port.

-  | 
   | This parameter is bits wide, containing a clock polarity bit for
     each write port.

The ``$mem`` cell has the following ports:

-  | 
   | This input is bits wide, containing all clock signals for the read
     ports.

-  | 
   | This input is bits wide, containing all enable signals for the read
     ports.

-  | 
   | This input is \* bits wide, containing all address signals for the
     read ports.

-  | 
   | This input is \* bits wide, containing all data signals for the
     read ports.

-  | 
   | This input is bits wide, containing all clock signals for the write
     ports.

-  | 
   | This input is \* bits wide, containing all enable signals for the
     write ports.

-  | 
   | This input is \* bits wide, containing all address signals for the
     write ports.

-  | 
   | This input is \* bits wide, containing all data signals for the
     write ports.

The ``memory_collect`` pass can be used to convert discrete ``$memrd``,
``$memwr``, and ``$meminit`` cells belonging to the same memory to a
single ``$mem`` cell, whereas the ``memory_unpack`` pass performs the
inverse operation. The ``memory_dff`` pass can combine asynchronous
memory ports that are fed by or feeding registers into synchronous
memory ports. The ``memory_bram`` pass can be used to recognize ``$mem``
cells that can be implemented with a block RAM resource on an FPGA. The
``memory_map`` pass can be used to implement ``$mem`` cells as basic
logic: word-wide DFFs and address decoders.

Finite State Machines
~~~~~~~~~~~~~~~~~~~~~

.. container:: fixme

   Add a brief description of the ``$fsm`` cell type.

Specify rules
~~~~~~~~~~~~~

.. container:: fixme

   Add information about ``$specify2``, ``$specify3``, and ``$specrule``
   cells.

Formal verification cells
~~~~~~~~~~~~~~~~~~~~~~~~~

.. container:: fixme

   Add information about ``$assert``, ``$assume``, ``$live``, ``$fair``,
   ``$cover``, ``$equiv``, ``$initstate``, ``$anyconst``, ``$anyseq``,
   ``$allconst``, ``$allseq`` cells.

.. container:: fixme

   Add information about ``$ff`` and ``$_FF_`` cells.

.. _sec:celllib_gates:

Gates
-----

For gate level logic networks, fixed function single bit cells are used
that do not provide any parameters.

Simulation models for these cells can be found in the file
``techlibs/common/simcells.v`` in the Yosys source tree.

.. container:: tabular

   | ll Verilog & Cell Type
   | ``Y = A`` & ``$_BUF_``
   | ``Y = ~A`` & ``$_NOT_``
   | ``Y = A & B`` & ``$_AND_``
   | ``Y = ~(A & B)`` & ``$_NAND_``
   | ``Y = A & ~B`` & ``$_ANDNOT_``
   | ``Y = A | B`` & ``$_OR_``
   | ``Y = ~(A | B)`` & ``$_NOR_``
   | ``Y = A | ~B`` & ``$_ORNOT_``
   | ``Y = A ^ B`` & ``$_XOR_``
   | ``Y = ~(A ^ B)`` & ``$_XNOR_``
   | ``Y = ~((A & B) | C)`` & ``$_AOI3_``
   | ``Y = ~((A | B) & C)`` & ``$_OAI3_``
   | ``Y = ~((A & B) | (C & D))`` & ``$_AOI4_``
   | ``Y = ~((A | B) & (C | D))`` & ``$_OAI4_``
   | ``Y = S ? B : A`` & ``$_MUX_``
   | ``Y = ~(S ? B : A)`` & ``$_NMUX_``
   | (see below) & ``$_MUX4_``
   | (see below) & ``$_MUX8_``
   | (see below) & ``$_MUX16_``
   | ``Y = EN ? A : 1'bz`` & ``$_TBUF_``
   | ``always @(negedge C) Q <= D`` & ``$_DFF_N_``
   | ``always @(posedge C) Q <= D`` & ``$_DFF_P_``
   | ``always @* if (!E) Q <= D`` & ``$_DLATCH_N_``
   | ``always @* if (E)  Q <= D`` & ``$_DLATCH_P_``

.. container::
   :name: tab:CellLib_gates_adff

   .. table:: Cell types for gate level logic networks (FFs with reset)

      +----------------+----------------+----------------+----------------+
      | :              | :math:`RstLvl` | :math:`RstVal` | Cell Type      |
      | math:`ClkEdge` |                |                |                |
      +================+================+================+================+
      | ``negedge``    | ``0``          | ``0``          | `              |
      |                |                |                | `$_DFF_NN0_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_NN0_`` |
      +----------------+----------------+----------------+----------------+
      | ``negedge``    | ``0``          | ``1``          | `              |
      |                |                |                | `$_DFF_NN1_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_NN1_`` |
      +----------------+----------------+----------------+----------------+
      | ``negedge``    | ``1``          | ``0``          | `              |
      |                |                |                | `$_DFF_NP0_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_NP0_`` |
      +----------------+----------------+----------------+----------------+
      | ``negedge``    | ``1``          | ``1``          | `              |
      |                |                |                | `$_DFF_NP1_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_NP1_`` |
      +----------------+----------------+----------------+----------------+
      | ``posedge``    | ``0``          | ``0``          | `              |
      |                |                |                | `$_DFF_PN0_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_PN0_`` |
      +----------------+----------------+----------------+----------------+
      | ``posedge``    | ``0``          | ``1``          | `              |
      |                |                |                | `$_DFF_PN1_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_PN1_`` |
      +----------------+----------------+----------------+----------------+
      | ``posedge``    | ``1``          | ``0``          | `              |
      |                |                |                | `$_DFF_PP0_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_PP0_`` |
      +----------------+----------------+----------------+----------------+
      | ``posedge``    | ``1``          | ``1``          | `              |
      |                |                |                | `$_DFF_PP1_``, |
      |                |                |                | `              |
      |                |                |                | `$_SDFF_PP1_`` |
      +----------------+----------------+----------------+----------------+

.. container::
   :name: tab:CellLib_gates_dffe

   .. table:: Cell types for gate level logic networks (FFs with enable)

      =============== ============= ==============
      :math:`ClkEdge` :math:`EnLvl` Cell Type
      =============== ============= ==============
      ``negedge``     ``0``         ``$_DFFE_NN_``
      ``negedge``     ``1``         ``$_DFFE_NP_``
      ``posedge``     ``0``         ``$_DFFE_PN_``
      ``posedge``     ``1``         ``$_DFFE_PP_``
      =============== ============= ==============

.. container::
   :name: tab:CellLib_gates_adffe

   .. table:: Cell types for gate level logic networks (FFs with reset
   and enable)

      +-------------+-------------+-------------+-------------+-------------+
      | :mat        | :ma         | :ma         | :m          | Cell Type   |
      | h:`ClkEdge` | th:`RstLvl` | th:`RstVal` | ath:`EnLvl` |             |
      +=============+=============+=============+=============+=============+
      | ``negedge`` | ``0``       | ``0``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_NN0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NN0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NN0N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``0``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_NN0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NN0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NN0P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``1``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_NN1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NN1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NN1N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``1``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_NN1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NN1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NN1P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``0``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_NP0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NP0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NP0N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``0``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_NP0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NP0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NP0P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``1``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_NP1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NP1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NP1N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``1``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_NP1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_NP1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_NP1P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``0``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_PN0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PN0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PN0N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``0``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_PN0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PN0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PN0P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``1``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_PN1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PN1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PN1N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``1``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_PN1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PN1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PN1P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``0``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_PP0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PP0N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PP0N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``0``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_PP0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PP0P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PP0P_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``1``       | ``0``       | ``$_DF      |
      |             |             |             |             | FE_PP1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PP1N_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PP1N_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``1``       | ``1``       | ``$_DF      |
      |             |             |             |             | FE_PP1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FE_PP1P_``, |
      |             |             |             |             | ``$_SDF     |
      |             |             |             |             | FCE_PP1P_`` |
      +-------------+-------------+-------------+-------------+-------------+

.. container::
   :name: tab:CellLib_gates_dffsr

   .. table:: Cell types for gate level logic networks (FFs with set and
   reset)

      =============== ============== ============== ================
      :math:`ClkEdge` :math:`SetLvl` :math:`RstLvl` Cell Type
      =============== ============== ============== ================
      ``negedge``     ``0``          ``0``          ``$_DFFSR_NNN_``
      ``negedge``     ``0``          ``1``          ``$_DFFSR_NNP_``
      ``negedge``     ``1``          ``0``          ``$_DFFSR_NPN_``
      ``negedge``     ``1``          ``1``          ``$_DFFSR_NPP_``
      ``posedge``     ``0``          ``0``          ``$_DFFSR_PNN_``
      ``posedge``     ``0``          ``1``          ``$_DFFSR_PNP_``
      ``posedge``     ``1``          ``0``          ``$_DFFSR_PPN_``
      ``posedge``     ``1``          ``1``          ``$_DFFSR_PPP_``
      =============== ============== ============== ================

.. container::
   :name: tab:CellLib_gates_dffsre

   .. table:: Cell types for gate level logic networks (FFs with set and
   reset and enable)

      +-------------+-------------+-------------+-------------+-------------+
      | :mat        | :ma         | :ma         | :m          | Cell Type   |
      | h:`ClkEdge` | th:`SetLvl` | th:`RstLvl` | ath:`EnLvl` |             |
      +=============+=============+=============+=============+=============+
      | ``negedge`` | ``0``       | ``0``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_NNNN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``0``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_NNNP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``1``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_NNPN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``0``       | ``1``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_NNPP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``0``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_NPNN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``0``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_NPNP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``1``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_NPPN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``negedge`` | ``1``       | ``1``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_NPPP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``0``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_PNNN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``0``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_PNNP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``1``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_PNPN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``0``       | ``1``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_PNPP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``0``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_PPNN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``0``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_PPNP_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``1``       | ``0``       | ``$_DFF     |
      |             |             |             |             | SRE_PPPN_`` |
      +-------------+-------------+-------------+-------------+-------------+
      | ``posedge`` | ``1``       | ``1``       | ``1``       | ``$_DFF     |
      |             |             |             |             | SRE_PPPP_`` |
      +-------------+-------------+-------------+-------------+-------------+

.. container::
   :name: tab:CellLib_gates_adlatch

   .. table:: Cell types for gate level logic networks (latches with
   reset)

      ============= ============== ============== =================
      :math:`EnLvl` :math:`RstLvl` :math:`RstVal` Cell Type
      ============= ============== ============== =================
      ``0``         ``0``          ``0``          ``$_DLATCH_NN0_``
      ``0``         ``0``          ``1``          ``$_DLATCH_NN1_``
      ``0``         ``1``          ``0``          ``$_DLATCH_NP0_``
      ``0``         ``1``          ``1``          ``$_DLATCH_NP1_``
      ``1``         ``0``          ``0``          ``$_DLATCH_PN0_``
      ``1``         ``0``          ``1``          ``$_DLATCH_PN1_``
      ``1``         ``1``          ``0``          ``$_DLATCH_PP0_``
      ``1``         ``1``          ``1``          ``$_DLATCH_PP1_``
      ============= ============== ============== =================

.. container::
   :name: tab:CellLib_gates_dlatchsr

   .. table:: Cell types for gate level logic networks (latches with set
   and reset)

      ============= ============== ============== ===================
      :math:`EnLvl` :math:`SetLvl` :math:`RstLvl` Cell Type
      ============= ============== ============== ===================
      ``0``         ``0``          ``0``          ``$_DLATCHSR_NNN_``
      ``0``         ``0``          ``1``          ``$_DLATCHSR_NNP_``
      ``0``         ``1``          ``0``          ``$_DLATCHSR_NPN_``
      ``0``         ``1``          ``1``          ``$_DLATCHSR_NPP_``
      ``1``         ``0``          ``0``          ``$_DLATCHSR_PNN_``
      ``1``         ``0``          ``1``          ``$_DLATCHSR_PNP_``
      ``1``         ``1``          ``0``          ``$_DLATCHSR_PPN_``
      ``1``         ``1``          ``1``          ``$_DLATCHSR_PPP_``
      ============= ============== ============== ===================

.. container::
   :name: tab:CellLib_gates_sr

   .. table:: Cell types for gate level logic networks (SR latches)

      ============== ============== ============
      :math:`SetLvl` :math:`RstLvl` Cell Type    
      ============== ============== ============
      ``0``          ``0``          ``$_SR_NN_`` 
      ``0``          ``1``          ``$_SR_NP_`` 
      ``1``          ``0``          ``$_SR_PN_`` 
      ``1``          ``1``          ``$_SR_PP_`` 
      ============== ============== ============

Tables `[tab:CellLib_gates] <#tab:CellLib_gates>`__,
`1.4 <#tab:CellLib_gates_dffe>`__, `1.3 <#tab:CellLib_gates_adff>`__,
`1.5 <#tab:CellLib_gates_adffe>`__, `1.6 <#tab:CellLib_gates_dffsr>`__,
`1.7 <#tab:CellLib_gates_dffsre>`__,
`1.8 <#tab:CellLib_gates_adlatch>`__,
`1.9 <#tab:CellLib_gates_dlatchsr>`__ and
`1.10 <#tab:CellLib_gates_sr>`__ list all cell types used for gate level
logic. The cell types ``$_BUF_``, ``$_NOT_``, ``$_AND_``, ``$_NAND_``,
``$_ANDNOT_``, ``$_OR_``, ``$_NOR_``, ``$_ORNOT_``, ``$_XOR_``,
``$_XNOR_``, ``$_AOI3_``, ``$_OAI3_``, ``$_AOI4_``, ``$_OAI4_``,
``$_MUX_``, ``$_MUX4_``, ``$_MUX8_``, ``$_MUX16_`` and ``$_NMUX_`` are
used to model combinatorial logic. The cell type ``$_TBUF_`` is used to
model tristate logic.

The ``$_MUX4_``, ``$_MUX8_`` and ``$_MUX16_`` cells are used to model
wide muxes, and correspond to the following Verilog code:

.. code:: verilog

   // $_MUX4_
   assign Y = T ? (S ? D : C) :
                  (S ? B : A);
   // $_MUX8_
   assign Y = U ? T ? (S ? H : G) :
                      (S ? F : E) :
                  T ? (S ? D : C) :
                      (S ? B : A);
   // $_MUX16_
   assign Y = V ? U ? T ? (S ? P : O) :
                          (S ? N : M) :
                      T ? (S ? L : K) :
                          (S ? J : I) :
                  U ? T ? (S ? H : G) :
                          (S ? F : E) :
                      T ? (S ? D : C) :
                          (S ? B : A);

The cell types ``$_DFF_N_`` and ``$_DFF_P_`` represent d-type
flip-flops.

The cell types ``$_DFFE_[NP][NP]_`` implement d-type flip-flops with
enable. The values in the table for these cell types relate to the
following Verilog code template.

.. code:: verilog

   always @($ClkEdge$ C)
           if (EN == $EnLvl$)
               Q <= D;

The cell types ``$_DFF_[NP][NP][01]_`` implement d-type flip-flops with
asynchronous reset. The values in the table for these cell types relate
to the following Verilog code template, where ``$RstEdge$`` is
``posedge`` if ``$RstLvl$`` if ``1``, and ``negedge`` otherwise.

.. code:: verilog

   always @($ClkEdge$ C, $RstEdge$ R)
           if (R == $RstLvl$)
               Q <= $RstVal$;
           else
               Q <= D;

The cell types ``$_SDFF_[NP][NP][01]_`` implement d-type flip-flops with
synchronous reset. The values in the table for these cell types relate
to the following Verilog code template:

.. code:: verilog

   always @($ClkEdge$ C)
           if (R == $RstLvl$)
               Q <= $RstVal$;
           else
               Q <= D;

The cell types ``$_DFFE_[NP][NP][01][NP]_`` implement d-type flip-flops
with asynchronous reset and enable. The values in the table for these
cell types relate to the following Verilog code template, where
``$RstEdge$`` is ``posedge`` if ``$RstLvl$`` if ``1``, and ``negedge``
otherwise.

.. code:: verilog

   always @($ClkEdge$ C, $RstEdge$ R)
           if (R == $RstLvl$)
               Q <= $RstVal$;
           else if (EN == $EnLvl$)
               Q <= D;

The cell types ``$_SDFFE_[NP][NP][01][NP]_`` implement d-type flip-flops
with synchronous reset and enable, with reset having priority over
enable. The values in the table for these cell types relate to the
following Verilog code template:

.. code:: verilog

   always @($ClkEdge$ C)
           if (R == $RstLvl$)
               Q <= $RstVal$;
           else if (EN == $EnLvl$)
               Q <= D;

The cell types ``$_SDFFCE_[NP][NP][01][NP]_`` implement d-type
flip-flops with synchronous reset and enable, with enable having
priority over reset. The values in the table for these cell types relate
to the following Verilog code template:

.. code:: verilog

   always @($ClkEdge$ C)
           if (EN == $EnLvl$)
               if (R == $RstLvl$)
                   Q <= $RstVal$;
               else
                   Q <= D;

The cell types ``$_DFFSR_[NP][NP][NP]_`` implement d-type flip-flops
with asynchronous set and reset. The values in the table for these cell
types relate to the following Verilog code template, where ``$RstEdge$``
is ``posedge`` if ``$RstLvl$`` if ``1``, ``negedge`` otherwise, and
``$SetEdge$`` is ``posedge`` if ``$SetLvl$`` if ``1``, ``negedge``
otherwise.

.. code:: verilog

   always @($ClkEdge$ C, $RstEdge$ R, $SetEdge$ S)
           if (R == $RstLvl$)
               Q <= 0;
           else if (S == $SetLvl$)
               Q <= 1;
           else
               Q <= D;

The cell types ``$_DFFSRE_[NP][NP][NP][NP]_`` implement d-type
flip-flops with asynchronous set and reset and enable. The values in the
table for these cell types relate to the following Verilog code
template, where ``$RstEdge$`` is ``posedge`` if ``$RstLvl$`` if ``1``,
``negedge`` otherwise, and ``$SetEdge$`` is ``posedge`` if ``$SetLvl$``
if ``1``, ``negedge`` otherwise.

.. code:: verilog

   always @($ClkEdge$ C, $RstEdge$ R, $SetEdge$ S)
           if (R == $RstLvl$)
               Q <= 0;
           else if (S == $SetLvl$)
               Q <= 1;
           else if (E == $EnLvl$)
               Q <= D;

The cell types ``$_DLATCH_N_`` and ``$_DLATCH_P_`` represent d-type
latches.

The cell types ``$_DLATCH_[NP][NP][01]_`` implement d-type latches with
reset. The values in the table for these cell types relate to the
following Verilog code template:

.. code:: verilog

   always @*
           if (R == $RstLvl$)
               Q <= $RstVal$;
           else if (E == $EnLvl$)
               Q <= D;

The cell types ``$_DLATCHSR_[NP][NP][NP]_`` implement d-type latches
with set and reset. The values in the table for these cell types relate
to the following Verilog code template:

.. code:: verilog

   always @*
           if (R == $RstLvl$)
               Q <= 0;
           else if (S == $SetLvl$)
               Q <= 1;
           else if (E == $EnLvl$)
               Q <= D;

The cell types ``$_SR_[NP][NP]_`` implement sr-type latches. The values
in the table for these cell types relate to the following Verilog code
template:

.. code:: verilog

   always @*
           if (R == $RstLvl$)
               Q <= 0;
           else if (S == $SetLvl$)
               Q <= 1;

In most cases gate level logic networks are created from RTL networks
using the ``techmap`` pass. The flip-flop cells from the gate level
logic network can be mapped to physical flip-flop cells from a Liberty
file using the ``dfflibmap`` pass. The combinatorial logic cells can be
mapped to physical cells from a Liberty file via ABC using the ``abc``
pass.

.. container:: fixme

   Add information about ``$slice`` and ``$concat`` cells.

.. container:: fixme

   Add information about ``$lut`` and ``$sop`` cells.

.. container:: fixme

   Add information about ``$alu``, ``$macc``, ``$fa``, and ``$lcu``
   cells.
