==================
What Phantom OS is
==================

To be short:

- Orhogonal persistence. Application does not feel OS shutdown and restart.
  Even abrupt restart. It is guaranteed that application will be restarted in
  consistent state, which is not too old.
- As long as you have reference to any variable, it's state is the same between OS 
  reboots. You don't have (though you can)
  save porgram state to files. It is persistent.
- Managed code. Though Phantom has simple Posix compatibility subsystem, 
  native Phantom applications are running in a bytecode machine.
- Global address space. Phantom OS is an application server. All 
  applications can communicate directly, by sharing objects.

Phantom OS persistence is achieved not by serializing data to files,
but by running all applications in a persistent RAM. You can (and it will be true)
think of Phantom memory subsystem as of a persistent paging engine. All
the memory is paged to disk in a way that lets OS to restore whole memory image 
on restart. Consistently.

Which subsystems Phantom currently includes:

- Kernel itself: threads, syncronization, persistent memory management.
- Bytecode virtual machine - running native applications.
- Posix layer - running Linux compatible (but non yet persistent) code.
- Graphics subsystem - Windows, controls, UI.
- Networking (TCP/IP)
- Phantom language compiler - the most native userland language
- Java to Phantom translator - work in progress
- Python to Phantom translator - just started

Technical information and links:

- `Source code <https://github.com/dzavalishin/phantomuserland/>`_
- `Wiki <https://github.com/dzavalishin/phantomuserland/wiki>`_
- `Web site <http://phantomos.org>`_

----------------------
Do we need one more OS
----------------------

There are millions of homegrown operating systems around.
Why one more?

Well, here I will try to describe the reasons behing making Phantom OS.

Operating system of these days is, actualy, is all that is seen by the appication.
It is, in fact, is a virtual computer an appication is runing on.
As such, operating system can create nearly any kind of environment for the program.

But due just to historical reasons traditional operating systems keep to be very
thin wrap around the CPU providing just some drivers and libraries.

One of things that is quite possible but never even thought about is ability to
hide from the program fact of stop and start of OS kernel. It is not very hard to
acheve, and changes situation dramatically.

Having this as a goal one can design a persistent environment, which changes game rules
quite a lot. But! Program running in a persistent OS can completely ignore the 
difference between persistent and usual OS environment, not forcing programmer
to learn new tricks.

What is good about persistent environment?

You don't need files any more
-----------------------------

Really. The file is just a tool which helps program to survive 
for the time computer is being turned off. If operating system
hides this situation from you, there is no need to save anything
to file. Any variable is... like file now? It just keeps its value
forever.

What's more interesting - all your complex data structures don't have
to be serialized. You are not limited to structure which simplifies
serializing, and don't have to write corresponding code.

Well, one can say that you still need to be able to save data to file
to interchange data with traditional software. Yes an no.

- You don't have to do it from the very beginning - write your program in
  a simple way and add file operations later.

- You don't have file interface to be complete - sometimes partial save to
  file is ok. For example, program settings and configuration need not to
  be saved in files at all.

- Nowadays a lot of interchange is being done via the Internet API, which is
  different kind of fish.

Don't recreate the environment
------------------------------

Think of the program which works with a lot of, say, TrueType fonts.
In classic OS it takes a lot of time each time it starts to build
list of fonts available scanning files and parsing data structures 
again and again. 

In persistent OS list of fonts you built once is available for you forever.
Just in a form that is ideal for your program.

Don't recreate results
----------------------

In traditional OS every program with UI is a complex combination of code that
builds program state and code that has to re-visualise it through the UI.

Lets imagine we hav to paint day to day temperature curve in traditional OS.

We will need:

- Code that gets new measurements and stores data to database
- Code that extracts data from database
- Code that paints dataset in an OS window 
- Control logic to orchestrate all these parts together
- Some deployment rules and glue settings for program to find its database

Now for the persistent OS:

- Code to read new measurement and put one pixel to the window.

And... that is all. Really.

Window is persistent too, you never have to repaint it. As a result - you 
do not need to store data at all! No repaint - no storage. No database, no 
deplpyment rules, no setup. Just 5-10 lines of code.

What Phantom OS is going to be - an environment where simple goals can be
achieved in a simple way.


Phantom OS lifecycle
--------------------

.. index:: single: snapshot
.. index:: single: restart


Persistent OS lifecycle is differs a bit from traditional
OS.

We differentiate between initial OS instance start and
all of the subsequent kernel starts, which we call 
*restarts*.

There is also an events in the usual life of the kernel 
called *snapshots*. These events store consistent image
of OS userland programs on disk. If kernel will be stopped
abrupty, the next *restart* will continue executing user
programs from last successful *snapshot* point.

As long as *snapshot* is being done and is not finished 
complletely OS keeps previous two snapshots intact. After
the new snapshot is made the most old one is released.

It means that at any point in time there at least one 
complete snapshot available on disk. Most of the time
there are two.

When kernel restarts it is possible to choose if we restart 
from latest or previous snapshot.

Technically it is possible to keep more snapshots, like, for 
example, weekly or monthly ones.

In fact, snapshots engine at the same time is incremental
backup engine.

