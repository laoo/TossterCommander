
        text

c_conws   = 9

tos_puts::
        move.l  4(sp),-(sp)
        move.w  #c_conws,-(sp)
        trap    #1
        addq.l  #6,sp
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


        data

        align   8
tos_image::
        rept    32768
        dc.q    $544f53494d414745
        endr
tosster_slot::
        dc.l    $3
