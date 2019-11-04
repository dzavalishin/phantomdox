========================
Bytecode Virtual Machine
========================

Phantom bytecode virtual machine (*VM*) is an interpreter
which executes Phantom bytecode. Main components of VM are:

* Main code execution loop
* Class loader and cache
* Internal classes engine
* Syscall restart engine
* Root object and services 
* Zero state creation code
* Stacks subsystem
* Object allocator
* Garbage collectors

--------------
Code execution
--------------

Phantom VM executes bytecode in threads. Each thread is native kernel
thread and corresponding VM `.internal.thread`_ object. During snapshot 
thread state (current *this*, method *ordinal* and *instruction pointer*) 
is stored in this object, and that is the state thread will be restarted
on kernel reboot.

Main code execution loop
------------------------

.. index:: single: bytecode

Heart of VM. Reads and executes bytecode instrutctions.


Stacks subsystem
----------------

.. index:: single: VM stack

There are four stacks in Phantom VM. Object stack,
integer (numeric) stack, call stack and exception
handlers stack.

Call stack is used when method call or return instruction
is executed. Each stack record is a call frame for a running
method. Call frame keeps reference to three other stacks for 
this call frame.

Object stack is main stack for code execution. Most of
bytecode operations read input from and put output to
this stack. For example, method call reads parameters
from object stack and puts func return value here too.

.. sidebar:: Numerics on object stack

  Numeric values can reside on object stack too.
  Boxed (object form) integers, longs, floats and doubles
  are used when we store them in objects or pass as 
  arguments.

Numeric stack. This stack can contain 32 or 64 (uses two 
slots) numeric values: ints, longs, floats and doubles.

Int and float are allways 32 bits, long and double are allways
64 bits. Longer numerics can be processed in object form only.

Numeric stack contains unboxed numeric binaries and is used
for numeric and boolean operations.

---------------------
Execution environment
---------------------

Class loader and cache
----------------------

.. index:: single: class loader

VM code looks for classes in 3 places:

* List of internal classes compiled into the VM implemenation.
* Boot level class loader which looks for classes in 
  kernel module provided with kernel. It is a usual 
  source for non-internal OS provided classes.
* User level class loader registered on OS startup.

Internal classes engine
-----------------------

.. index:: single: internal class

Internal classes are classes provided by kernel.
All of them have name starting with ``.internal``, 
like ``.internal.string``.

These classes connect VM with OS kernel and provide
different API entry points.

Most of intrnal classes have to recreate their objects
state on kernel restart.

Syscall restart engine
----------------------

.. index:: single: restart

Blocking or just long running calls from VM code to
kernel can not survive snapshot and restart and arranged 
to be re-run if interrupted by OS reboot.

.. TODO write more

Root object and services 
------------------------

.. index:: single: root object 

Phantom object land has one special object called root object.

Root object is used at restart to find some key objects of
running OS instance.

* Class objects for very frequently used classes
* List of running VM threads
* Objects that need attention on kernel restart
* User mode class loader
* Root kernel environment (key=val) list
* Kernel persistent stats object
* Class cache map
* Named object directory

Zero state creation code
------------------------

If OS instance starts for the first time, minimal object land
environmet has to be created for the first class to be loaded
and be able to build the rest of object land.

Such an environment is created *manually* by the VM init code
and results in a few very basic classes to be created in 
persistent memory.

------------------------
VM and persistent memory
------------------------



Object allocator
----------------

.. index:: single: object allocator

Object allocator is memory allocation code that works in
the persistent memory.

Object allocator is working with memory arenas. Each arena
is dedicated to either objects of some size or object of 
some kind. For example, there is arena for integer objects
which are all of the same size and are being allocated a
lot.



Garbage collectors
------------------

.. index:: single: garbage collector

There are two garbage collectors in Phantom OS.

Fast and not ideal one is based on reference counts. It 
is good at releasing short living objects fast and
provides for quick object memory turn around.

Slow GC is a classic mark/sweep algorythm, and, depending
on persistent memory size, can take very long to complete.


