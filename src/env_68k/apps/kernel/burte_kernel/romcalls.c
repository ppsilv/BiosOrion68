/* File :romcalls.c */
/* routine that builds an array of all user available rom calls */
/*
 *
 *  Author             Date         Description
 *  --------------     --------     -----------------------------------------
 *  Andrew H. Fagg     03/02/93     Removed tm_read, tm_set, tm_scale since
 *                                   these are no longer supported (332 vers).
 *
 *
 */

#include <burte.h>
#include <burteio.h>
#include "system.h"
#include <romcalls.h>

make_rtable ()
{
    PFI *kp;
    kp = krn_table;
				/* ordering must match romcalls.h */
    *kp++ = (PFI)r_malloc;
    *kp++ = r_free;
    *kp++ = (PFI)r_realloc;
    *kp++ = r_tell_free;
    *kp++ = r_is_mem;
    *kp++ = r_lock;
    *kp++ = r_set_baud;
    *kp++ = r_download;
    *kp++ = force_print;
    *kp++ = force_char;
    *kp++ = force_hex;
    *kp++ = force_hexf;
    *kp++ = force_flush;
    *kp++ = outs;
    *kp++ = NULL; /*(PFI)tm_read; */
    *kp++ = NULL; /*tm_set;*/
    *kp++ = NULL; /*tm_scale; */
}
