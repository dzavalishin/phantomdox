
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

Source codes for this book can be found in `Phantom OS GitHub`_ repository.

.. _Phantom OS GitHub: https://github.com/dzavalishin/phantomuserland/issues/

.. only:: html

   Package version |version|
   
   You can get this document in `PDF format`_.

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
