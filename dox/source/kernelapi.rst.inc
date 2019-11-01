====================
Kernel API Reference
====================

This section is addressed to Phantom OS kernel developer.

--------------------------
Threads and syncronization
--------------------------

Sync primitives:

- Spinlock
- Mutex
- Cond
- Semaphore

Spinlocks
---------


.. code:: c

    #include <spinlock.h>

    void	hal_spin_init(hal_spinlock_t *sl);
    void	hal_spin_lock(hal_spinlock_t *sl);
    void	hal_spin_unlock(hal_spinlock_t *sl);

Spinlock must be taken with interrupts disabled and for very short time, like
few code lines. Special version of spinlock functions disables and restores
interrupts.

.. code:: c

    void    hal_spin_lock_cli(hal_spinlock_t *sl);
    void    hal_spin_unlock_sti(hal_spinlock_t *sl);

Spinlock can not be placed in paged/persistent memory, because it is
forbidden to switch thread context with spinlock locked. If you absolutely
have to place spinlock into a persistent memory object, use these functions:

.. code:: c

    // Turns off interrupts too
    void    hal_wired_spin_lock(hal_spinlock_t *l);
    void    hal_wired_spin_unlock(hal_spinlock_t *l);

Note that they wire (prevent pageout) memory page(s) spinlock resides in.
It means that locking such a spinlock can cause pagein, which is disk IO
and can take unexpectedly long time.

Note that there is a special ``pvm_spinlock``, which is quite a different kind
of spinlock and is dedicated to use with bytecode virtual machine.

.. TODO describe it here?


Mutex
-----

.. code:: c

    #include <kernel/mutex.h>

    errno_t hal_mutex_init(hal_mutex_t *m, const char *name);
    errno_t hal_mutex_lock(hal_mutex_t *m);
    errno_t hal_mutex_unlock(hal_mutex_t *m);
    errno_t hal_mutex_destroy(hal_mutex_t *m);
    int     hal_mutex_is_locked(hal_mutex_t *m);

    #define ASSERT_LOCKED_MUTEX(m) assert(hal_mutex_is_locked(m))

.. important::
    Be careful using ``hal_mutex_is_locked()``.
    Races possible - mutex can get locked by other thread just after return.
    The only safe use is for ASSERT_LOCKED_MUTEX.



Cond
----

.. code:: c

    #include <kernel/cond.h>

    errno_t hal_cond_init( hal_cond_t *c, const char *name );
    errno_t hal_cond_wait( hal_cond_t *c, hal_mutex_t *m );
    errno_t hal_cond_timedwait( hal_cond_t *c, hal_mutex_t *m, long msecTimeout );
    errno_t hal_cond_signal( hal_cond_t *c );
    errno_t hal_cond_broadcast( hal_cond_t *c );
    errno_t hal_cond_destroy( hal_cond_t *c );

Semaphore
---------

.. index:: single: sem
.. index:: single: semaphore

The main reason for semaphores to exist when we have mutex/cond is that 
semaphore can be signalled (released) from interrupt and if you release it
before waitng thread will wait (attempt to acquire) for it, it will not be
put asleep. 

.. TODO need code example

.. code:: c

    int     hal_sem_init( hal_sem_t *s, const char *name );
    void    hal_sem_release( hal_sem_t *s );
    int     hal_sem_acquire( hal_sem_t *s );
    errno_t sem_get_count(hal_sem_t *s, int *count);
    void    hal_sem_destroy( hal_sem_t *s );
    errno_t hal_sem_zero( hal_sem_t *s );

**hal_sem_acquire**
  Attempt to acquire a semaphore. Try to decrement semaphore
  value. If it becomes less than zero - sleep until semaphore is
  released (incremented) enough times for it to be decremented to zero.

**hal_sem_release**
  Increment semaphore value. If some thread attempted to acquire and
  sleeping in that state waiting for semaphore to become positive,
  wake up that thread.

**sem_get_count**
  Return current semaphore count.

**hal_sem_zero**
  Set semaphore to zero. Next hal_sem_acquire will sleep waiting for
  someone to release.


--------------------
Allocators and Pools
--------------------

Kernel allocators:

- Main physical memory allocator
- Address space allocator
- P+V allocator
- Kernel heap
- Pools


Physical memory allocator
-------------------------

Allocate and free physical computer memory. Allocation unit is page (usually
4094 bytes). Physical memory is usually quite limited resource, don't waste it.

.. warning::
  Allocated physical memory is not mapped into the kernel (or any other)
  address space and can not be accessed just after being allocated. You
  need to map it to some address in virtual address space, or use for 
  physical IO. 

.. code:: c

    errno_t hal_alloc_phys_page(physaddr_t *result);
    void    hal_free_phys_page(physaddr_t page);
    errno_t hal_alloc_phys_pages(physaddr_t *result, int npages);
    void    hal_free_phys_pages(physaddr_t page, int npages);

.. note::
  Before being freed physical memory must be umapped - detouched from address space.

If you need physical memory to be allocated and mapped to some
address, you can use following functions:

.. code:: c

    void hal_pv_alloc( physaddr_t *pa, void **va, int size_bytes );
    void hal_pv_free( physaddr_t pa, void *va, int size_bytes );

**hal_pv_alloc**
  Allocate physmem, allocate address space for it, and map memory to that address space.
  Panics if out of anything.
  Returns physical address of allocated memory in ``pa``, virtual address in
  ``va``, size is increased to be whole number of pages.

**hal_pv_free**
  Unmap, free address space and physical memory.

This set of fuctions is good if you need just big amount of kernel memory.

.. warning::
  If you need buffer to communicate with hardware, do not use these functions.
  Allocate and map memory manually, beacuse you will need special mode of mapping.

Address space allocator
-----------------------

Allocate address space - page address or set of page addresses. This set of
calls **does not allocate memory**, just interval of addresses to be mapped to
some physical memory.

Typical use is to map device memory into the kernel address space.

.. code:: c

    errno_t hal_alloc_vaddress(void **result, int n_pages);
    void hal_free_vaddress(void *addr, int n_pages);

Mapping and unmapping memory
----------------------------

This set of fuctions controls relations between physical memory and
virtual address space. You can call these functions just for address
space allocated with ``hal_alloc_vaddress``, other parts of address 
space are conrolled by kernel in a special way.

.. code:: c

    void hal_page_control( physaddr_t pa, void *va, page_mapped_t mapped, page_access_t access );
    void hal_pages_control( physaddr_t pa, void *va, int n_pages, page_mapped_t mapped, page_access_t access );
    void hal_page_control_etc(
         physaddr_t pa, void *va,
         page_mapped_t mapped, page_access_t access,
         u_int32_t flags
        );
    void hal_pages_control_etc( physaddr_t pa, void *va, int n_pages, page_mapped_t mapped, page_access_t access, u_int32_t flags );

**hal_pages_control_etc**
  Map or unmap (depending on ``mapped`` parameter value) physical memory
  at ``pa`` to address space at ``va``.

Possible values for ``mapped`` parameter are:
- page_unmap: Unmap page(s)
- page_map: Map page of RAM (cached access)
- page_map_io: Map page of device memory (non-cached access)

Possible values for ``access`` parameter:
- page_noaccess: No access (but mapping is set up in pagemap)
- page_readonly: Read only access.
- page_readwrite: Read and write access.

.. TODO flags

Physical memory access
----------------------

Sometimes it is needed to copy data to or from physical memory.
Set of fuctions helps to do it easily.

.. code:: c

    void hal_copy_page_v2p( physaddr_t to, void *from );
    void memcpy_p2v( void *to, physaddr_t from, size_t size );
    void memcpy_v2p( physaddr_t to, void *from, size_t size );
    void memzero_page_v2p( physaddr_t to );


Kernel heap
-----------

Not much to say, use classic ``malloc``, ``calloc`` and ``free`` functions
as usual. Note that size of heap is fixed and you can easily run out of it.
Use ``hal_pv_alloc`` if you need a lot of memory.

Pools
-----

Aside from allocators, but used together is pools subsystem. Pool is an
engine which can keep data structures (referred as *elements*) of one type inside. 
Structures can be 
accessed by handle. Pool keeps reference count of structure usage and can
free it on reference count to become zero. Constructor and destructor 
functions can be registered if needed.

