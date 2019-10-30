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

    bigtime_t 		hal_system_time(void); // uptime
    bigtime_t 		hal_local_time(void);  // real time/date

    time_t 			time(time_t *timer);
    void 			set_time(time_t time);

    // Fast, but less accurate time, sec
    time_t          fast_time(void);

    // Uptime in seconds
    time_t          uptime(void);

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

    // Event execution will be postponed while this spinlock is locked
    // TIMEDCALL_FLAG_CHECKLOCK flag must be set
        hal_spinlock_t	*lockp;

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

-----------------
DPC: Thread pools
-----------------

-------
Disk IO
-------

IO Request
----------

Syncronous disk IO
------------------

Parttitions
-----------

File systems
------------

Native Phantom code can be (and usually is) written
so that it needs no access to file systems. But it
is unrealistic yet and not reasonable to live completely 
without traditional FS access.



-------
Network
-------

Interfaces
----------

DHCP
----

----------------------
Main drivers subsystem
----------------------


-----------------------
Video drivers subsystem
-----------------------

Video driver
------------

Accelerating driver
-------------------

Video driver which can not work by itself and relies
on VESA or some other video driver to set up video mode
and access to frame buffer, but can provide accelerated 
functions.

Mouse cursor
^^^^^^^^^^^^

Support for hardware mouse cursor painting.

Rectangle copy
^^^^^^^^^^^^^^

Copying screen parts.

Rectangle fill
^^^^^^^^^^^^^^

Filing part of screen with color.



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
    void drv_video_window_destroy(window_handle_t w);

    void 	w_to_top(window_handle_t w);
    void 	w_to_bottom(window_handle_t w);

    void    w_clear( window_handle_t win );
    void    w_fill( window_handle_t win, rgba_t color );

    void 	w_draw_rect( window_handle_t win, rgba_t color, rect_t r );
    void 	w_fill_rect( window_handle_t win, rgba_t color, rect_t r );

    void	w_draw_pixel( window_handle_t w, int x, int y, rgba_t color );
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
    void	w_set_bg_color( window_handle_t w, rgba_t color );


Controls
--------

Control is graphical elemen attached to window and processing user input.

Create control:

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


Change control state or parameters:

.. code:: c

    //! UTF-8
    void w_control_set_text( window_handle_t w, control_handle_t c, const char *text, color_t text_color );
    //! UTF-8
    void w_control_get_text( window_handle_t w, control_handle_t c, char *text_buf, size_t buf_size );
    void w_control_set_icon( window_handle_t w, control_handle_t ch, drv_video_bitmap_t *icon );

    //! NB! Allocates new bitmaps and does alpha blending with basic window background
    void w_control_set_background( window_handle_t w, control_handle_t ch, 
        drv_video_bitmap_t *normal, drv_video_bitmap_t *pressed, drv_video_bitmap_t *hover  );

    void w_control_set_visible( window_handle_t w, control_handle_t ch, int visible ); // unimpl yet
    void w_control_set_flags( window_handle_t w, control_handle_t ch, int toSet, int toReset );
    void w_control_set_position( window_handle_t w, control_handle_t ch, int x, int y );

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

----------------
UTF-8 and UTF-32
----------------

Main string encoding in Phantom is UTF-8. But there 
are parts of system (namely - keyboard driver) that 
expect UTF-32 encoding.
Conversion is supported, of course.

-------------------------
Persistent virtual memory
-------------------------

This is the core of native Phantom personality.

--------------
Unix subsystem
--------------

Simple POISX/Linux emulation.

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

