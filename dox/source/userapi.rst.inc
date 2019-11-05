====================
Native API Reference
====================

Native Phantom applications API Reference - set of classes
available to user program written for Phantom OS.

Most of kernel features are available to Phantom program with
the help of one of classes from ``.internal`` package.

Usually it is supposed that user code creates instance of ``.internal``
class and uses it to access kernel API. Sometimes there are 
handy wrappers from ``.phantom`` package.

-----------------
Package .internal
-----------------

.. index:: single: internal class

.internal.string
----------------

.. index:: single: .internal.string

It is just the same as any string constant in user program.
You can use it's methods with string constant:

.. code:: java

    "abcd".substring( 3, 1 );

String encoding in Phantom is UTF-8. There will be corresponding
``.internal.wstring`` class for UTF-32 representation.

Most of the methods are self explaining.

.. code:: java

    int equals(var object);
    .internal.string substring( var index : int, var length : int );
    .internal.string concat( var s : string );
    int length();
    int strstr( var s : string );
    int  toInt();
    long toLong();

The next method parses JSON and returns tree of 
correspondinf arrays/directories/leaf objects.

.. code:: java

    .internal.object parseJson();

This method returns not char, but byte at given position
and will be renamed accordingly.

.. code:: java

    int charAt( var index : int );




.internal.stringbuilder
-----------------------

.. index:: single: .internal.stringbuilder


Mutable string optimized for being extended from the tail.

.. code:: java

	int equals(var object)
	.internal.string toString()
	.internal.string substring( var index : int, var length : int )
	int byteAt( var index : int )
	int concat( var s : string )
	int length()
	int strstr( var s : string )

The main difference from .internal.string is that ``concat`` 
does not return new string, but extends ``this`` object.





.internal.binary
----------------

.. index:: single: .internal.binary

Binary data buffer.

.. code:: java

    int  getByte( var index : int );
    void setByte( var index : int, var value : int );
    void setRange( var from : .internal.binary, var toPos : int, var fromPos : int, var ln : int );

.internal.bitmap
----------------

.. index:: single: .internal.bitmap

Graphical image. Used for painting backgrounds and icons in UI code.

Parse PPM (P6) bitmap file contents. Alpha set to 255.

.. code:: java

    void loadFromString(var src : string);

There is a way to import images into the Phantom program:

.. code:: java

    bmpw = new .internal.bitmap();
    bmpw.loadFromString(import "../resources/backgrounds/weather_sun_sm.ppm");
  
Here ``import "file name"`` operator creates a string constant which contains
all the contents of the named file on the computer where we compile the program.

The ``bmpw.loadFromString()`` method converts file contents into an internal 
bitmap representation and then garbage collector disposes the string
which contained file.

Phantom's persistence then keeps contents of bitmap object as long as you need it.

.internal.bootstrap
-------------------

.. index:: single: .internal.bootstrap

This class is used only during OS initialization and is not to be used by 
application code.

.internal.class
---------------

.. index:: single: .internal.class

Note that ``class`` is reserved word in Phantom language, so if you have
to refer to class class, it is done like this: ``.internal."class"``.

Object of this class represents Phantom class. It can be used to check object
type.

.. TODO write me

.internal.cond
--------------

.. index:: single: .internal.cond

Cond syncronization primitive.

Correspondingly, wait for event, wait with timeout, broadcast (wake up all waiters), and 
signal - wake up one waiting thread. 

.. code:: java

    void wait( var toUnlock : .internal.mutex );
    void twait( var toUnlock : .internal.mutex, var waitTimeMsec : int );
    void broadcast();
    void signal();


.internal.mutex
---------------

.. index:: single: .internal.mutex

Mutex synchronization primitive. Take or release mutex.

.. code:: java

    void lock();
    void unlock();

    ...
    mtx.lock();
    // update complex state
    mtx.unlock();
    ...


.internal.directory
-------------------

.. index:: single: .internal.directory

Hash map.

.. code:: java

    int put( var key : .internal.string, var value );
    .internal.object get( var key );
    int remove( var key : .internal.string );
    int size();




Numeric classes
---------------

.. index:: single: .internal.int
.. index:: single: .internal.long
.. index:: single: .internal.float
.. index:: single: .internal.double


Classes ``.internal.int``, ``.internal.long``, ``.internal.float`` and ``.internal.double``
represent numeric types.

The only notable method now is ``toString()``.

.internal.io
------------

.. index:: single: .internal.io

Represents file/device access.

.. code:: java

    void    open( var filename : string );
    void    close();
    void    read( var nBytes : int );
    void    write( var data : string );
    void    seek( var pos: int, var whence: int );
    // ioctl
    void    setAttribute( var name : string, var value : string );
    string  getAttribute( var name : string );



.internal.stat
--------------

.. index:: single: .internal.stat

Access kernel statistics. ``getStat()`` returns statistics information,
first arg is counter number and second is kind of statistics counter value.

Corresponding conunter number values can be found in ``<kernel/stats.h>``. There are no 
Phantom language defines yet, sorry. 

Kind values are:

- 0: Current counter value
- 1: Average value per second
- 2: Value for this kernel boot
- 3: Value for previous boot 
- 4: Value for the whole history of this OS instance

Note that 4-th kind, among others, lets you find out number of
reboots done and snapshots made.

.. code:: java

    class .internal.stat
    {
        int getStat( var nCounter : int, var kind : int );
        int getIdle();
    };

