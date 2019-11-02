==================
What Phantom OS is
==================

To be short:

- Orhogonal persistence. Application does not feel OS shutdown and restart.
  Even abrupt restart. It is guaranteed that application will be restarted in
  consistent state, which is not too old.
- As long as you don't loose your variable, it's state is the same between OS reboots. You don't have (though you can)
  save porgram state to files. It is persistent.
- Managed code. Though Phantom has simple Posix compatibility subsystem, native Phantom applications are running in a bytecode machine.
- Global address space. Phantom OS is an application server. All applications can communicate directly, by sharing objects.

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