When Phantom OS instance boots for the first time in its life
and has no snapshot to continue from, it has to create basic
initial state. It includes initial set of classes in 
object land and some initial code running. This code is 
supposed to set up user environment and bring applications to
OS instance.

It is similar in some ways to Unix ``init`` process, but 
``init`` has to set up OS environment at every start of kernel,
which is not needed in Phantom.

Note that all of that is true for native Phantom OS
*personality*. There is also POSIX *personality*, 
which currently is not persistent.


-----------------------
Phantom OS Architecture
-----------------------


.. figure:: img/PhantomComponents.*

   Major components of Phantom OS instance

   From bottom to top are basic kernel engines, main
   kernel subsystems and userland objects.



Basic kernel engines
--------------------

Basic engines provide lowest level services for functional kernel subsystems
and create hardware independent abstractions.

**Threads**
  Preemprive multitasking for kernel threads.

**Scheduler**
  Therad priorities, including real-time.

**Mutex/Cond/Sem**
  And spinlock as basic primitive. Provide threads 
  syncronization and event signalling.

**Physical memory allocator**
  RAM allocation. Co-operates with paging engine to 
  keep supply of free memory by paging memory out.
  Also includes address space allocator which can be used
  to create memory windows or memory page aliases.

**Bus IO**
  Also PCI enumeration. Provides for low level hardware
  communications.

**Interrupts and exceptions**
  Hardware interrupts, context switching, page faults.


**Page table**
  Main low-level engine of persistent memory.


**Cbuf,KHash**
  Basic kernel data structures.


**Pools**
  Reference counting kernel object containers.

**Timing**
  Time of day, timer callouts, etc.


Main kernel subsystems
----------------------

These subsystems are either visible to userland code
through some API (like, for example, TCP/IP), or
provide seamless services - persistent memory is
obvious example.

.. TODO need pictures for all subsystems

**Drivers**
  Of course, this is the lowest level which can be directly
  visible to user code. Drivers are divided into block (disk),
  character, network and graphics drivers. Sound driver is, 
  for example, a character driver.

  Please note driver/device properties engine. It's a phantom 
  specific ioctl like interface.


**Network stack**
  One of the basic building blocks of modern kernel.
  Note that Phantom is able to set up paging over the network.

**Graphics/Windows/Controls/Events**
  There is an integrated kernel-level windowing subsystem in Phantom OS.
  TrueType fonts, bitmap fonts, set of UI components, UTF-8 and -32, etc.

**Main disk IO stack**
  Asyncronous disk IO based on request objects. Partitions, sync IO interface,
  deep integration with paging system, cache support engine, all the stuff.

**Unix emulation layer**
  It is possible to compile Phantom kernel with Unix support turned off,
  but if you want files - Unix prt is a point for filesystems to live in.
  Is able to run more or less POSIX-compatible Elf executables.

**Persistent pager**
  And snapshots subsystem - this is the heart of Phantom OS.
  Makes application code to believe that OS never reboots.

**Internal classes implementation**
  A channel from object land to kernel API.


**Persistent syscalls**
  An engine that resolves problem of kernel reboot during a long syscall
  from persistent program.


**Restart manager**
  Provides means to find all objects that have to be re-connected to kernel on
  reboot.

**Bytecode VM**
  Virtual machine interpretator - executes userland bytecode.


**Persistent memory allocator and GC**
  Even persistent memory should be allocated.


**Kernel class loader**
  Used to initiate object land on the first boot of
  empty OS instance.


Native Toolchain
----------------

   Phantom OS programs can be written in its own language which is
   directly supported by ``plc`` (phantom language compiler) or
   in Java or Python (this part is in experimental stage).

   For Java and Python compilation is done in two stages. First
   native parser compiles code and produces some intermediate
   representation, then ``plc`` consumes it and generates Phantom
   bytecode.


.. figure:: img/PipeLine.*

   Native toolchain of Phantom OS


   Phantom bytecode can be run by Phantom OS kernel or by 
   hosted runtime. In the last case persistence is not (yet?)
   supported.


--------------------------
Frequently Asked Questions
--------------------------

In classic OS we can restart program if it fails. In 
persistent OS program will run forever and if it failed,
it can not be restarted?
  Program can be restarted in persistent OS just as you wish.
  The difference is that operating system does not kill
  program on each reboot. It does not forbid user to kill and
  re-run any program.

Other systems will use files for long time. How can Phantom
communicate wiith them if Phantom does not use files?
  Phantom lets you write native programs that can work without
  files. But still you can use files. Actually, application
  program can ignore all the Phantom OS special features and
  work as if it was in traditional OS. Or use Phantom specific 
  features partially, or mix traditional and Phantom OS style.  

  There are special services in Phantom which simplify writing 
  programs that need to interoperate with others.

.. TODO which features? save to JSON?


.. Persistent OS update


There soon will be hardware RAM persistence in all computers and Phantom will not be needed,
every OS will become a persistent OS.
  It is not that simple. There are problems that must be solved by any OS to really hide
  a reboot from software. This book will give some ideas about these problems and solutions
  impemented in Phantom.