.. code:: c

    #include <kernel/pool.h>

    pool_t *create_pool();
    errno_t destroy_pool(pool_t *);
    pool_t *create_pool_ext( int inital_elems, int arena_size );

.. TODO create_pool_ext

After creating pool you can set additional properties.

.. code:: c

    pool_t fonts_pool = create_pool();

    fonts_pool->init = font_allocation_func;
    fonts_pool->destroy = font_release_func;

    fonts_pool->flag_autoclean = 1;
    fonts_pool->flag_autodestroy = 1;
    fonts_pool->flag_nofail = 1;

**flag_autoclean**
  If pool itself is destroyed, destroy all of contents.

**flag_autodestroy**
  If element reference count becomes zero, destroy element.  

**flag_nofail**
  If anything goes wrong, call ``panic()``, never return an error.
  Don't use this mode.

**init**
  This function is called on element creation. An ``arg`` parameter of
  ``pool_create_el`` is passed to init, and its result is stored in pool.
  This function is, usually, allocates memory and sets up initial values
  for element.

**destroy**
  Called on element destruction. Usually releases related (referenced by
  pool element) resources and deallocates element's memory.

.. code:: c

    pool_handle_t pool_create_el( pool_t *pool, void *arg );
    void *pool_get_el( pool_t *pool, pool_handle_t handle );
    errno_t pool_release_el( pool_t *pool, pool_handle_t handle );
    errno_t pool_destroy_el( pool_t *pool, pool_handle_t handle );

**pool_create_el**
  Create new element in pool. An ``arg`` is poiner to structure to
  be placed in pool as is, or, if ``init`` (constructor) function
  is set for pool, an argument for that function. Reference count for
  new element is 1. Element handle is returned, or ``INVALID_POOL_HANDLE``
  in case of error.

**pool_destroy_el**
  Destroy pool element with given handle, even if reference count
  for it is not zero.

**pool_get_el**
  Get pointer to pool element for given handle, increase reference count.

**pool_release_el**
  Tell pool that we do not use poiner to element any more. Decrease element
  count. Element can be destroyed if count becomes zero.

.. TODO need example


----------
Interrupts
----------

Hardware interrupts
-------------------

.. code:: c

    errno_t hal_irq_alloc( int irq, void (*func)(void *arg), void *arg, int is_shareable );
    void hal_irq_free( int irq, void (*func)(void *arg), void *arg );

**hal_irq_alloc**
  Set interrupt handler.

Software interrupts
-------------------

Software interrupts are called from hardware interrupt handler after all 
hardware interrupts are handled and interrupt state is left. Main use is
thread preemption.

.. code:: c

    void hal_request_softirq( int sirq );
    void hal_set_softirq_handler( int sirq, void (*func)(void *), void *_arg );

    int hal_alloc_softirq(void); // Return next unused softirq number

    void hal_enable_softirq(void);
    void hal_disable_softirq(void);



---------------
Time and timers
---------------

Sleep functions
---------------

.. code:: c

    void hal_sleep_msec( int miliseconds );
    void phantom_spinwait_msec( int miliseconds );
    void tenmicrosec(void);

**hal_sleep_msec**
  Sleep for given time by switching current thread off the CPU.
  It does not guarantee that exact time will pass, for it can take 
  some more time to return thread to CPU.

**phantom_spinwait_msec**
  Sleep by spinning - does not switch threads. You should be disabling
  interrupts for CPU not to be stolen by timer interrupt. Do not use if
  possible.

**tenmicrosec**
  Spin for ~10uSec. Also - disable interrupts.

.. note::
  It is forbidden for a kernel code to sleep or wait
  for a long time keeping access to a persistent
  virtual memory.

.. TODO ref to pers mem lock/unlock

Polled timeouts
---------------

These functions can be used to check if given time is passed.

.. code:: c

    typedef bigtime_t polled_timeout_t;

    void set_polled_timeout( polled_timeout_t *timer, bigtime_t timeout_uSec );
    bool check_polled_timeout( polled_timeout_t *timer );

**set_polled_timeout**
  Set up a timeout.

**check_polled_timeout**
  Check if requested time is passed.

.. code:: c

    polled_timeout_t tmo;
    // start IO
    set_polled_timeout( &tmo, CONST_IO_TIME*2 );
    // Do something
    if( check_polled_timeout( &tmo ) ) return -1; // Failed

Main system timer
-----------------

.. code:: c

    #include <time.h>

    bigtime_t hal_system_time(void); // uptime
    bigtime_t hal_local_time(void);  // real time/date

    time_t    time(time_t *timer);
    void      set_time(time_t time);

    // Fast, but less accurate time, sec
    time_t    fast_time(void);

    // Uptime in seconds
    time_t    uptime(void);

**bigtime_t**
  Time in microseconds.

**time_t**
  Traditional Unix time in seconds.

Time of day
-----------

Time of day subsystem keeps track of current time and
date.

.. code:: c

    time_t mktime(struct tm *);
    size_t strftime(char * , size_t, const char * , const struct tm * );
    char *ctime_r(const time_t *, char *);
    struct tm *gmtime_r(const time_t *, struct tm *);
    struct tm *localtime_rb(bigtime_t timer, struct tm *tmb);


.. TODO describe


In-interrupt timer calls
------------------------

It is possible to call some callback after a given time
right from a timer interrupt. Should be used very carefuly.

.. code:: c

    #include <kernel/timedcall.h>

    typedef void (*timedcall_func_t)( void * );

    typedef struct timedcall
    {
        timedcall_func_t 	f;
        void                *arg;
        long                msecLater;

        hal_spinlock_t *    lockp;

        // Private filds follow

    } timedcall_t;

**f**
  Function to be called by timer.

**arg**
  Parameter to pass to ``f``.

**msecLater**
  Time to pass before ``f`` is called.

.. code:: c

    #define TIMEDCALL_FLAG_PERIODIC         (1 << 0)
    #define TIMEDCALL_FLAG_CHECKLOCK        (1 << 2)

    void phantom_request_timed_call( timedcall_t *entry, u_int32_t flags );
    void phantom_undo_timed_call(timedcall_t *entry);

**phantom_request_timed_call**
  Request call of function after given time is passed. All the
  parameters are to be set up in ``timedcall_t`` structure.

**phantom_undo_timed_call**
  Un-request previously requested timed call. Note that just when
  this function is called timed call may start to execute. So call
  to it does not guarantee that call did not happen. See
  ``TIMEDCALL_FLAG_CHECKLOCK`` for interlocking.

**TIMEDCALL_FLAG_PERIODIC**
  Execute timed call periodically.

**TIMEDCALL_FLAG_CHECKLOCK**
  If this flag is given, timed call won't happen as long as
  spinlock given in ``lockp`` field is locked. This way you 
  can guarantee that timed call is not performed when it is
  not needed anymore and you are going to disable it with 
  ``phantom_undo_timed_call``.


In-thread timer calls
---------------------

Kernel call can request for a callback in a thread
context to be done after a given time. Such a callback 
should not take long for it will prevent other callbacks
to be executed.

.. code:: c

    #include <kernel/net_timer.h>

    int set_net_timer(net_timer_event *e, unsigned int delay_ms, 
        net_timer_callback callback, void *args, int flags);
    int cancel_net_timer(net_timer_event *e);

**set_net_timer**
  Request call to ``callback`` after ``delay_ms``.

**cancel_net_timer**
  Cancel request. Request still can be fired even after call to
  this function. 

-------
Threads
-------

Phantom kernel supports kernel multithreading and quite
powerful multitasking features in general.

.. code:: c

    #include <threads.h>

    typedef struct phantom_thread phantom_thread_t;

    tid_t   hal_start_thread(void (*thread)(void *arg), void *arg, int flags);
    errno_t t_kill_thread( tid_t tid );

Thread properties
-----------------

.. code:: c

    tid_t   get_current_tid(void);

    errno_t t_set_owner( tid_t tid, void *owner );
    errno_t t_get_owner( tid_t tid, void **owner );

    errno_t t_new_ctty( tid_t tid );
    errno_t t_get_ctty( tid_t tid, struct wtty ** );

    errno_t t_set_pid( tid_t tid, pid_t pid );
    errno_t t_get_pid( tid_t tid, pid_t *pid );

    errno_t t_set_priority( tid_t tid, int prio );
    errno_t t_get_priority( tid_t tid, int *prio );

    errno_t t_add_flags( tid_t tid, u_int32_t set_flags );
    errno_t t_remove_flags( tid_t tid, u_int32_t reset_flags );
    errno_t t_get_flags( tid_t tid, u_int32_t *flags );

