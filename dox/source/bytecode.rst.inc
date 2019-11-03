===================
Byte Code Reference
===================

.. index:: single: bytecode

This section contains a list of all virtual machine bytecode 
operations implemented now.


------------
Flow control
------------

jmp
---

Unconditional jump.


djnz
----

Decrement top of int stack. If top of int stack is not zero – jump to label.

.. note::
 This operation does **not** pop integer stack!


jz
---

Jump if top of int stack is zero.

.. note::
  This operation **does** pop integer stack.

switch
------

``switch shift divisor label...``


.. code::

    // calculate expression on int stack
    switch 0 1 case1 case2 case3;
    // here we’ll come on default (fall through)
    jmp out;

    case1:
    // if expression was 0 we’ll come here
    jmp out;

    case2:
    // if expression was 1 we’ll come here
    jmp out;

    case3:
    // if expression was 2 we’ll come here
    jmp out;

    out:


Top of int stack (popped) is diminished by shift (signed), 
divided by divisor (unsigned) and is used as offset into the 
jump address table. If result is out of bounds, operator falls 
through, else jumps to selected address.


ret
---


Top of object stack is popped and pushed on caller’s stack. 
If stack is empty null object is pushed to caller. All the 
exception catchers pushed in this call are discarded.

Note that every method returns something, even void. If stack
of returning method is empty, VM returns null object for it.


call
----

``call methodIndex numberOfArgs``

Pops ``numberOfArgs`` arguments from object stack, than pops 
object to call method for.

Calls selected method passing given number of args. Top of 
integer stack of called method will have number of arguments 
passed. After the return exactly one object (possibly null) 
will be on the object stack.

static_invoke
-------------

.. index:: single: static_invoke

``static_invoke methodIndex numberOfArgs``

Pops ``numberOfArgs`` arguments from object stack, than pops object 
to call method for, then pops class to call method in. Class is 
 checked to be base class for ``this`` that was passed. 
 
This instruction is supposed to be used for constructor calls.


dynamic_invoke
--------------

.. index:: single: dynamic_invoke

.. code::

    const «arg val»;
    const «arg val»;
    const 2; // 2 args
    // here must be code to bring this object on stack
    const «toString»;
    dynamic_invoke;

Dynamically invoke method by name.



sys
---

.. index:: single: sys
.. index:: single: syscall
.. index:: single: internal


``sys syscall_number``

Executes this object’s embedded method. 
Object must be of internal class. (See `Internal classes`_ section)

.. TODO ref to internal classes & index

Nothing can be guaranteed about ``sys``. As for now all of them return 
something, but it is not enforced by interpreter. Though, for normal 
operation ``sys`` must return some object or throw an exception.

Actual implementation uses for ``sys`` the same calling convention as for
regular methods.

.. sidebar:: Sys is not call

  It is possible to pass parameters to ``sys`` on int stack and receive some 
  return there as well, which is differs from ``call``, which passes 
  data on object stack only. That is so because ``sys`` does not create 
  a call frame and thus all the current stack data is available to its code.

Generally, sys is used as the only content of internal class methods.

Current implementation of ``sys`` is restartable: if ``sys`` is interrupted 
(snapshot happens during ``sys`` execution and OS is rebooted then),
it will be run once again. Note that for that to be possible, C code 
implementing ``sys`` must be running between
``vm_unlock_persistent_memory()`` and ``vm_lock_persistent_memory()`` calls.

.. TODO wiki ref? See also[[BlockingSyscalls]]








---------------
Basic stack ops
---------------

os dup, is dup
--------------

Duplicate stack top – object or integer correspondingly.

os drop, is drop
----------------

Delete (pop and throw away) stack top.

os pull
-------

``os pull displacement``

Copy object *displacement* steps down from top of object 
stack on top of it. Pull 0 is equal to dup.

os load
-------

``os load slot_num``

Load object reference from this object slot (field), push to top of object stack.

is load
-------

``is load slot_num``

Load object reference from this object slot (field), 
assume object to be numeric, push to top of integer stack.


os save
-------

``os save slot_num``

Pop object reference from object stack, store to selected slot (field) of this object.

is save
-------

``is save slot_num``

Pop value from integer stack, convert to object,
store to selected slot (field) of this object.

.. TODO impl ignores prefix, compiler does not use op

os get, is get
--------------

``os get absolute_stack_position``
``is get absolute_stack_position``

Load object from given position of stack, push to top of stack.
Used to access local (stack) variables.

os set, is set
--------------

``set absolute_stack_position``

Pop object, store to selected position of stack. 


os eq
-----

Pop and compare two object references on object stack. If they are the same, 
push non-zero on integer stack, else push zero.


os neq
------

Pop and compare two object references on object stack. 
If they are the same, push zero on integer stack, else push non-zero.


os isnull
---------

Pop object reference. If it is a null, push non-zero on integer stack, else push zero.






---------
Constants
---------


const
-----

.. code::

    const “hi there”;
    // top of ``object`` stack is string object now
    const 32656;
    // top of ``integer`` stack has 32656 now


