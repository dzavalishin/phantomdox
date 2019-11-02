
==============
Directory tree
==============

GitHub project directory tree:

**oldtree**
  Kernel source code

**phantom**
  Kernel libaries and subsystems source code

  **dev**

    Device drivers. Drivers moved here are more or less stable.
    Drivers that are still in development are in kernel src dir.

  **libc**

    This libc is used for kernel code.

  **libkern**

    Kernel infrastructure components that can't be
    used in user mode.

  **libphantom**

    Kernel components that are used in usermode
    virtual machine builds.

  **libwin**

    Graphics, windows, controls.

  **modules**

    Loadable kernel modules. Mostly experimental.

  **threads**

    Kernel threads subsystem. Also
    sync code - mutex/cond/sema.

  **user**

    Simple usermode support for
    POSIX subsystem.

  **vm**

    Last not least: bytecode virtual machine,
    code of native Phantom personality.


**plib**
  Phantom user mode libraries

**run**
  Setup for running Phantom OS in QEMU

**test**
  Regress tests for various subsystems

  **plc**
  
    Regress test for *plc* compiler. There are
    source files and corresponding compiler listings
    produced with last stable version of compiler.

    New compiler is used to produce its listings and
    lstings are compared to reference ones.

    You can check differences and decide if new verion is
    correcrt. There are two listings - *lst* and *lstc*.
    Former one dumps structure of internal compiler's AST
    like tree with node data types and relations. Latter one
    is an output bytecode listing. 

  **unit**

    Unit tests for kernel C libraries.

  **external**

    These tests are supposed to connect to running kernel by 
    TCP/IP. Implemented as JUnit tests.

**tools**
  Non-kernel code, tools to support OS
  programming. Notably here is *plc*, 
  Phantom Language Compiler.

  **plc**
    Phantom Language Compiler. Also 
    Java to Phantom translator, and 
    preliminary code for Python to 
    Phantom translator.

  **pfsextract**
    Tool to extract persistent memory image
    from Phatom disk image file.



**build**
  Built executables for tools, scripts.  

