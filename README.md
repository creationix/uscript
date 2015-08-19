# Network Nodes

The following are the three main types of networks in the μScript system.

## HubMaster

The hub-master is a machine on the network that accepts connections from robots
and editors.  The protocol can be done over web-socket or plain TCP.

HubMasters will try make themselves easy to find using two methods.

First they will listen for broadcast UDP hub-master queries and respond with
their local address.  This will work on lans that don't block broadcast UDP.
The clients can then connect directly using TCP.

Also they will attempt to connect to an internet service and publish their
network information (ip addresses with subnets).  The internet service will also
log their public IP address as it sees it.  Clients can then query the service
and it will respond with hub-masters who look close (same public IP, matching
subnets, etc..)

Each device that connects to a hub will choose it's own address from 0-15.
Robots can send and receive simple messages via numbered "slots" indexed 0-15.
Also editors can program the robots via the editing protocol.

## Robot

A robot is a device that understands the byte-code and can execute it.
Typically these are micro-controllers, but can also be things like raspberry PIs
or even tablets or browsers.

## Editor

The editor will be a web-app served from a internet server, but works offline
using web technology (like cache manifest and eventually service workers).

We may make a hybrid version of the editor that runs on mobile devices or
chrome-books and can use UDP to quickly find the local hub-master even when
there is no internet.

# Network Protocols

There are 4 main protocols in this system.

## UDP Discovery

This protocol is simple, a client will broadcast a UDP message to the μScript
port.  If there is a local hub-master, it will respond with it's network
information.  The client may repeat the query several time while trying to
discover a hub.

## Internet Assisted Discovery

If both the master and client have internet connectivity, they can find each
other through the help of an internet service.  The master will connect to the
service via `wss://` and register with it's network information.  A client will
connect the same, but note that it's a client.  The directory will then respond
with likely candidates.  The client will disconnect once it's done with
discovery.

## Robot to Robot Messages

Each robot will have an address from 0-15 and 16 slots numbered 0-15.  Any robot
can write to the slots of any other robot by sending one byte for the
address/slot and more bytes for the actual value.  The hub will proxy the
message to the correct robot and tell it to set the value of the slot.

## Editor Commands

The editor will be able to query the hub about connected robots as well as
control the robots.  It can send down commands to run and get the result.
Also it can write to the robots's filesystem to save persistent code.  We may
add other commands like getting free heap or to restart the device/vm.

# Programming Language

μScript will have it's own simple programming language.  This will be the
assembly that runs on the robots.  It is low-level enough to be implemented as
an interpreter on 8-bit AVR (Arduino) chips with only 2kb of ram, but high-level
enough that kids can write it using a graphical block system in the editor.

The main goal of the language is to control GPIO pins and respond to inputs.

The language will have a portable byte-code that is extremely compact.  

## Registers

The VM has a sliding window of registers.  In any particular function invocation
the window will be able to see 16 slots in the stack as registers 0-15.  This
window will slide during function calls so that subroutines don't clobber state
of their callers.

Also each VM will have 16 global slots where other robots can read and write
values.

## Opcodes

TODO: define opcodes
