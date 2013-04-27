# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2013-04-26
# License: Apache V2.0, http://www.apache.org/licenses/LICENSE-2.0

Description
===========
CULIST [coolest] helps to connect devices and systems to LIS (Labour Automation System).

Motivation
==========
As I had to connect a customer's device to an LIS I found out, there are no really usable tool
for analysing and debugging ASTM E1394 (LIS01-A2, LIS2-A2) protocols. Some commercial tools, I've got for evaluation
used to be either old, bad, vendor-specific, expensive or (mostly) all these together.

Features
========
-Client/server TCP connection mode
-Parsing of ASTM-trace files into a tree-like structure
-Displaying each field separately
-All but manufacturer and query records implemented
-LIS01-A2 and LIS2-A2

TODOs
=====
-Load/store projects
-Message editing/saving

Future goals
============
Currently ASTM protocol in TCP mode is implemented. Serial connection and other protocols like HL7 are the future goals. 