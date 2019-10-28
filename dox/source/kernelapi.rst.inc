====================
Kernel API Reference
====================

This section is addressed to Phantom OS kernel developer.

--------------------------
Threads and syncronization
--------------------------

Sync primitives:

- Spinlock
- Mutex
- Cond
- Semaphore

--------------------
Allocators and Pools
--------------------

Kernel allocators:

- Main physical memory allocator
- Address space allocator
- P+V allocator
- Kernel heap

Aside from allocators, but used together is pools subsystem.

---------------
Time and timers
---------------

Main system timer
-----------------


Time of day
-----------

Time of day subsystem keeps track of current time and
date.

In-interrupt timer calls
------------------------

It is possible to call some callback after a given time
right from a timer interrupt. Should be used very carefuly.

In-thread timer calls
---------------------

Kernel call can request for a callback in a thread
context to be done after a given time. Such a callback 
should not take long for it will prevent other callbacks
to be executed.

Thread sleep
------------

It is more or less ok for a kernel thread to sleep for
some time.

Relations with snapshots subsystem
----------------------------------

It is forbidden for a kernel code to sleep or wait
for a long time keeping access to a persistent
virtual memory.



-----------------
DPC: Thread pools
-----------------

-------
Disk IO
-------

IO Request
----------

Syncronous disk IO
------------------

Parttitions
-----------

File systems
------------

Native Phantom code can be (and usually is) written
so that it needs no access to file systems. But it
is unrealistic yet and not reasonable to live completely 
without traditional FS access.



-------
Network
-------

Interfaces
----------

DHCP
----

----------------------
Main drivers subsystem
----------------------


-----------------------
Video drivers subsystem
-----------------------

Video driver
------------

Accelerating driver
-------------------

Video driver which can not work by itself and relies
on VESA or some other video driver to set up video mode
and access to frame buffer, but can provide accelerated 
functions.

Mouse cursor
^^^^^^^^^^^^

Support for hardware mouse cursor painting.

Rectangle copy
^^^^^^^^^^^^^^

Copying screen parts.

Rectangle fill
^^^^^^^^^^^^^^

Filing part of screen with color.



--------
Graphics
--------

Windows
-------

Controls
--------

Fonts
-----

----------------
UTF-8 and UTF-32
----------------

Main string encoding in Phantom is UTF-8. But there 
are parts of system (namely - keyboard driver) that 
expect UTF-32 encoding.
Conversion is supported, of course.

-------------------------
Persistent virtual memory
-------------------------

This is the core of native Phantom personality.

--------------
Unix subsystem
--------------

Simple POISX/Linux emulation.

--------------------
Debugging facilities
--------------------


Logging
-------