**t_set_owner**
  Is not interpreted by threads engine, used to connect low level
  thread to bytecode virtual machine threads.

**t_new_ctty**
  Detouch thread's controlling terminal from parent thread's tty.
  It is up to the caller to connect controlling terminal somewhere.

**t_set_pid**
  Used by POSIX subsystem only. Native Phantom code does not use
  concept of processes.

Thread priorities from 1 to 0xF are normal timesharing priorities.
Priority of 0 is idle time execution only. If prio is above
``THREAD_PRIO_MOD_REALTIME`` (0x10 to 0x1F) - thread is real time,
and will take all CPU preventing any lower prio threads to run.
Use with big care.

Following functions control current thread only.

.. code:: c

    errno_t         t_current_set_priority( int prio );
    errno_t         t_current_get_priority( int *prio);

    errno_t         t_current_set_name(const char *name);

    errno_t         t_current_set_death_handler(void (*handler)( phantom_thread_t * ));

Thread flags:

- THREAD_FLAG_USER: Runs in user mode. Not implemented.
- THREAD_FLAG_VM: Runs bytecode virtual machine thread, owner points to VM thread object.
- THREAD_FLAG_JIT: JITted VM tread - not imlpemented
- THREAD_FLAG_TIMEDOUT: Cond (or something else) was timed out.
- THREAD_FLAG_UNDEAD: This thread can't be killed for some reason. Usually it's some special one like CPU idle thread.
- THREAD_FLAG_NOSCHEDULE: Must not be selected by scheduler in usual way - per CPU 'parking' (idlest) thread
- THREAD_FLAG_SNAPPER: I am a snapper thread.
- THREAD_FLAG_HAS_PORT: This thread (possibly) owns port (see newos/ports)

The only flag that can be set with t_add_flags/t_remove_flags is 
``THREAD_FLAG_HAS_PORT``.



DPC: Thread pools
-----------------

DPC (Deferred Procedure Call) requests are supposed to be used in interrupts 
handlers to offload long part of interrpt processing to thread and not to 
keep seprate thread for every function of every driver.

DPC subsystem keeps pool of threads used to handle requests, and adds threads
to pool if all of existing threads are used.

.. code:: c

    #include <kernel/dpc.h>

    void dpc_request_init(dpc_request *rq, void (*_func)(void *));
    void dpc_request_trigger(dpc_request *me, void *_arg);

DPC can be triggered from interrupt and does not require any 
memory allocation or other resources at trigger point.

Example code:

.. code:: c

    dpc_request drq;

    static void driver_init( void )
    {
        dpc_request_init( &drq, &dpc_handler );
    }

    static void dpc_handler( void *arg )
    {
        // Read data from device
    }

    static void interrupt_handler( void *arg )
    {
        if( read_request )
            dpc_request_trigger( &drq, 0 );
    }

Classic thread pool subsystem - can not be triggered from interrupt.

.. code:: c

    typedef void (*runnable_t)( void *arg );
    void run_in_thread_pool( runnable_t func, void *arg );

**run_in_thread_pool**
  Requests ``func`` to be started with ``arg`` in separate thread.

-------
Disk IO
-------

Low level disk IO subsystem is targeted mostly to support paging 
function as main Phantom IO subsystem is persistent memory.

IO Request
----------

.. code:: c

    #include <pager_io_req.h>

    typedef struct pager_io_request
    {
        physaddr_t     phys_page;        	// physmem address
        disk_page_no_t disk_page;        	// disk address in pages - as pager requested (ignored by io code)

        unsigned char  flag_pagein;            // Read
        unsigned char  flag_pageout;           // Write

        void           (*pager_callback)( 
            struct pager_io_request *req, int write );

        errno_t        rc; 

        // Other fields are private

    } pager_io_request;

**phys_page**
  Physical memory address for IO.

**disk_page**
  Disk page address for IO. In 4K pages.  

**flag_pagein**
  If not zero - perform read.

**flag_pageout**
  If not zero - perform write.

**pager_callback**
  If not zero - call when request is done or failed.

**rc**
  Result code. Zero on success.

Syncronous disk IO
------------------

To be described.

.. TODO write

Disk registration
-----------------

Here is an example of disk driver entry point and registering
a new disk in the system.

.. code:: c

    #include <disk_q.h>

    phantom_disk_partition_t *p = phantom_create_virtio_partition_struct( cfg.capacity, &vdev );
    errno_t ret = phantom_register_disk_drive(p);

    static errno_t memdisk_AsyncIo( struct phantom_disk_partition *part, pager_io_request *rq )
    {
        phantom_device_t *dev = part->specific;
        rq->rc = 0;

        size_t size = rq->nSect * part->block_size;
        off_t shift = rq->blockNo * part->block_size;

        if( size+shift > dev->iomemsize )
        {
            rq->rc = EIO;
            pager_io_request_done( rq );
            return EIO;
        }

        if(rq->flag_pageout)
            rq->rc = EIO;
        else
            // read
            memcpy_v2p( rq->phys_page, (void *)dev->iomem+shift, size );

        pager_io_request_done( rq );
        return rq->rc;
    }

    phantom_disk_partition_t *phantom_create_memdisk_partition_struct( 
        phantom_device_t *dev, long size, int unit )
    {
        phantom_disk_partition_t * ret = 
            phantom_create_partition_struct( 0, 0, size );

        ret->asyncIo = memdisk_AsyncIo;
        ret->flags |= PART_FLAG_IS_WHOLE_DISK;

        ret->specific = dev;
        strlcpy( ret->name, "MEMD0", sizeof(ret->name) );
        ret->name[4] += unit;
        return ret;
    }


    static void memdisk_connect( phantom_device_t *dev, int nSect )
    {
        phantom_disk_partition_t *part = 
            phantom_create_memdisk_partition_struct( dev, nSect, 0 );
        if(part == 0)
        {
            SHOW_ERROR0( 0, "Failed to create whole disk partition" );
            return;
        }

        errno_t err = phantom_register_disk_drive(part);
        if(err)
        {
            SHOW_ERROR( 0, "Disk %p err %d", dev, err );
            return;
        }
    }

Now some explanation.

**memdisk_AsyncIo**
  This is driver entry point which is called with a new IO request
  and has to handle it. In this simple example request is proicessed
  right in place, but in real driver this function will, usually,
  just start IO and set things up so that pager_io_request_done() will
  be called in device interrupt.

**pager_io_request_done**
  This function must be called for a reqest when it is processed (with
  either success or error) with driver. Request ``rc`` field must be
  non-zero if request is not executed for some reason.

**phantom_create_memdisk_partition_struct**
  This is a driver's helper function which creates driver descriptor
  structure for a kernel. This structure is used by kernel to access this
  driver for IO. Note ``ret->asyncIo = memdisk_AsyncIo;`` line, which tells
  what function to call for incoming IO request for this driver's device.

**phantom_register_disk_drive**
  Driver init code calls this function to tell kernel that this is a disk
  that is served by this driver. Kernel will try to identify disk contents -
  if there are partitions on disk, kernel will add partition handling structures
  and in turn try to identify partition contents. Finally either disk or partition
  will be recognized as Phantom OS disk, some kind of file system (FAT32 is the
  only one that is supported), or not recognized at all.

Partitions
-----------

The only type of partitioning supported in current kernel is traditional 
PC MBR type one. Additional kinds of disk breakdown can be added easily.

.. TODO give a clue

File systems
------------

Native Phantom code can be (and usually is) written
so that it needs no access to file systems. But it
is unrealistic yet and not reasonable to live completely 
without traditional FS access.

Connecting FS driver
--------------------

Here is an example of how to connect a file system to kernel.

FS probe entry point is called for each whole disk or partition and
for each registered FS:

