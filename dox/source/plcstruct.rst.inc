=========================
Phantom Language Compiler
=========================

Structure and implementation details of the Phantom OS
compiler (*PLC*).

Basis of native Phantom OS personality is a bytecode
virtual machine, which, integrated with persistent memory
subsystem guarantees Phantom's ability to restart running
application code after OS restart.

PLC supports two modes of operation. It can work as a 
complete compiler (and process source files written in
Phantom language), or can be a backend for another compiler.

There are two language support projects currently in progress.

First one is translator for Java class files which is intended
to support not only Java language, but also all languages that
are able to work on top of JVM, such as Kotlin, Scala, ...

.. TODO add languages

Second one is translator for Python language. It is based
on Python parser written in Python and generating intermediate
file which is read by PLC and ued as input for PLC code generator.

PLC pipeline
------------


Phantom language parser
-----------------------


Java class file loader
----------------------


Python frontend connector
-------------------------


Code tree prerprocessing
------------------------


Bytecode generation
-------------------


Phantom class file
------------------




