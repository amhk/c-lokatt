#ifndef LOKATT_WINDOW_H
#define LOKATT_WINDOW_H
#include <stdint.h>

#include "lokatt/buffer.h"

/**
 * A curses window. Multiple (non-overlapping) windows can appear on the screen
 * simultaneously.
 *
 * The contents of a window is based on a buffer, but the window is responsible
 * for scrolling, text formatting including color, etc. Think of a window as
 * the Activity or Fragment in Android and a buffer as the LiveData; a signal
 * such as SIGWINCH corresponds to onConfigurationChanged.
 */

typedef uintptr_t window_t;

/**
 * Create a new window. The caller becomes the owner of the window.
 *
 * The window is initially not associated with a buffer.
 *
 * Requires that curses has been initialized (i.e. initscr has been called).
 */
window_t window_create(size_t x, size_t y, size_t cols, size_t rows);

/**
 * Destroy a window.
 */
void window_destroy(window_t);

#define NO_BUFFER 0
/**
 * Set the contents of the window. This does not transfer ownership of the
 * buffer. Multiple windows can have the same buffer assigned at the same time.
 *
 * Use NO_BUFFER to clear any previous buffer assignment.
 */
void window_set_buffer(window_t, buffer_t);

/**
 * Update the internal data structures with the current data from the
 * associated buffer, or blank the window if no buffer is set.
 *
 * This call has no visible effect; a single call to doupdate is required for
 * that.
 */
void window_refresh(window_t);

/**
 * Get a string representation of what the window would show if doupdate was
 * called.
 */
void window_dump(window_t, char *out, size_t n);

#endif