.. code:: c

    errno_t fs_probe_fat(phantom_disk_partition_t *p )
    {
        unsigned char buf[PAGE_SIZE];

        SHOW_FLOW( 1, "%s look for FAT", p->name );

        switch( p->type )
        {
            case 1: // FAT 12
            case 4: // FAT 16 below 32M
            case 6: // FAT 16 over 32M
            // Check more types we support
            break;
        default:
            SHOW_ERROR( 1, "Not a FAT partition type 0x%X", p->type );
            return EINVAL;
        }

        if( phantom_sync_read_sector( p, buf, 0, 1 ) )
        {
            SHOW_ERROR( 0, "%s can't read sector 0", p->name );
            return EINVAL;
        }

        if( (buf[0x1FE] != 0x55) || (buf[0x1FF] != 0xAA) )
        {
            SHOW_ERROR0( 1, "No magic" );
            return EINVAL;
        }

    // Do more checks, return zero if we decide this is our FS disk

    return 0;
    }

If we said that it is our partiotion, next entry point will be called by kernel:

.. code:: c

    errno_t fs_start_fat( phantom_disk_partition_t *p )
    {
        FATFS *fs = calloc( sizeof(FATFS), 1);

        fs->dev = p;

        // Do FS specific init code

        uufs_t *ufs = fatff_create_fs( fs );
        if( !ufs )
        {
            SHOW_ERROR( 0, "can't create uufs for %s", p->name );
            return ENOMEM;
        }

        char pname[FS_MAX_MOUNT_PATH];
        partGetName( p, pname, FS_MAX_MOUNT_PATH );

        char amnt_path[128];
        if( ufs && auto_mount( pname, ufs, amnt_path, sizeof(amnt_path), AUTO_MOUNT_FLAG_AUTORUN ) )
            SHOW_ERROR( 0, "can't automount %s", p->name );

        return 0;
    }

This code createds specific structure for use inside the FS driver (``FATFS``),
creates uufs_t file system descriptor, and mounts new file system. Note that 
given in partition access structure ``p`` is stored in ``fs->dev`` so that FS
can access its disk partition.

Here is how its done:

.. code:: c

    int disk_write( FATFS *fs, const void* data, int sector, int nsect)
    {

        if( phantom_sync_write_sector( fs->dev, data, sector, nsect ) )
            return RES_ERROR;

        // Put to cache only if successfully written to disk
        if( fs->cache )
            cache_put_multiple( fs->cache, sector, nsect, data );

        return 0;
    }

    static int disk_read ( FATFS *fs, void* data, int sector, int nsect)
    {
        if( fs->cache )
            if( 0 == cache_get_multiple( fs->cache, sector, nsect, data ) )
                return 0;

        if( phantom_sync_read_sector( fs->dev, data, sector, nsect ) )
            return RES_ERROR;

        if( fs->cache )
            cache_put_multiple( fs->cache, sector, nsect, data );

        return 0;
    }

This code also shows how to use disk cache subsystem.


Disk cache subsystem
--------------------

Default disk IO cache machinery. Disk IO subsystem does not
use cache subsystem automatically, you have to add it to your
code manually. The reason is simple - Phantom persistent memory
subsystem is itself a huge cache, and does not need separate IO
cache at all. So adding cache to FS implementation must be done 
in FS code, not in disk IO code.

.. code:: c

    #include <kernel/disk_cache.h>

    cache_t * cache_init( size_t block_size );

    errno_t cache_get( cache_t *c, long blk, void *data );
    errno_t cache_get_multiple( cache_t *c, long blk, int nblk, void *data );

    errno_t cache_put( cache_t *c, long blk, const void *data );
    errno_t cache_put_multiple( cache_t *c, long blk, int nblk, const void *data );

    errno_t cache_set_writeback( cache_t *c, writeback_f_t *func, void *opaque );
    errno_t cache_flush_all( cache_t *c );

    errno_t cache_destroy( cache_t * );

**cache_init**
  Create cache of default size for given page (sector) size.
  Returns cache struct or null on fail (usually out of mem).

**cache_get**
  Find a cache entry and get data from it, or return ENOENT
  if corresponding disk block is not in cache.

**cache_get_multiple**
  Find a cache entry and get data from it, or return ENOENT - multisector.

**cache_put**
  Place data to cache - find or reuse entry as needed.

**cache_put_multiple**
  Place data to cache - find or reuse entry as needed - multisector.

**cache_set_writeback**
  Set function to call if cache needs to write out data from cache
  to disk.

**cache_flush_all**
  Make sure all the cached data is written to disk.

.. warning::  
  It is not guaranteed that cache_flush_all returns after all 
  outstanding data is surely written to disk.

**cache_destroy**
  Destroy cache.


-------
Network
-------

Phantom has classical TCP/IP netwrok stack. 

Interfaces
----------

This chapter describes how to connect new network driver to a
network interface.

Driver entry points for network IO:

.. code:: c

    #include <kernel/ethernet_defs.h>
    #include <kernel/net.h>

    int pcnet32_read( struct phantom_device *dev, void *buf, int len);
    int pcnet32_write(struct phantom_device *dev, const void *buf, int len);
    int pcnet32_get_address( struct phantom_device *dev, void *buf, int len);

**pcnet32_get_address**
  Read network card MAC address.

.. code:: c

    phantom_device_t * dev = malloc(sizeof(phantom_device_t));
    dev->name = "pcnet";
    dev->seq_number = seq_number++;
    dev->dops.read = pcnet32_read;
    dev->dops.write = pcnet32_write;
    dev->dops.get_address = pcnet32_get_address;

    // Init other fields of dev as it is needed

    ifnet *interface;
    if( if_register_interface( IF_TYPE_ETHERNET, &interface, dev) )
        printf("Failed to register interface for %s", dev->name );
    else
        if_simple_setup(interface, WIRED_ADDRESS, WIRED_NETMASK, WIRED_BROADCAST, WIRED_NET, WIRED_ROUTER, DEF_ROUTE_ROUTER );

**if_simple_setup**
  Right now we use some predefined addresses first, but 
  this function starts DHCP to get correct setup.
  It is a hack and must be fixed later.

.. TODO something is wrong here - netmask? def route?

.. code:: c

    #define WIRED_ADDRESS    IPV4_DOTADDR_TO_ADDR(10, 0, 2, 123)
    #define WIRED_NETMASK    0xffffff00
    #define WIRED_BROADCAST  IPV4_DOTADDR_TO_ADDR(10, 0, 2, 0xFF)
    #define WIRED_NET        IPV4_DOTADDR_TO_ADDR(10, 0, 2, 0)
    #define WIRED_ROUTER     IPV4_DOTADDR_TO_ADDR(10, 0, 2, 123)
    #define DEF_ROUTE_ROUTER IPV4_DOTADDR_TO_ADDR(10, 0, 2, 2)



----------------------
Main drivers subsystem
----------------------

These are structures that represent a driver. Just fields that are of
use for a driver are shown, there are more fields, but most of them are
just for kernel.

Driver entry points:

.. code:: c

    #include <device.h>

    struct phantom_dev_ops
    {
        int (*start)(struct phantom_device *dev);
        int (*stop)(struct phantom_device *dev);

        int (*read)(struct phantom_device *dev, void *buf, int len);
        int (*write)(struct phantom_device *dev, const void *buf, int len);

        int (*get_address)(struct phantom_device *dev, void *buf, int len);

        errno_t (*ioctl)(struct phantom_device *dev, int type, void *buf, int len);

        errno_t	(*getproperty)( struct phantom_device *dev, const char *pName, char *pValue, int vlen );
        errno_t	(*setproperty)( struct phantom_device *dev, const char *pName, const char *pValue );
        errno_t	(*listproperties)( struct phantom_device *dev, int nProperty, char *pValue, int vlen );
    };


**start**
  Called if kernel is going to use this device - before first call to any
  other entry point.

**stop**
  Called if no access to driver is going to be any more.

**read**
  Read data from device. For character or network devices only. Block devices register
  themselves for block IO requests separately. 

.. TODO reference

**write**
  Write data to device.

**get_address**
  Special for network devices - must return MAC address.

**ioctl**
  For POSIX subsystem. Classic ioctl.

**getproperty**
  Reach man's ioctl, get device's named property value.
  Can be used from POSIX and object land code.

**getproperty**
  Set device's named property value.

**listproperties**
  Tell names of properties device supports.

.. TODO ref to object interface, ref to POSIX interface

Device descriptor structure:

.. code:: c

    struct phantom_device
    {
        const char *                name;
        int                         seq_number; // Number of the device for the same name (driver)

        void *                      drv_private; // Driver's private part

        phantom_dev_ops_t           dops;

        addr_t                      iobase;
        int                         irq;
        physaddr_t                  iomem;
        size_t                      iomemsize;

        struct properties           *props;
    };

    typedef struct phantom_device phantom_device_t;

    void devfs_register_dev( phantom_device_t* dev );

