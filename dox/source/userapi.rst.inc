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
.internal package
-----------------

.internal.string
----------------

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

.internal.binary
----------------

Binary data buffer.

.. code:: java
    int  getByte( var index : int );
    void setByte( var index : int, var value : int );
    void setRange( var from : .internal.binary, var toPos : int, var fromPos : int, var ln : int );

.internal.bitmap
----------------

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

This class is used only during OS initialization and is not to be used by 
application code.

.internal.class
---------------

Note that ``class`` is reserved word in Phantom language, so if you have
to refer to class class, it is done like this: ``.internal."class"``.

Object of this class represents Phantom class. It can be used to check object
type.

.. TODO write me

.internal.cond
--------------

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

Mutex synchronization primitive. Take or release mutex.

.. code:: java
  void lock();
  void unlock();

  ...
  mtx.lock();
  update complex state
  mtx.unlock();
  ...


.internal.directory
-------------------

Hash map.

.. code:: java
  int put( var key : .internal.string, var value );
  .internal.object get( var key );
  int remove( var key : .internal.string );
  int size();












----------------
.phantom package
----------------

Classes in ``.phantom`` package are not implemented in kernel but 
nevertheless provide basic OS functions too.

.phantom.application
--------------------

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





