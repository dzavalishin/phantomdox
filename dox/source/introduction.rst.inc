==================
What Phantom OS is
==================

To be short:

- Orthogonal persistence. Application does not feel OS shutdown and restart.
  Even abrupt restart. It is guaranteed that application will be restarted in
  consistent state, which is not too old.
- As long as you have reference to any variable, it's state is the same between OS 
  reboots. You don't have (though you can)
  save program state to files. It is persistent.
- Managed code. Native Phantom applications are running in a bytecode machine. 
  (But it is worth to mention that Phantom has simple Posix compatibility subsystem too.)
- Global address space. Phantom OS is an application server. All 
  applications can communicate directly, by sharing objects.

Phantom OS persistence is achieved not by serializing data to files,
but by running all applications in a persistent RAM. You can (and it will be true)
think of Phantom memory subsystem as of a persistent paging engine. All
the memory is paged to disk in a way that lets OS to restore whole memory image 
on restart. Consistently.

Which subsystems Phantom currently includes:

- Kernel itself: threads, synchronization, persistent memory management.
- Bytecode virtual machine - running native applications.
- Posix layer - runs Linux compatible (but not yet persistent) code.
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

Well, here I will try to describe the reasons behind making Phantom OS.

Operating system of these days is, actually, all that is seen by the appication.
It is, in fact, a virtual computer an application is running on.
As such, operating system can create nearly any kind of environment for the program.

But due just to historical reasons traditional operating systems keep to be very
thin wrap around the CPU providing just some drivers and libraries.

One of things that is quite possible but never even thought about is ability to
hide from the program fact of stop and start of OS kernel. It is not very hard to
achieve, and changes situation dramatically.

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
to interchange data with traditional software. Yes and no.

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

Lets imagine we have to paint day to day temperature curve in traditional OS.

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
deployment rules, no setup. Just 5-10 lines of code.

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
abruptly, the next *restart* will continue executing user
programs from last successful *snapshot* point.

As long as *snapshot* is being done and is not finished 
completely OS keeps previous two snapshots intact. After
the new snapshot is made the most old one is released.

It means that at any point in time there is at least one 
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
  Thread priorities, including real-time.

**Mutex/Cond/Sem**
  And spinlock as lowest level primitive. Provide threads 
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

.. TODO need pictures for each subsystem

**Drivers**
  Of course, this is the lowest level which can be directly
  visible to user code. Drivers are divided into block (disk),
  character, network and graphics drivers. Sound device driver is, 
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
  but if you want files - Unix part is a point for filesystems to live in.
  Is able to run more or less POSIX-compatible Elf executables.

**Persistent pager**
  And snapshots subsystem - this is the heart of Phantom OS.
  Makes native application code to believe that OS never reboots.

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
   in Java or Python (this part is experimental yet).

   For Java and Python compilation is done in two stages. First
   native parser compiles code and produces some intermediate
   representation, then ``plc`` consumes it and generates Phantom
   bytecode.


.. figure:: img/PipeLine.*

   Native toolchain of Phantom OS


   Phantom bytecode can be run by Phantom OS kernel or by 
   hosted runtime. In the last case persistence is not (yet?)
   supported.


Example of Phantom program
--------------------------


Lets review a simple Phantom OS program that gets weather information from public service
and draws a diagram of temperature with the one minute step.

