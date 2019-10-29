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

--------------------
Allocators and Pools
--------------------

Kernel allocators:

- Main physical memory allocator
- Address space allocator
- P+V allocator
- Kernel heap

Aside from allocators, but used together is pools subsystem.

---------------
Time and timers
---------------

Main system timer
-----------------


Time of day
-----------

Time of day subsystem keeps track of current time and
date.

In-interrupt timer calls
------------------------

It is possible to call some callback after a given time
right from a timer interrupt. Should be used very carefuly.

In-thread timer calls
---------------------

Kernel call can request for a callback in a thread
context to be done after a given time. Such a callback 
should not take long for it will prevent other callbacks
to be executed.

Thread sleep
------------

It is more or less ok for a kernel thread to sleep for
some time.

Relations with snapshots subsystem
----------------------------------

It is forbidden for a kernel code to sleep or wait
for a long time keeping access to a persistent
virtual memory.



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


..code:: c

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

..code:: c
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

..code:: c

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

..code:: c
    void w_control_set_callback( window_handle_t w, control_handle_t c, control_callback_t cb, void *callback_arg );


Set control's children - controlled objects. If just window is given, switch its visibility. If window and control - switch control visibility.

..code:: c
    void w_control_set_children( window_handle_t w, control_handle_t c, window_handle_t w_child, control_handle_t c_child );

Show bullet with number (``count`` parameter) in top right corner

..code:: c
    void w_control_set_notify( window_handle_t w, control_handle_t ch, int count ); 

Set context (right click) menu for the control. Note that it is
possible to set context menu for a whole window too.

..code:: c
    void w_control_set_menu( window_handle_t w, control_handle_t ch, window_handle_t m );

For switch type controls (button, check box, etc) - set or get on/off (pressed/released) state of control.

..code:: c
    void w_control_set_state( window_handle_t w, control_handle_t ch, int pressed );
    void w_control_get_state( window_handle_t w, control_handle_t ch, int *ret );

Control scrollbar position and size.
- value - current value. Determines start position of scroll bar.
- width - width of scroll bar - how big part of possible values it takes. If == maxval - minval - takes 100% of scrollbar size

Set value or width to be negative to disable display of bar at all.

..code:: c
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

..code:: c

  #define DEBUG_MSG_PREFIX "vm.exec"
  #include <debug_ext.h>
  #define debug_level_flow 10
  #define debug_level_error 10
  #define debug_level_info 0

Set up logging subsystem with message prefix to be ``vm.exec``, log level 
for LOG_FLOW messages to be 10 (messages with levels up to 10 are printed,
11 and more are skipped), LOG_ERROR level of 10 and LOG_INFO\_ level of 0.

Main kernel logging facility has 6 entry points:

..code:: c

  LOG_FLOW( level, "format", args )
  LOG_ERROR( level, "format", args )
  LOG_INFO\_( level, "format", args )
  LOG_FLOW( level, "format", args )
  LOG_ERROR( level, "format", args )
  LOG_INFO\_( level, "format", args )

Additional marcos show message in Phantom's debug window and log it.

..code:: c

  SHOW_FLOW( level, "format", args )
  SHOW_ERROR( level, "format", args )
  SHOW_INFO\_( level, "format", args )
  SHOW_FLOW( level, "format", args )
  SHOW_ERROR( level, "format", args )
  SHOW_INFO\_( level, "format", args )

If there is a message with no args, add 0 to macros name:

..code:: c

  SHOW_FLOW0( 1, "Driver started" );

