
In this chapter the most discussed problems of Phantom OS are discussed.

.. warning: This chapter is a discussion

   Most of discussed concepts are not implemented or
   implemented partially. You're very welcome to provide
   input: critics, ideas or, maybe, implementations.


=======================================
Conceptual difficulties of persisten OS
=======================================

.. TODO network exchange
.. TODO functional programming 
.. TODO functional data structures
.. TODO immutable

--------------
Program update
--------------

.. TODO refer to class version - need article on that?

How can one update program code if program never stops?

It seems that it is impossible to answer this quiestion. But we will try.

First of all, it must be noted, that code in Phantom, which is allways 
part of class, can be versioned. Each class can exist in multiple versions
in one OS instance. By default when variables are instantiated, newest
version of class is used. But older ones still exist as long as exist 
old objects that refer to such a class.

It lets us to have simple way to do the first step - bring new code to system.
Developer can just install a new version of code and it will work smoothly -
new objects will be created with new implementation, old ones will continue to 
use code they was created with.

But still - we want already running programs to use new code, do we? There are 
many possible ways to do so.

1. Actually ask user to... stop program and restart it. The fact that Phantom 
   does not stop programs does not prevent us from doing it. We can arrange code
   in a classic ways - here are data objects, and here is a program that uses 
   that data objects. Restarting such a program is re-creating objects it is
   made of.
  
   It looks that we loose in this case all the better features of persistent OS,
   but it is not so. As long as program runs it still can keep its state in 
   persistent objects and gain from doing so.

2. Arrange code so that there is separate state objects and set of processing 
   classes. If we will be creating instances of processing classes on each use,
   update of code will go seamlessly for us.
   
   
   ``rgbImage = new ImageProcessor( grgbImageObject ).convert()``

   Such calls can be hidden by public interface of serving class.


3. Provide means for conversion. Each new class can have copy constructor for
   creating object of that class from the object of previous class version.
   OS can, in this case, run over an object tree for your application and perform
   object replacement, calling your copy constructor for each of objects bottom to top.


4. Convert in place without any support from code.

   The biggest problem of code replacement in run time is references to fields.
   If object structure changes, field displacements in objects can change.
   If we replcae code leaving data intact, new code will access wrong fields.
   
   Having field names and displacements for both old and new code OS can
   update objects, moving data in new positions and clearing fields which are
   new to this version of code.

   This method seems to be the best one. The only thing that it requests from
   application programmer is to be tolerant to absent field values.

   Or, again, provite an update constructor.