Push constant on stack top. Special shortcuts for 0 and 1 exist. 
Strings are Unicode UTF-8. 

const_pool
----------

``const_pool 22;``

Load constant number 22 from constants pool. Constants pool is 
loaded from class file. Practically is used instead of string constant,
but supposed to be more widely used.

-----------------------
Getting special objects
-----------------------

.. index:: single: summon


summon this
-----------

Puts ``this`` object reference on stack.

summon thread
-------------

Puts current thread object reference on stack.


summon null
-----------

Put null object on stack.


summon class_name
-----------------

``summon “internal.string”;``

Push corresponding class object on stack.




------------------------
Integer stack operations
------------------------

Integer stack supports long/float/double operations too. 
By default operations are performed for integer type. For other types
byte code operation must be prepended with a prefix.

prefix_long, prefix_float, prefix_double
----------------------------------------

Select numeric data type for the next integer stack operation.

.. code::

    opcode_prefix_double;
    i2o;
    // *object* stack now has ``.internal.double`` object

These prefixes change default integer type for integer stack operations 
to corresponding type. Next integer stack operation is performed for 
given type. If type is 64-bit one (long or double), 2 stack slots are 
used for each operand or result. Note though, that all compare 
operations return int no matter which prefix you use.


fromi, froml, fromf, fromd
--------------------------

.. code::

    const 10;
    prefix_float;
    fromi;
    // numeric stack now has float value 10.0 on top

Convert int stack value from given type to current (int by default, 
or as set by prefix).


i2o, o2i
--------

.. code::

    const 10;
    i2o;
    // ``object`` stack now has integer object
    o2i;
    // value of integer object moved back to numeric stack

These operations can be used to move numeric data between integer and object stacks.






isum, imul
----------

.. code::

    const 10;
    const 5;
    isum;
    // int stack has 15 on top
    const 10;
    imul;
    // int stack has 150 on top

Addition and multiplication.


isubul, isublu, idivul, idivlu
------------------------------

Integer subtraction and division.
Operand order:

* *ul* Subtract Upper from Lower.
* *lu* Subtract Lower from Upper.


iremul, iremlu
--------------

Division remainder (modulo).



ior, iand, ixor, inot
---------------------

Bit wise operations on integer stack top.


 log_or, log_and, log_xor, log_not
 ---------------------------------

logical operations on integer stack top.


ige, ile, igt, ilt
------------------

Comparison operations. Prefix sets just operand types, result is allways integer.


ishl
----

Shift left.



ishr
----

Shift right signed. Fills with sign bit.


ushr
----

Shift right unsigned. Fills with 0.




----------
Exceptions
----------

.. index:: single: exception

Phantom virtual machine supports quite classic set of exception
handling operations. Exception throw instruction accepts any object to 
be thrown. Catcher is waiting for object of given class or any of its 
subclasses.

push catcher
------------

.. code::

    summon “internal.string”;
    push catcher string_thrown;
    // code
    ret;

    string_thrown:
    // on exception of string type we’ll get here with thrown object on stack top
    // do cleanup
    ret;


Top of stack must contain class object. Exceptions of that class 
will be catched here and control will be passed to a label if such 
exception is thrown. Thrown object will be on stack top in this case. 
In general no other assumptions about stack state can be done.


pop catcher
-----------


Last pushed catcher will be deleted. Note that all method's catchers 
will be automatically popped on return.


throw
-----

.. index:: single: throw

.. code::

    const “we have a problem here”;
    throw;

Object on top of stack is thrown (exception is raised). If stack is empty - will 
throw special system-wide object (null?), if already on top of 
call stack - will kill current thread in a bad way.



























------------------
Special operations
------------------

nop
---

Of course.


debug
-----

Prints string argument (if any), and top values of integer and object stack. 
Integer argument (mode) is one byte and upper bit is used internally, so 
don’t pass anything > 0x7F or < 0 – will be stripped.

* Bit 0 (& 0x01) of mode byte turns on debug instruction printing.
* Bit 1 (& 0x02) turns it off.

Not really used now.


new
---

.. code::

    summon ".internal.string";
    new;


Create a new object of given (stack top) class. String, in this case.
No constructor called automatically. Compiler must generate code to call constructor.



cast
----

.. index:: single: cast

.. code::

    // push object to cast
    // push tagret type (class reference)
    cast
    // object reference now refers to specified class's interface

Pops target type from stack, pops object, checks that it has type is 
among parents, pushes object back.

When interfaces will be implemented, will do more - add or remove proxy?



stack_reserve
-------------

Reserve space on object and integer stacks. Used to set stack place
for stack varibles.

arg_count
---------

Parameter of this op is number of args this func waits for.
Operation compares it with integer on istack, and throws message if 
values differ.
 


lock_this, unlock_this
----------------------

Not implemented. Supposed to lock/relsease mutex attached to ``this`` as satellite.

.. TODO index satellite mutex

general_lock, general_unlock
----------------------------

Not implemented. Supposed to lock/relsease mutex at stack top.

.. TODO index satellite mutex