**name**
  Device name.

**seq_number**
  Sequence number of device, if driver presents more than one.

**drv_private**
  POinter to driver's private data structure which is specific for
  this device.

**dops**
  Entry points, must be filled for kernel to access.

**iobase, irq, iomem, iomemsize**
  Can be used or ignored by driver. Kernel does not interpret.

**props**
  Device properties, see separate description.

.. TODO ref to props description

Device properties
-----------------

.. code:: c

    #include <phantom_types.h>

    typedef enum
    {
        pt_int32,
        pt_mstring,         // malloced string
        pt_enum32,         	// enum int32 - unimpl!
    } property_type_t;

    struct property;

    typedef struct properties {
        const char *          prefix;         // 4-byte char prefix of this group, like 'dev.', 'gen.' or 'fsp.'

        struct property     *list;
        size_t              lsize;

        void *              (*valp)(struct properties *ps, void *context, size_t offset );
    } properties_t;

    typedef struct property {
        property_type_t     type;
        const char 		*name;
        size_t              offset;
        void                *valp;

        char                **val_list; // for enums

        void                (*activate)(struct properties *ps, void *context, size_t offset, void *vp );
        errno_t             (*setf)(struct properties *ps, void *context, size_t offset, void *vp, const char *val);
        errno_t             (*getf)(struct properties *ps, void *context, size_t offset, void *vp, char *val, size_t len);
    } property_t;


    errno_t gen_dev_listproperties( struct phantom_device *dev, int nProperty, char *pValue, int vlen );
    errno_t	gen_dev_getproperty( struct phantom_device *dev, const char *pName, char *pValue, int vlen );
    errno_t	gen_dev_setproperty( struct phantom_device *dev, const char *pName, const char *pValue );

    errno_t gen_listproperties( properties_t *ps, int nProperty, char *pValue, int vlen );
    errno_t	gen_getproperty( properties_t *ps, void *context, const char *pName, char *pValue, int vlen );
    errno_t	gen_setproperty( properties_t *ps, void *context, const char *pName, const char *pValue );

Usage example. Driver init code, set entry points for properties:

.. code:: c

    dev->dops.getproperty = es1370_getproperty;
    dev->dops.setproperty = es1370_setproperty;
    dev->dops.listproperties = es1370_listproperties;

List available properties. This function will be called with
increasing ``nProperty`` value until it returns an error.

.. code:: c

    static const char *pList = "sampleRate";

    static errno_t es1370_listproperties( struct phantom_device *dev, int nProperty, char *pValue, int vlen )
    {
        if( nProperty > 0 ) return ENOENT;
        strlcpy( pValue, pList, vlen );
        return 0;
    }

Get value of property:

.. code:: c

    static errno_t	es1370_getproperty( struct phantom_device *dev, const char *pName, char *pValue, int vlen )
    {
        es1370_t *es = dev->drv_private;

        if(0 == stricmp(pName, "samplerate"))
        {
            snprintf( pValue, vlen, "%d", es->samplerate );
            return 0;
        }

        return ENOTTY;
    }

.. code:: c

    static errno_t	es1370_setproperty( struct phantom_device *dev, const char *pName, const char *pValue )
    {
        es1370_t *es = dev->drv_private;

        if(0 == stricmp(pName, "samplerate"))
        {
            if( 1 != sscanf( pValue, "%d", &es->samplerate ) )
                return EINVAL;
            set_sampling_rate(dev, es->samplerate);
            return 0;
        }
    return ENOTTY;
    }

.. TODO describe other ways to use

-----------------------
Video drivers subsystem
-----------------------

Video driver
------------

Video driver must be, at least, able to:

- Turn on 24 or 32 bit video mode for the graphics hardware
- Set up linear video buffer access
- Provide functions to write and read video buffer

There are ready made functions to access usual frame buffer formats for 
24 or 32 bit buffers.

Video driver descriptor structure:

.. code:: c

    struct drv_video_screen_t
    {
        const char  *name;

        int     xsize;
        int     ysize;
        int     bpp;

        int     mouse_x;
        int     mouse_y;
        int     mouse_flags;

        char *  screen;

        int     (*probe) (void); // Returns true if hardware present, sets xsize/ysize.
        int     (*start) (void); // Start driver, switch to graphics mode.
        errno_t (*accel) (void); // Start driver in accelerating mode - video mode is already set by VESA, just add some acceleration to existing drv
        int     (*stop)  (void); // Stop driver, switch to text mode. Can be called in unstable kernel state, keep it simple.

        void    (*update) (void);

        void    (*bitblt) (const rgba_t *from, int xpos, int ypos, int xsize, int ysize, zbuf_t zpos, u_int32_t flags);
        void    (*winblt) ( const window_handle_t from, int xpos, int ypos, zbuf_t zpos);
        void    (*readblt) ( rgba_t *to, int xpos, int ypos, int xsize, int ysize);

        void    (*mouse)  (void); // mouse activity detected - callback from driver

        void    (*mouse_redraw_cursor)(void);
        void    (*mouse_set_cursor)(drv_video_bitmap_t *cursor);
        void    (*mouse_disable)(void);
        void    (*mouse_enable)(void);

    // Acceleration

        void    (*copy) (int src_xpos, int src_ypos, int dst_xpos, int dst_ypos, int xsize, int ysize );
        void    (*clear) (int xpos, int ypos, int xsize, int ysize );
        void    (*bitblt_part) (const rgba_t *from, int src_xsize, int src_ysize, int src_xpos, int src_ypos, int dst_xpos, int dst_ypos, int xsize, int ysize, zbuf_t zpos, u_int32_t flags );
    };

    #define VIDEO_PROBE_FAIL    0
    #define VIDEO_PROBE_SUCCESS 1
    #define VIDEO_PROBE_ACCEL   2 

**probe**
  This is the entry point which is called by graphics subsystem on start.
  It must find out if there is some video device that this driver can control.
  Return values are: ``VIDEO_PROBE_FAIL`` - no hardware found, driver is skipped.
  ``VIDEO_PROBE_SUCCESS`` - driver can work with this hardware, either in basic or
  accelerated mode (if accelerated - driver must provide corresponding entry points).
  ``VIDEO_PROBE_ACCEL`` - driver can not initialize hardware and setup frame buffer,
  but can provide additional acceleration. The last mode is usually assumes that
  basic VESA driver will init hardware and this driver will do just acceleration.
  If driver reports success, its descriptor structure must be filled with maximum
  possible screen size and bpp. 

**start**
  Start driver. Graphics subsystem decided that this driver is the best among all 
  and want it to control the display. Driver must set up video mode, set entry points
  and map frame buffer into the kernel address space.

**stop**
  Turn graphics off, set character mode if possible, release resources.

**accel**
  Start in accelerating mode. Other driver is already set up video mode,
  this one has just to provide acceleration entry points.

Following entry points are called after driver started to access device:

**update**
  Finally after frame buffer is updated. Used if hardware needs some kick
  to redisplay screen or if screen is virtual or network.

**bitblt**
  Copy bitmap to screen. 

.. TODO give info on standard funcs

**winblt**
  Copy contents of a window to screen.

**readblt**
  Read bitmap from screen. Used by default mouse cursor engine.

Accelerating driver
-------------------

Video driver which can not work by itself and relies
on VESA or some other video driver to set up video mode
and access to frame buffer, but can provide accelerated 
functions.

Entry points:

**mouse_set_cursor**
  In accelerated mode supposed to provide hardware cursor.

**mouse_redraw_cursor**
  New cursor coordinated are in ``mouse_x`` and ``mouse_y`` fields.
  Move hardware cursor to that position.

**copy**
  Copy (bitblt) part of screen to other place.

**clear**
  Fill part of screen with color.



--------
Graphics
--------

Phantom graphical subsystem is built in 4 layers:

- Video driver
- Bitblt to screen functions - copy window image to screen
  supportnig z-order of windows
- Windowing system: windows, painting to windows, events,
  window decorations and related support
- Controls - graphical components supporting user interaction:
  buttons, check boxes, text entry fields, etc.

Windows
-------


