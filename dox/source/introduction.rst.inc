What Phantom OS is
------------------

To be short:

- Orhogonal persistence. Application does not feel OS shutdown and restart.
  Even abrupt restart. It is guaranteed that application will be restarted in
  consistent state, which is not too old.
- As long as you don't loose your variable, it's state is the same between OS reboots. You don't have (though you can)
  save porgram state to files. It is persistent.
- Managed code. Though Phantom has simple Posix compatibility subsystem, native Phantom applications are running in a bytecode machine.
- Global address space. Phantom OS is an application server. All applications can communicate directly, by sharing objects.

Phantom OS persistence is achieved not by serializing data to files,
but by running all applications in a persistent RAM. You can (and it will be true)
think of Phantom memory subsystem as of a persistent paging engine. All
the memory is paged to disk in a way that lets OS to restore whole memory image 
on restart. Consistently.

Which subsystems Phantom currently includes:

- Kernel itself: threads, syncronization, persistent memory management.
- Bytecode virtual machine - running native applications.
- Posix layer - running Linux compatible (but non yet persistent) code.
- Graphics subsystem - Windows, controls, UI.
- Networking (TCP/IP)
- Phantom language compiler - the most native userland language
- Java to Phantom translator - work in progress
- Python to Phantom translator - just started

Do we need one more OS
----------------------

There are millions of homegrown operating systems around.
Why one more?

Well, here I will try to describe the reasons behing making Phantom OS.


