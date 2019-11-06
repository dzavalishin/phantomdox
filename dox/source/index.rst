
.. toctree::
   :maxdepth: 3
.. rem    :caption: Contents:


""""""""""""""""""""""""""""
The Phantom Operating System
""""""""""""""""""""""""""""

............
Introduction
............

This book is Phantom OS developer's guide. It contains description of OS internals
and examples on writing kernel components and userland code.

Source codes for OS can be found in `Phantom OS GitHub`_ repository.

Source for this book itself are in `book GitHub`_ repository.

.. _Phantom OS GitHub: https://github.com/dzavalishin/phantomuserland/
.. _book GitHub: https://github.com/dzavalishin/phantomdox

.. only:: html

.. rem   Package version |version|.
   
   You can also get this document in `PDF format`_.

.. _PDF format: https://buildmedia.readthedocs.org/media/pdf/phantomdox/latest/phantomdox.pdf

.. include:: introduction.rst.inc

.................
Project structure
.................

.. include:: directories.rst.inc


.........
Reference
.........

.. include:: userapi.rst.inc

.. include:: kernelapi.rst.inc


......................
Implementation details
......................

.. include:: vmstruct.rst.inc

.. include:: plcstruct.rst.inc

.. include:: bytecode.rst.inc



.....................
Concepts and Problems
.....................


.. include:: concepts.rst.inc