.. code:: c

    #include <video/window.h>

    window_handle_t drv_video_window_create(int xsize, int ysize, int x, int y, rgba_t bg, const char* title, int flags );
 
    void    drv_video_window_destroy(window_handle_t w);

    void    w_to_top(window_handle_t w);
    void    w_to_bottom(window_handle_t w);

    void    w_clear( window_handle_t win );
    void    w_fill( window_handle_t win, rgba_t color );

    void    w_draw_rect( window_handle_t win, rgba_t color, rect_t r );
    void    w_fill_rect( window_handle_t win, rgba_t color, rect_t r );

    void    w_draw_pixel( window_handle_t w, int x, int y, rgba_t color );
    void    w_draw_line( window_handle_t w, int x1, int y1, int x2, int y2, rgba_t c);
    void    w_fill_ellipse( window_handle_t w, int x,int y,int lx,int ly, rgba_t c);
    void    w_fill_box( window_handle_t w, int x,int y,int lx,int ly, rgba_t c);
    void    w_draw_box( window_handle_t w, int x,int y,int lx,int ly, rgba_t c);
    void    w_draw_bitmap( window_handle_t w, int x, int y, drv_video_bitmap_t *bmp );

    // Draw with alpha blending
    void    w_draw_blend_bitmap( drv_video_window_t *w, int x, int y, drv_video_bitmap_t *bmp ); 

    void    w_move( window_handle_t w, int x, int y );

    errno_t w_scroll_hor( window_handle_t w, int x, int y, int xs, int ys, int s );
    void    w_scroll_up( window_handle_t win, int npix, rgba_t color);

    void    w_set_title( window_handle_t w,  const char *title );
    void    w_get_bounds( window_handle_t w, rect_t *out );
    void    w_set_visible( window_handle_t h, int v );
    void    w_set_bg_color( window_handle_t w, rgba_t color );


Controls
--------

Control is graphical elemen attached to window and processing user input.

Create control:

.. TODO pictures

.. code:: c

    #include <video/control.h>

    control_handle_t w_add_button( window_handle_t w, int id, int x, int y, drv_video_bitmap_t *bmp, drv_video_bitmap_t *pressed, int flags );
    control_handle_t w_add_radio_button( window_handle_t w, int id, int group_id, int x, int y );
    control_handle_t w_add_checkbox( window_handle_t w, int x, int y );
    control_handle_t w_add_menu_item( window_handle_t w, int id, int x, int y, int xsize, const char*text, color_t text_color );
    control_handle_t w_add_label( window_handle_t w, int x, int y, int xsize, int ysize, const char *text, color_t text_color );
    control_handle_t w_add_label_transparent( window_handle_t w, int x, int y, int xsize, int ysize, const char *text, color_t text_color );
    control_handle_t w_add_label_ext( window_handle_t w, int x, int y, int xsize, int ysize, const char *text, color_t text_color, 
                                    drv_video_bitmap_t *bg, uint32_t flags );
    control_handle_t w_add_text_field( window_handle_t w, int x, int y, int xsize, int ysize, const char *text, color_t text_color );
    control_handle_t w_add_scrollbar_ext( window_handle_t w, int x, int y, int xsize, int ysize, int minval, int maxval, uint32_t flags );
    control_handle_t w_add_scrollbar( window_handle_t w, int x, int y, int xsize, int ysize, int maxval );


Change control state or parameters. Text functions accept UTF-8 encoding.

.. code:: c

    void w_control_set_text( window_handle_t w, control_handle_t c, const char *text, color_t text_color );
    void w_control_get_text( window_handle_t w, control_handle_t c, char *text_buf, size_t buf_size );
    void w_control_set_icon( window_handle_t w, control_handle_t ch, drv_video_bitmap_t *icon );

    //! NB! Allocates new bitmaps and does alpha blending with basic window background
    void w_control_set_background( window_handle_t w, control_handle_t ch, 
        drv_video_bitmap_t *normal, drv_video_bitmap_t *pressed, drv_video_bitmap_t *hover  );

    void w_control_set_visible( window_handle_t w, control_handle_t ch, int visible ); // unimpl yet
    void w_control_set_position( window_handle_t w, control_handle_t ch, int x, int y );


Set flags for a control:

.. code:: c

    void w_control_set_flags( window_handle_t w, control_handle_t ch, int toSet, int toReset );

Following flags are defined:

**CONTROL_FLAG_DISABLED**
  This control is disabled - not painted and does not process input.
  
**CONTROL_FLAG_NOPAINT**
  Control is not painted, but processes input. Use if you paint control manually.

**CONTROL_FLAG_NOBORDER**
  Do not paint border around the control. Border still will be paint if
  control is in focus.
  
**CONTROL_FLAG_CALLBACK_HOVER**
  Callback function will be called on mouse over the control.
  
**CONTROL_FLAG_CALLBACK_KEY**
  Call callback on any key press. If this flag is not set, callback is just
  called for enter in text entry field or key that changes on/off state
  for buttons and similar controls.

**CONTROL_FLAG_TOGGLE**
  Button or menu item swithces on and off with each press.

**CONTROL_FLAG_HORIZONTAL**
  Lay control horizontally - not yet used.

**CONTROL_FLAG_TEXT_RIGHT**
  Put button text to the right of image.
  
**CONTROL_FLAG_NOFOCUS**
  This control is passive - no events, no focus.

**CONTROL_FLAG_NOBACKGROUND**
  Do not paint control background - Unused?

**CONTROL_FLAG_READONLY**
  Control is just for display, don't accept user input - TODO implement

**CONTROL_FLAG_ALT_FG**
  Control foreground (active part) is paint in a different way.
  Supported by scroll bar only.

**CONTROL_FLAG_ALT_BG**
  Control background (passive part) is paint in a different way
  Supported by scroll bar only.



Set callback to be called on major control state change.

.. code:: c

    void w_control_set_callback( window_handle_t w, control_handle_t c, control_callback_t cb, void *callback_arg );


Set control's children - controlled objects. If just window is given, switch its visibility. If window and control - switch control visibility.

.. code:: c

    void w_control_set_children( window_handle_t w, control_handle_t c, window_handle_t w_child, control_handle_t c_child );

Show bullet with number (``count`` parameter) in top right corner

.. code:: c

    void w_control_set_notify( window_handle_t w, control_handle_t ch, int count ); 

Set context (right click) menu for the control. Note that it is
possible to set context menu for a whole window too.

.. code:: c

    void w_control_set_menu( window_handle_t w, control_handle_t ch, window_handle_t m );

For switch type controls (button, check box, etc) - set or get on/off (pressed/released) state of control.

.. code:: c

    void w_control_set_state( window_handle_t w, control_handle_t ch, int pressed );
    void w_control_get_state( window_handle_t w, control_handle_t ch, int *ret );

Control scrollbar position and size.
- value - current value. Determines start position of scroll bar.
- width - width of scroll bar - how big part of possible values it takes. If == maxval - minval - takes 100% of scrollbar size

Set value or width to be negative to disable display of bar at all.

.. code:: c

    void w_control_set_value( window_handle_t w, control_handle_t ch, int value, int width );  //< For scrollbar - set value & bar handle width
    void w_control_get_value( window_handle_t w, control_handle_t ch, int *value, int *width ); //< For scrollbar - get value & bar handle width




Fonts
-----

Phantom graphics subsystem supports bitmap and TrueType fonts.

.. code:: c

    #incude <video/font.h>

Preinstalled set of bitmap fonts:

drv_video_16x16_font, drv_video_8x16ant_font, drv_video_8x16bro_font,
drv_video_8x16cou_font, drv_video_8x16med_font, drv_video_8x16rom_font,
drv_video_8x16san_font, drv_video_8x16scr_font, drv_video_kolibri1_font,
drv_video_kolibri2_font, drv_video_gallant12x22_font, drv_video_freebsd_font.

Window functions to work with bitmap fonts:

.. code:: c

    void w_font_draw_string(
        window_handle_t win,
        const drv_video_font_t *font,
        const char *s, 
        const rgba_t color,
        const rgba_t bg,
        int x, int y );

    void w_font_scroll_line(
        window_handle_t win,
        const drv_video_font_t *font, rgba_t color );

    void w_font_tty_string(
        drv_video_window_t *win,
        const struct drv_video_font_t *font,
        const char *s,
        const rgba_t fg_color,
        const rgba_t bg_color,
        int *x, int *y );

**w_font_draw_string**
  Paint string with given font.

