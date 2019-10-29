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

