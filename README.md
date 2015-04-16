# Copyright 2013: Valentin Heinitz, www.heinitz-it.de
# CULIST [coolest] Comprehensive, usable LIS tool
# Author: Valentin Heinitz, 2015-04-16
# License: MIT

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
-LIS01-A2 and LIS2-A2

Future goals
============
-Serial connection
-HL7
-Monitoring Files/Directories for sending/receiving actions

TODOs
=====
-Show current project in status or window-title
-Save project as new Project (started on scratch)
-Project should have a name (create simple New ProJect wizard)
-Load/save all project configs, server, etc.
-New view for displaying traces
-New view for editing profiles
-Select Project's profile in Settings.
-Impelement Serial connection
-Display current connection status
-Display local/server IP, Serial Port
-Implement connection-test (Enq-Ack-EOT)
-Help
-Assign accoustic signals to events (LOW)
-DB-connection
-Select LIS2-A2(single frame) / LIS01-A1
-Record Termination with <CR> or <CR><LF> possible
-Show session/message contents when clicked LIS01-A1/LIS2-A2
-Add comments in traces
-Implement configurable timing settings:
 -repeat time (15s)
 -repeat number (6)
 -record pause (0s)
 -message pause (20ms)
 -response pause (2s)
-Implement configurable settings:
 -use fixed delimeters
 -use received delimeters
-Implement configurable error simulation:
 -wrong CS
 -wrong framenr
 -wrong length (>240, no ETB)
 -Responses on ENQ
  -ACK, NAK, ENQ, None x n-tme
 -Responses on Record
  -ACK, NAK, None x n-tme
 -Incomplete record
  -no STX, CR, ETX/ETB, CR, LF
-Store traces in internal DB for analysis
 -Export this DB
 -Create reports
-Export data as html
-Create profiles from traces (used messages, fields)
 -Warn user, when too less data for reliable statistics
-Sniff on Serial, Ethernet



CHANGELOG
=========
2013-0-02 
	Load/store projects
	Message editing/saving
	Manufacturer and query records implemented