**w_font_scroll_line**
  Scroll window one line up using given font height.  

**w_font_tty_string**
  Paint string processing TTY control chars:
  \\n, \\r, some of ANSI control sequences (text color).
  Update given coordinates.

Truetype fonts support lets caller choose font style and size and provides
painting and size request functions.

Access font:

.. code:: c

    font_handle_t w_get_system_font_ext( int font_size );
    font_handle_t w_get_system_font( void );

    font_handle_t w_get_system_mono_font_ext( int font_size );
    font_handle_t w_get_system_mono_font( void );

    font_handle_t w_get_tt_font_file( const char *file_name, int size );
    font_handle_t w_get_tt_font_mem( void *mem_font, size_t mem_font_size, const char* diag_font_name, int font_size );

    errno_t w_release_tt_font( font_handle_t font );


**w_get_system_font_ext**
  Get handle of default system font for given font size.

**w_get_system_mono_font_ext**
  Get handle of default monospaced (tty style) font for given size.

**w_get_tt_font_file**
  Load font from file - requires some filesystem to be running.

**w_get_tt_font_mem**
  Load font from memory - usually from persistent binary object.

**w_release_tt_font**
  Release font when it is not needed any more.


Use font:

.. code:: c

    void w_ttfont_draw_string(
        window_handle_t win,
        font_handle_t font,
        const char *s, const rgba_t color,
        int x, int y );

    void w_ttfont_draw_string_ext(
        window_handle_t win,
        font_handle_t font,
        const char *str, size_t strLen,
        const rgba_t color,
        int win_x, int win_y,
        int *find_x, int find_for_char );


    void w_ttfont_draw_char(
        window_handle_t win,
        font_handle_t font,
        const char *str, const rgba_t color,
        int win_x, int win_y );

    void w_ttfont_string_size( font_handle_t font,
        const char *str, size_t strLen,
        rect_t *r );

**w_ttfont_draw_string**
  Paint string with given ttf font.

**w_ttfont_draw_string_ext**
  Extended string paint version. Returns X coordinte of 
  ``find_for_char`` character in ``find_x`` variable.
  This feature is good for painting cursor line.

**w_ttfont_draw_char**
  Paint one character with TrueType font.

.. note::
  It is not a good idea to paint text character by charcter, for
  kerning is not processed if you do it this way.

**w_ttfont_string_size**
  Calculate bounding rectangle for string.

There are UTF-32 versions of these functions exist, see header file.

.. TODO UTF-32 versions


UTF-8 and UTF-32
----------------

Main string encoding in Phantom is UTF-8. But there 
are parts of system (namely - keyboard driver) that 
expect UTF-32 encoding.
Conversion is supported, of course.

.. TODO conv funcs

Rectangle algebra
-----------------

There is a set of functions to work with window/image coordinates.

They are based on a rect_t rectangle descriptor:

.. code:: c

    #include <video/rect.h>

    typedef struct rect
    {
        int x, y;
        int xsize, ysize;
    } rect_t;

    #define rect_copy( out, in ) rect_add( out, in, in )

    void rect_add( rect_t *out, rect_t *a, rect_t *b );
    int rect_mul( rect_t *out, rect_t *a, rect_t *b );
    int rect_sub( rect_t *out1, rect_t *out2, rect_t *old, rect_t *new );

    int rect_eq( rect_t *a, rect_t *b );
    int rect_empty( rect_t *a );

    int rect_includes( rect_t *a, rect_t *b );
    int rect_intersects( rect_t *a, rect_t *b );
    int point_in_rect( int x, int y, rect_t *r );

    int rect_dump( rect_t *a );

**rect_add**
  Resulting rectangle is rectangle that includes both of source ones.

**rect_mul**  
  Resulting rectangle is intersection. Function returns true if intersection exists.

**rect_sub**
  Calculates two rectangles which together cover space which is
  covered by ``old``, but not covered by ``new``. Returns nonzero if
  out2 is not equal to out1 and not empty.

**rect_eq**
  True if rectangles are equal.

**rect_empty**
  True if at least one of rectangle sizes is negative or zero.

**rect_includes**
  Returns true if a includes (or is equal to) b.

**rect_intersects**
  Returns true if a intersects with (or is equal to) b.

**point_in_rect**
  Returns true if point belongs to rectangle.

-------------------------
Persistent virtual memory
-------------------------

This is the core of native Phantom personality.


Persistent memory is used by Phantom OS bytecode vitrual machine (referred to as pvm).
This memory is occupied by pvm objects and is subject of garbage collector work.

Kernel can access this memory and create and read objects in there, but there are some
rules.

First of all, this memory is snapshotted to disk from time to time, and it 
must be in consistent state during snapshot. It means that when key stage if
snapshot is going on no thread can access this memory. 

.. sidebar:: Snapshots are fast

  Note that key stage of snapshot is extremely short, so it does not 
  stop any activity for a noticeable time.

There are two functions that are used to interlock access to persistent memory.


.. code:: c

    #include <kernel/snap_sync.h>

    void vm_lock_persistent_memory( void );
    void vm_unlock_persistent_memory( void );

**vm_lock_persistent_memory**
  Request access to persistent memory address space, prevent snapshots.

**vm_unlock_persistent_memory**
  Release access to persistent memory address space, enable snapshots.

Kernel code must call ``vm_lock_persistent_memory`` before it is 
going to access persistent memory and ``vm_unlock_persistent_memory``
as soon as possible after it.

.. warning::

  Keeping access to persistent memory for too long will keep other threads waiting
  for snapshot to finish for too long too, because snapshot can not happen 
  as long as you keep persistent memory locked. Be really quick.

Objects and classes
-------------------

Phantom VM is an object-oriented VM (actually it has special support for 
functional programming too). Everything inside persistent memory is object.
Object is instance of class. Which is object too.

Regular objects contain just pointers to other objects. Irregular (internal classes)
objects contain some kind of data. For exmaple, object of class 
``internal.string`` contains text string.

.. sidebar:: Strings are files

  Actually phantom strings do not restrict its contents in any way.
  String can contain just any binary data including binary zeroes.
  That is why strings in Phantom are freqently used as... anonymous files.
  It is handy to put assets into strings and Phantom language compiler
  has special support for this kind of use.
  For example, some applications keep bitmap pictures in strings.
  See an example for `.internal.bitmap`_ class.

Internal classes
----------------

Phantom userland (virtual bytecode machine) code communicates with OS kernel
by means of so called *internal* classes. Each internal class is implemented by
in-kernel C code.

`Source code of internal classes <https://github.com/dzavalishin/phantomuserland/tree/master/phantom/vm/sys>`_

Please refer to `Package .internal`_ for a complete reference of existing
internal classes.

Objects as they look from C
---------------------------

From the kernel point of view PVM object is just a structure with an array or 
structure at the end. If it is a regular object, it's data part is array of 
pointers to other objects. If it is of internal class, object contents is a 
structure of some type.

.. code:: c

    #include <vm/object.h>

    struct object_PVM_ALLOC_Header
    {
        unsigned int                object_start_marker;
        volatile int32_t            refCount;
        unsigned char               alloc_flags;
        unsigned char               gc_flags;
        unsigned int                exact_size;
    };

    struct pvm_object_storage
    {
        struct object_PVM_ALLOC_Header _ah;

        struct pvm_object_storage *    _class;
        struct pvm_object_storage *    _satellites;
        u_int32_t                      _flags; 
        unsigned int                   _da_size;

        unsigned char                  da[];
    };

    typedef struct pvm_object_storage   pvm_object_storage_t;
    typedef struct pvm_object_storage * pvm_object_t;

**_class**
  Pointer to an object which describes class of this object.

**_satellites**
  Used in a very special cases where there some additional data
  must be associated with our object. Examples are weak references
  (object must keep list of weak references that look at it) or
  Java-style syncronization machinery, which needs mutex to be 
  associated with arbitrary object.

**_flags**
  Used to denote special cases or help to check for some types
  of objects very fast. Example is object of ``.internal.int``
  class. It would be too long to go through the class reference to
  check if this object is an int.

**_da_size**
  Size of object contents **in bytes**.

There are numerous functions to access objects in different ways.  