.. code:: java

    package .ru.dz.demo;
    
    import .phantom.os;
    import .internal.window;
    import .internal.bitmap;
    import .internal.tcp;
    import .internal.time;
    import .internal.directory;
    import .internal.long;
    
    
    class weather
    {
        var xpos : int;
        var ypos : int;
    
        var win   : .internal.window;
        var sleep : .internal.time;
        var http  : .internal.tcp;
    
        var itemp : .internal.long;
    
    
        void run( void )
        {
            var bmp         : .internal.bitmap;
            var bmpw        : .internal.bitmap;
            var json_string : .internal.string;
            var json        : .internal.directory;
            var jtmp        : .internal.directory;
    
            bmp = new .internal.bitmap();
            bmp.loadFromString(getBackgroundImage());
    
            win = new .internal.window();
    
            win.setWinPosition(650,500);
    
            win.setTitle("Weather");
    
            win.setFg(0xFF93CDB4); // light green
            win.setBg(0xFFccccb4); // light milk
    
            win.clear();
            win.drawImage( 0, 0, bmp );
    
            bmpw = new .internal.bitmap();
            bmpw.loadFromString(import
                "../resources/backgrounds/weather_sun_sm.ppm");
            win.drawImage( 17, 102, bmpw );
            win.update();
    
            sleep = new .internal.time();
            http = new .internal.tcp();
    
            xpos = 17;
            while(1)
            {
                json_string = http.curl(
                    "http://api.weather.yandex.ru/v1/forecast?extra=true&limit=1",
                    "X-Yandex-API-Key: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\r\n" );
    
                json = json_string.parseJson();
                jtmp = json.get("fact");
                itemp = jtmp.get("temp");
    
                win.drawImagePart( 0, 0, bmp, 250, 240, 120, 22 );
    
                win.setFg(0); // black
                win.drawString( 280, 235, itemp.toString() );
                win.drawString( 250, 235, "T =" );
    
                ypos = 15 + (itemp * 2);
    
                win.setFg(0xFF93CDB4); // light green
                if( ypos < 70 )
                    win.fillBox( xpos, ypos, 2, 2 );
    
    
                sleep.sleepSec( 60 );
    
                xpos = xpos+1;
                if( xpos > 358 ) xpos = 17;
            }
        }

        .internal.string getBackgroundImage()
        {
            return import "../resources/backgrounds/weather_window.ppm" ;
        }

    };


``import .internal.window;``

  Import statements bring in system service classes, such as window.


``win = new .internal.window();``
  Create instance of window. That is a canvas for us to paint on.
  In Phantom there is no paint() or repaint() request or message
  that forces us to repaint window contents. Persistent paradigm
  dictates direct way of communication: program paints into the
  window whenever it is needed and window contents are saved even
  across the OS reboots.

``win.setBg(0xFFccccb4);``
  Do some settings for window, such as foreground and backgroud
  colors, clear it, paint background image.

Now, here is quite an intersting part:

.. code:: java

    bmpw = new .internal.bitmap();
    bmpw.loadFromString(import
        "../resources/backgrounds/weather_sun_sm.ppm");
    win.drawImage( 17, 102, bmpw );

We create a variable of ``.internal.bitmap`` type, which is suitable
for holding a bitmap image.

An ``import ".../rweather_sun_sm.ppm"`` construct is specific for Phantom
language and creates a string constant which contains whole contents of
host computer file.

Phantom OS is a system which can work completely without file system.
If program needs some resource for its work, the resource can be just
stored in a variable. An ``import`` operator creates such a variable
with an asset (file contents) inside.

But it contains a bitmap in a ``.ppm`` file format, and we need a
bitmap object which can be paint onto the window.

Here goes ``bmpw.loadFromString()``, which can parse image file fomat
from string and convert it into the internal representation.

This internal representation will continue to exist as content of
``bmpw`` variable.

Note that:

* We keep assets in program variables, not in files. It is safer as there is
  absolutely no access to those assets from outside of program. No virus or malware
  attack is possible as there is no way to reach our ``bmpw`` variable from outside.

  It is not so critical for a bitmap, but suppose it is not a bitmap but a Lua script?
  Or just a program code? Note that executable code of program is not a file too, it is
  a persistent object that is visible just to Phantom executive subsystem. You can not
  search filesystem and modify ``.exe`` file to implant virus code.

* We brought asset in as an image of some file, which is kept in a string constant.
  But this constant won't consume any memory or disk space as soon as program started,
  because it will be processed by ``bmpw.loadFromString()`` and freed by garbage collector.

* What we will keep as persistent variable value is internal representation of bitmap,
  which is instanly ready for use. After reboot or any number of reboots it will not
  be needed to re-read image file contents, parse file and convert it to bitmap.

  Any asset that is needed by Phantom program can be kept in a persistent variable
  just in a form that is needed by program.

* You don't have to provide a way to build distributive and install a program, deciding
  about which directory it should occupy on target machine and how it will find its assets.
  All the assets are inside. For this program just **one** class image is all the distributive.
  Well, for a more complex program it will be a set of classes (combined in a container,
  of course), but still there will be no problems with locating assets on a target file system.

.. note:: Strings are binaries.

  Phantom .internal.string class can contain any binary content including binary zeroes
  and sometimes is used as a container for complex data or just as a byte buffer.

Ok, lets go furter.

``win.drawImage( 17, 102, bmpw );``

  We paint our bitmap held in a persistent variable into the persistent window.

  Note that this operation is performed just one time in this program's life too.
  The program will start and work as long as user needs it - practically until it
  will be killed by hand.

  Image paint by this line of code will exist in a tagret window forever or as long
  as it is not overpaint by other program statements.


