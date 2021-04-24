.. role:: raw-latex(raw)
   :format: latex
..

.. container:: center

   **Yosys Manual**

   Clifford Wolf

Abstract
========

Most of today’s digital design is done in HDL code (mostly Verilog or
VHDL) and with the help of HDL synthesis tools.

In special cases such as synthesis for coarse-grain cell libraries or
when testing new synthesis algorithms it might be necessary to write a
custom HDL synthesis tool or add new features to an existing one. In
these cases the availability of a Free and Open Source (FOSS) synthesis
tool that can be used as basis for custom tools would be helpful.

In the absence of such a tool, the Yosys Open SYnthesis Suite (Yosys)
was developed. This document covers the design and implementation of
this tool. At the moment the main focus of Yosys lies on the high-level
aspects of digital synthesis. The pre-existing FOSS logic-synthesis tool
ABC is used by Yosys to perform advanced gate-level optimizations.

An evaluation of Yosys based on real-world designs is included. It is
shown that Yosys can be used as-is to synthesize such designs. The
results produced by Yosys in this tests where successfully verified
using formal verification and are comparable in quality to the results
produced by a commercial synthesis tool.

This document was originally published as bachelor thesis at the Vienna
University of Technology :raw-latex:`\cite{BACC}`.

Abbreviations
=============

========== =======================================
AIG        And-Inverter-Graph
ASIC       Application-Specific Integrated Circuit
AST        Abstract Syntax Tree
BDD        Binary Decision Diagram
BLIF       Berkeley Logic Interchange Format
EDA        Electronic Design Automation
EDIF       Electronic Design Interchange Format
ER Diagram Entity-Relationship Diagram
FOSS       Free and Open-Source Software
FPGA       Field-Programmable Gate Array
FSM        Finite-state machine
HDL        Hardware Description Language
LPM        Library of Parameterized Modules
RTLIL      RTL Intermediate Language
RTL        Register Transfer Level
SAT        Satisfiability Problem
VHDL       VHSIC Hardware Description Language
VHSIC      Very-High-Speed Integrated Circuit
YOSYS      Yosys Open SYnthesis Suite
========== =======================================

includeCHAPTER_Intro

includeCHAPTER_Basics

includeCHAPTER_Approach

includeCHAPTER_Overview

includeCHAPTER_CellLib

includeCHAPTER_Prog

includeCHAPTER_Verilog

includeCHAPTER_Optimize

includeCHAPTER_Techmap

includeCHAPTER_Eval

appendix

includeCHAPTER_Auxlibs

includeCHAPTER_Auxprogs

.. _commandref:

Command Reference Manual
========================

includeCHAPTER_TextRtlil

includeCHAPTER_Appnotes

includeCHAPTER_StateOfTheArt

bibliographyliterature

bibliographystylealphadin

bibliographyweblinkweblinks

bibliographystyleweblinkabbrv