.. code:: c

    pvm_object_t pvm_create_object(pvm_object_t type);
    pvm_object_t pvm_get_class( pvm_object_t o );

    #define      pvm_is_null( o ) ...
    #define      pvm_isnull( o ) pvm_is_null( o )

    pvm_object_t pvm_get_field( pvm_object_t , unsigned int no );
    void         pvm_set_field( pvm_object_t , unsigned int no, pvm_object_t value );

**pvm_create_object**
  Create object of given class. For most internal classes there is
  a ``pvm_get_NAME_class(void)`` function exist. For example, 
  ``pvm_get_window_class(void)`` for ``.internal.window``. So if you
  need a new int, ``pvm_create_object( pvm_get_int_class() )`` will do.

**pvm_get_class**
  Find out class of object.

**pvm_is_null**
  Check if object is null.

**pvm_get_field, pvm_set_field**
  Access fields of regular objects.

.. code:: c

    int pvm_object_class_exactly_is( pvm_object_t object, pvm_object_t tclass );
    int pvm_object_class_is_or_parent( pvm_object_t object, pvm_object_t tclass );
    int pvm_object_class_is_or_child( pvm_object_t object, pvm_object_t tclass );

Check object type.

**pvm_object_class_exactly_is**
  Find if object is of exactly this class. This one is rarely needed.

**pvm_object_class_is_or_parent**
  True if object class is ``tclass`` or its parent.

**pvm_object_class_is_or_child**
  True if object class is ``tclass`` or its child. This is the most typical
  type check.

.. TODO if implements interface


Arrays operations. Note that there is no syncronization provided
by basic array operations. You have to provide your own protection.

.. TODO ref to pvm_spinlock and mutex

.. code:: c

    pvm_object_t pvm_get_array_ofield(pvm_object_t o, unsigned int slot  );
    void         pvm_set_array_ofield(pvm_object_t o, unsigned int slot, pvm_object_t value );

    int          pvm_get_array_size(pvm_object_t array);
    void         pvm_append_array(pvm_object_t array, pvm_object_t value_to_append );
    void         pvm_pop_array(pvm_object_t array, pvm_object_t value_to_pop );

**pvm_get_array_ofield**
  Read field of array.

**pvm_set_array_ofield**
  Set array field. Will silently extend array if you access absent element.

**pvm_get_array_size**
  How many slots in array right now.

**pvm_append_array**
  Extend array by one slot and put object to last slot. Practically push to stack.
  **Mind the syncronization!**

**pvm_pop_array**
  Return last element, decreasing size of array by one. Pop from stack.

.. TODO difference from usual field read?

.. TODO need list implementation too.

Debugging and printing.

.. code:: c

    void         pvm_object_print( pvm_object_t );
    void         pvm_object_dump( pvm_object_t o );
    void         dumpo( addr_t addr );
    void         pvm_puts(pvm_object_t o );
    pvm_object_t pvm_get_class_name( pvm_object_t );



Running PVM code
----------------

Run new VM instance in a new thread. An ``object`` parameter must be 
of ``.phantom.runnable`` class. 

.. code:: c

    errno_t pvm_run_new_thread( pvm_object_t object, pvm_object_t arg );

This function will create new thread and execute method (with ordinal of 8)
with given object as *this* and passing ``arg`` to that method.



--------------
Unix subsystem
--------------

Phantom OS includes simple POSIX/Linux emulation.

(This part of book is not complete)

File IO
-------


BeOS ports
----------

This is not POSIX, but is more or less meaningless without POSIX.
Ports is a microkernel style gear that can be used to write server
processes that serve other parts of OS. Port is named message passing IPC
channel. Port message is message type (integer) and message body, which is 
simply a byte buffer. Message type is not interpreted by kernel. Its meaning is
defined by server party.

.. code:: c

    #include <newos/port.h>

    errno_t phantom_port_create(port_id *ret, int32 queue_length, const char *name);
    errno_t phantom_port_close(port_id id);
    errno_t phantom_port_delete(port_id id);
    errno_t phantom_port_find(port_id *ret, const char *port_name);
    errno_t phantom_port_buffer_size(ssize_t *sizep, port_id port);
    errno_t phantom_port_buffer_size_etc(ssize_t *sizep, port_id port,
					uint32 flags,
					bigtime_t timeout);
    errno_t phantom_port_count(int32 *countp, port_id port);
    errno_t phantom_port_read(ssize_t *len, 
					port_id port,
					int32 *msg_code,
					void *msg_buffer,
					size_t buffer_size);
    errno_t phantom_port_read_etc(ssize_t *len, 
					port_id port,
					int32 *msg_code,
					void *msg_buffer,
					size_t buffer_size,
					uint32 flags,
					bigtime_t timeout);
    errno_t phantom_port_write(port_id port,
					int32 msg_code,
					void *msg_buffer,
					size_t buffer_size);
    errno_t phantom_port_write_etc(port_id port,
					int32 msg_code,
					void *msg_buffer,
					size_t buffer_size,
					uint32 flags,
					bigtime_t timeout);



**phantom_port_create**
  Create port win given ``name``. The ``queue_length`` parameter sets number
  of outstanding messages that can wait in port queue for processing. 

**phantom_port_find**
  Find port created by other party by name.

**phantom_port_buffer_size_etc**
  Get size of buffer required to receive next message from port.

**phantom_port_buffer_size**
  Get size of buffer required to receive next message from port.
  Wait forever.

**phantom_port_write_etc**
  Send a message to ``port``. Message type is in ``msg_code`` parameter. 
  Message itself is in the ``msg_buffer``, its size is ``buffer_size``. 
  If ``flags`` has value of ``PORT_FLAG_TIMEOUT``, function will fail after
  ``timeout`` microseconds if unable to send message.

**phantom_port_read_etc**
  Get next message from port. Parameters are similar to ``phantom_port_write_etc``.

.. TODO there must be answer channel, answer message must be targeted to
.. TODO tid or pid - like unix multiplex files, msg recv must give out tid of sender,
.. TODO but just for port owner



------------------------------
Kernel infrastructure services
------------------------------

Misc info
---------

Current architecture and board (hw conf) names.

.. code:: c

    #include <kernel/init.h>

    extern char arch_name[];
    extern char board_name[];



Kernel subsystems init and stop
-------------------------------

Any kernel source file can have own init and stop code. Use following
macros:

.. code:: c

    #include <kernel/init.h>

    #define INIT_ME(__init1,__init2,__init3)
    #define STOP_ME(__stop1,__stop2,__stop3)

Usage:

.. code:: c

    static void init_module( void )
    {
    ...
    }

    INIT_ME(0,init_module,0);

Three parameters of ``INIT_ME`` macros are functions (or zero to skip) that
will be called at different moments.

An ``__init1`` is called very early, most of the kernel services are not
ready at this moment. Use just for modules that will be needed for later
starting code. No threads, no mutex/cond/etc, no network, no disks.

An ``__init2`` is called when most of the kernel is working.

An ``__init3`` is called very late. Quite everything is running at this
moment, object land is available. Use for slow and not critical components
or additional startup of things that started in basic form before.

Character buffer chains - cbuf
------------------------------


Hash tables - khash
-------------------


--------------------
Debugging facilities
--------------------


Logging
-------

.. code:: c

  #define DEBUG_MSG_PREFIX "vm.exec"
  #include <debug_ext.h>
  #define debug_level_flow 10
  #define debug_level_error 10
  #define debug_level_info 0

Set up logging subsystem with message prefix to be ``vm.exec``, log level 
for LOG_FLOW messages to be 10 (messages with levels up to 10 are printed,
11 and more are skipped), LOG_ERROR level of 10 and LOG_INFO\_ level of 0.

Main kernel logging facility has 6 entry points:

.. code:: c

  LOG_FLOW( level, "format", args )
  LOG_ERROR( level, "format", args )
  LOG_INFO\_( level, "format", args )
  LOG_FLOW( level, "format", args )
  LOG_ERROR( level, "format", args )
  LOG_INFO\_( level, "format", args )

Additional marcos show message in Phantom's debug window and log it.

.. code:: c

  SHOW_FLOW( level, "format", args )
  SHOW_ERROR( level, "format", args )
  SHOW_INFO\_( level, "format", args )
  SHOW_FLOW( level, "format", args )
  SHOW_ERROR( level, "format", args )
  SHOW_INFO\_( level, "format", args )

If there is a message with no args, add 0 to macros name:

.. code:: c

  SHOW_FLOW0( 1, "Driver started" );