``sleep = new .internal.time();``

  Here we create an instance of class that provides us with OS timing API. Later
  it will be used to sleep for a minute: ``sleep.sleepSec( 60 );``.


``http = new .internal.tcp();``

  This object is a handle to network services. Despite the name this class provides
  not TCP only, but some higher level API.

.. note:: Forever means forever in Phantom

  As it is usually said, ``while(1)`` is a ``forever()`` statement.

  In Phantom OS it is **literally** true. This loop will **never** end
  for the rest of this program's life. Again, as long as user does not kill
  the instance of this program.

``json_string = http.curl( url, header )``

  Here we ask OS default HTTP client to retrieve some web content.
  As you can guess, in this case server returns us a JSON structure
  with current weather infiormation.


``json = json_string.parseJson();``

  Parsing a JSON is a method of string class, because nowadays it is really
  frequently used thing. The returned object is of ``directory`` class, which
  is simply a hash map inside.

``jtmp = json.get("fact");``

  Extract JSON child node. It is a hash map too.

``itemp = jtmp.get("temp");``

  Extract numeric value of node.


``win.drawImagePart( 0, 0, bmp, 250, 240, 120, 22 );``

  Paint part of background image to clear old temperature value string. Note that
  background image is kept in persistent variable too.


``win.drawString( 280, 235, itemp.toString() );``

  Paint new temperature value as sting.

``win.fillBox( xpos, ypos, 2, 2 );``

  Paint next point on temperature diagram.

  Note again that we don't have to keep historical data for temperature and
  repaint diagram on program restart: we are in a persistent OS and previously
  paint data just keep exist in a window.

Of course, this program is (intentionally) extremely simpe, it, for example,
does not scroll diagram image (though that is another 2 lines of code) and
does not check for exceptions, so it will die on any incorrect JSON received.
But that is another 2 lines of code too.

One could ask, is it ok not to keep historical temperature data at all, and
it is a good question.

First of all, it is not a complex thing in Phantom at all. Just add
``tempHistory = new .internal.array()`` and put data in with
``tempHistory.append( itemp )`` - and you will get, practically, a
database. Yes, one variable in Phantom OS can be a database. Simple one, sure.

But, frankly, we just don't need it for *this* program. It's goal is to
paint diagram, and it does. There's no reason for a program to be more complex,
that it has to.

That is a basic idea of Phantom OS programming paradigm. Simple things must be implemented
in a simple way.



--------------------------
Frequently Asked Questions
--------------------------

**Q:** Traditional program starts and stops and when stopped, code of program can be updated,
a new version of program intalled. If program does not stop, how can you update program
code?
  **A:** This question is discussed in a separate chaprer, please refer to `Program update`_.

**Q:** In classic OS we can restart program if it fails. In 
persistent OS program will run forever and if it failed,
it can not be restarted?
  **A:** Program can be restarted in persistent OS just as you wish.
  The difference is that operating system does not kill
  program on each reboot. It does not forbid user to kill and
  re-run any program.

**Q:** Other systems will use files for long time. How can Phantom
communicate with them if Phantom does not use files?
  **A:** Phantom lets you write native programs that can work without
  files. But still you can use files. Actually, application
  program can ignore all the Phantom OS special features and
  work as if it was in traditional OS. Or use Phantom specific 
  features partially, or mix traditional and Phantom OS style.  

  There are special services in Phantom which simplify writing 
  programs that need to interoperate with others.

.. TODO which features? save to JSON?


.. Persistent OS update


**Q:** There soon will be hardware RAM persistence in all computers and Phantom will not be needed,
every OS will become a persistent OS.
  **A:** It is not that simple. There are problems that must be solved by any OS to really hide
  a reboot from software. This book will give some ideas about these problems and solutions
  impemented in Phantom.


**Q:** Phantom persistent memory is a paging subsystem. It is processing disk IO in a 
way that is not controlled by application. Will it lead to performance degradation?
  **A:** It is usual for modern software to suppose that work set of program 
  is much less than amount of available RAM. It is not supposed that 
  program will heavily rely on paging system to move pages to and from 
  disk. In this situation Phantom's interference with program will be�
  negligible.

  One more thing to note is that amount of disk IO in persistent OS
  depends on amount of modified memory. For hi-performance software
  such as games memory modified is a game state, which is not really
  huge and is modified more or less in place. Good design of persistent
  memory allocator can insure high affinity of modified memory regions.
  It means that snapshot footprint won't grow even if snapshots will
  take longer than usual.
