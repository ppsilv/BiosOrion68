

.global call_with_a5

call_with_a5:
    /* args on stack per m68k C ABI: entry_addr, a5_value, argc, argv */
    move.l  4(%sp), %a0      /* entry_addr */
    move.l  8(%sp), %a5      /* a5_value */
    move.l  12(%sp), %d0     /* argc */
    move.l  16(%sp), %a1     /* argv */
    move.l  %a1, -(%sp)
    move.l  %d0, -(%sp)
    jsr     (%a0)
    addq.l  #8, %sp
    rts
