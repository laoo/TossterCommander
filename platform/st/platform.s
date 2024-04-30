
        text

_putchar::
        move.l  4(sp),d0
        move.w  d0,-(sp)
        move.w  #2,-(sp)
        trap    #1
        addq.l  #4,sp
        rts

platform_init::
        rts

tosster_open::
        lea     $e00000,a0
        move.w  $100(a0),d0
        move.w  $200(a0),d0
        move.w  (a0),d0
        move.w  $200(a0),d0
        move.w  $200(a0),d0
        move.w  (a0),d0
        cmp.w   (a0),d0
        sne     d0
        rts

tosster_close::
        lea     $e00000,a0
        move.w  $200(a0),d0
        move.w  $200(a0),d0
        move.w  $200(a0),d0
        rts

tosster_put::
        lea     $e00000,a0
        move.l  4(sp),d0
        asl.w   d0
        move.w  (a0,d0),d1
        move.w  $200(a0),d1
        rts

tosster_get::
        move.w  $e00000,d0
        rts

__umodsi3::
        move.l  8(sp),d1
        move.l  4(sp),d0
        move.l  d1,-(sp)
        move.l  d0,-(sp)
        bsr.b   __udivsi3
        addq.l  #8,sp
        move.l  8(sp),d1
        move.l  d1, -(sp)
        move.l  d0,-(sp)
        bsr.b   __mulsi3
        addq.l  #8,sp
        move.l  4(sp),d1
        sub.l   d0,d1
        move.l  d1,d0
        rts

__udivsi3::
        move.l  d2,-(sp)
        move.l  12(sp),d1
        move.l  8(sp),d0
        cmp.l   #$10000,d1
        bcc.s   L3
        move.l  d0,d2
        clr.w   d2
        swap    d2
        divu    d1,d2
        move.w  d2,d0
        swap    d0
        move.w  10(sp),d2
        divu    d1,d2
        move.w  d2,d0
        bra.s   L6
L3:     move.l  d1,d2
L4:     lsr.l   #1,d1
        lsr.l   #1,d0
        cmp.l   #$10000,d1
        bcc.s   L4
        divu    d1,d0
        and.l   #$ffff,d0
        move.l  d2,d1
        mulu    d0,d1
        swap    d2
        mulu    d0,d2
        swap    d2
        tst.w   d2
        bne.s   L5
        add.l   d2,d1
        bcs.s   L5
        cmp.l   8(sp),d1
        bls.s   L6
L5:     subq.l  #1,d0
L6:     move.l  (sp)+,d2
        rts

__mulsi3::
        move.w  4(sp),d0
        mulu.w  10(sp),d0
        move.w  6(sp),d1
        mulu.w  8(sp),d1
        add.w   d1,d0
        swap    d0
        clr.w   d0
        move.w  6(sp),d1
        mulu.w  10(sp),d1
        add.l   d1,d0
        rts


        data

        align   8
tos_image::
        rept    32768
        dc.q    $544f53494d414745
        endr
tosster_slot::
        dc.l    $3