.internal.tcp
-------------

.. index:: single: .internal.tcp

This class will provide access to TCP protocol.
Currently it can be used to execute HTTP connection and get a result.

.. code:: java

    class .internal.tcp
    {
        // Call URL and return http server reply
        .internal.string curl( var host : .internal.string, var headers : .internal.string ) [24] {  }
    };


.internal.thread
----------------

.. index:: single: .internal.thread

Internal representation of thread. User code is not supposed to
create objects of this type. It is possible but meaningless. Though
it is possible to access such an object for own (current) thread.

.. code:: java

    class .internal.thread
    {
        .internal.string toString();

        /**
         *
         * Return (possibly specific for this thread) OS
         * interface object, wchich is used to access public
         * OS services.
         *
         * @See .phantom.osimpl
         *
        **/
        .phantom.osimpl getOsInterface();

        /**
         *
         * Return owner of this thread. For the root OS
         * threads this can be null.
         *
        **/
        .phantom.user getUser();

        /**
         *
         * Return this thread's environment (Unix-style).
         *
        **/
        .phantom.environment getEnvironment();
    };


.internal.time
--------------

.. index:: single: .internal.time

Time related kernel services.

.. code:: java

    class .internal.time
    {
        // classic Unix 32 bit time. TODO make it to be 64?
        .internal.int unixTime();

        void sleepSec( var timeSec : int );
        void sleepMsec( var timeMsec : int );

        // void runLater( var start : .phantom.runnable, var timeMsec : int );
    };


.internal.udp
-------------

.. index:: single: .internal.udp

UDP protocol. Port numbers and IP addresses are in host byte order (usual integers).

.. code:: java

    class .internal.udp
    {
        // Set local port number
        .internal.string bind( var port : .internal.int );

        .internal.string recvFrom( var addr : .internal.int, var port : .internal.int );
        .internal.int sendTo( var data : .internal.string, var addr : .internal.int, var port : .internal.int );
    };


.internal.window
----------------

.. index:: single: .internal.window, window, persistent window

Access GUI subsystem.

.. code:: java

    class window
    {
        int     getXSize();
        int     getYSize();

        int     getX();
        int     getY(); // Get window position on screen

        void    update(); // Bring window image to screen

        // Update window after each paint - flickers a bit. 
        // Enabled by default. Turn off and update manually.
        void    setAutoUpdate( var auto : int );

        void    clear();
        void    fill(var fg : int);

        void    setBg( var bg : int ); // background color
        void    setFg( var fg : int ); // foreground color

        void    drawString( var x : int, var y : int, var s : string );
        void    drawImage( var x : int, var y : int, var img : .internal.object ); // param is bitmap

        // You can put bitmap with basic drawImage and redraw part with this call
        void    drawImagePart( var x : int, var y : int, var img : .internal.object, var xstart : int, var ystart : int, var xsize : int, var ysize : int ); // param is bitmap

        //void    setWinSize( var xsize : int, var ysize : int );
        void    setWinPosition( var x : int, var y : int );

        void    drawLine( var x : int, var y : int, var xsize : int, var ysize : int );
        void    drawBox( var x : int, var y : int, var xsize : int, var ysize : int );

        void    fillBox( var x : int, var y : int, var xsize : int, var ysize : int );
        void    fillEllipse( var x : int, var y : int, var xsize : int, var ysize : int );

        //void    setEventHandler( var handler : .ru.dz.phantom.handler );
        void    setTitle( var title : string );

        // errno
        int     scrollHor( var x : int, var y : int, var xsize : int, var ysize : int, var steps : int );
    };



.internal.world
---------------

.. index:: single: .internal.world

Access misc OS services which can't be accessed through ``.internal`` classes.

.. code:: java

    class .internal.world
    {
        .internal.thread getMyThread();

        // returns errno
        int startThread( var entry : .phantom.runnable, var arg : .internal.object );

        // Print to system logging facility - practically it is a 
        // log window in real OS and stdout in pvm_*.exe
        void log( var msg : .internal.string );
    };










----------------
Package .phantom
----------------

Classes in ``.phantom`` package are not implemented in kernel but 
nevertheless provide basic OS functions too.

.phantom.application
--------------------

.. index:: single: .phantom.application

This is a base class for Phantom OS application program. Instances of this class
are application instances - documents, for example.

.. code:: java

  class .dz.clock extends .phantom.application 
  {
    .internal.bitmap appIcon;

    void clock() // Constructor
    {
      appIcon = new .internal.bitmap();
      appIcon.loadFromString(import "../resources/icons/clock_icon.ppm");
    }

    void run( var arg )
    {
      // Instance of clock is created and thread is run here
      // Create application window, paint clock, etc
    }

    .internal.string getDescription( )
    {
      return "Clock";
    }

    .internal.bitmap getIcon()
    {
      return appIcon;
    }
  }


.phantom.runnable
-----------------

.. index:: single: .phantom.runnable

Abstract base class for various callbacks and code that can be
run in a thread.

.. code:: java

    class runnable
    {
        // They call us here
        void run( var arg @const ) [8]
        {
            throw "Abstract runnable started";
        }
    };


.phantom.environment
--------------------

.. index:: single: .phantom.environment

Similar to Uinx process environment - ``key=value`` settings storage.
Currently not really used.

.. code:: java

    class environment
    {
        int set( var key: .internal.string, var val  : .internal.object );
        .internal.object get(var key: .internal.string );
    };



