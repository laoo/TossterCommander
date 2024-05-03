
STACK_SIZE  = $100

P_TLEN      = $0C
P_DLEN      = $14
P_BLEN      = $1C

MSHRINK     = $4A
PTERM       = $4C

        text

_start::
        move.l  4(sp),a0
        lea.l   stack_base,sp
        move.l  a0,-(sp)


        move.l  #256,d0
        add.l   P_TLEN(a0),d0
        add.l   P_DLEN(a0),d0
        add.l   P_BLEN(a0),d0
        move.l  d0,-(sp)
        move.l  a0,-(sp)
        clr.w   -(sp)
        move.w  #MSHRINK,-(sp)
        trap    #1
        add.l   #12,sp

        move.l  (sp),a0
        add.l   #128,a0
        moveq   #0,d0
        move.b  (a0),d0
        lea.l   1(a0),a0
        move.l  a0,-(sp)         ;empty argv
        move.l  d0,-(sp)         ;argc = 0

        jsr     main

        move.w  d0,-(sp)
        move.w  #PTERM,-(sp)
        trap    #1

        bss
        even

        ds.l   STACK_SIZE
stack_base:
        ds.w   1
