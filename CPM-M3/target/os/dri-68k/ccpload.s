*****************************************************************
*                                                               *
*               COMMAND FILE LOADER FOR CPM68K                  *
*               ==============================                  *
*                                                               *
*       (c)     COPYRIGHT Digital Research 1983                 *
*               all rights reserved                             *
*                                                               *
*       THIS IS THE DUAL PROCESSOR,ROMABLE CP/M-68K SYSTEM      *
*       ==================================================      *
*                                                               *
*       Description:                                            *
*       -----------     The command file loader is envoked by   *
*                       the CCP after the CCP has successfully  *
*                       opened that file.  The loader must      *
*                       call the BDOS to obtain the boundries   *
*                       of the TPA.  The load parameter block   *
*                       defined in this loader holds all the    *
*                       memory size and location details.       *
*                       Next the loader returns the system to   *
*                       its original user #.  The CCP might     *
*                       have switched to user zero during its   *
*                       search for the file.  Next the default  *
*                       dma address is set for the loaded       *
*                       program.  Next the command tail is      *
*                       placed,along with the first two parsed  *
*                       fcb's,into the user basepage.           *
*                       Lastly the user stack pointer is set up *
*                       and the return address is put on the    *
*                       user stack.  An RTE transferes control. *
*                       If a load was not successfull, the      *
*                       appropriate error message is printed.   *
*                                                               *
*       Created by:     Tom Saulpaugh                           *
*                                                               *
*       Last Modified:  2/17/84 sw 68010 support                *
*                                                               *
*****************************************************************



        .globl _load68k         * make this procedure public
        .globl _user            * global user # before load occured
        .globl _cmdfcb          * parsed fcb
        .globl _tail            * global pointer to command tail
        .globl _fill_fcb        * procedure to fill fcb's
        .globl flags            * ROM SYSTEM INITIALIZATION
        .globl TPAB             * ROM SYSTEM INITIALIZATION
	.globl gouser		*sw 68000/68010 rte routine

reboot   =  0
printstr =  9
setdma   = 26
chuser   = 32
pgmldf   = 59
gettpa   = 63

_load68k:
*
*       Load the 68k file into the TPA
*       ------------------------------
* 
        .text

        move.l  #TPAB,d1        * move in address of tpa parameter block
        move.w  #gettpa,d0      * get function number
        trap    #2              * get the tpa limits 
        move.l  low,lowadr      * put it in the lpb
        move.l  high,hiadr      * put high tpa addr in lpb
        move.l  #_cmdfcb,LPB    * get address of opened fcb
        move.l  #pgmldf,d0      * move in bdos function no
        move.l  #LPB,d1         * d1 points to load block
        trap    #2              * do the program load
        tst     d0              * was the load successful?
        bne     lderr           * if not print error message and return
*
*       return to original user #
*       -------------------------
        move.w  _user,d1        * put user # to switch to in d1
        move.l  #chuser,d0      * put bdos func # in d0
        trap    #2              * do the user # change
*
*       set the default dma address
*       ---------------------------
        clr.l   d1              * clear d1 register
        move.l  baspag,d1       * d1 points to user base page
        add     #$80,d1         * d1 points to default dma in base page
        movea.l d1,a1           * save it for later use
        move    #setdma,d0      * move in bdos function no
        trap    #2              * set the default dma address
*
*       move in the command tail
*       ------------------------
        move.l  a1,a2           * save a pointer to the count field
        add.l   #$01,a1         * point past count field
        move.l  _tail,a0        * point to command tail
        clr.l   d0              * clear out d0
mvtail: cmpi.b  #$00,(a0)       * check for a NULL ending byte
        beq     done            * NULL byte terminates command
        cmpi.b  #$21,(a0)       * check for an '!'
        beq     done            * '!' ends the command
        move.b  (a0)+,(a1)+     * move a byte of the command tail
        addq    #1,d0           * bump up the character count
        bra     mvtail          * continue byte move
done:   move.b  d0,(a2)         * put in the character count
        move.b  #$00,(a1)       * terminate cmd tail with a NULL byte
*
*       fill fcb1 & fcb2
*       ----------------
        move.l  #_cmdfcb,-(sp)  * put address of fcb buffer onto stack
        move.w  #1,-(sp)        * put 1 on stack(parm1) 
        jsr     _fill_fcb       * jump to 'C' code & fill cmdfcb with parm1
        add.l   #6,sp           * clean off the stack
        clr.l   d0              * clear register d0
        moveq   #$5c,d0         * put basepage address of fcb1 in d0
        bsr     movfcb          * put fcb1 in the basepage
        move.l  #_cmdfcb,-(sp)  * put address of fcb buffer onto stack
        move.w  #2,-(sp)        * put 2 on stack(parm2)
        jsr     _fill_fcb       * jump to 'C' code & fill cmdfcb with parm2
        add.l   #6,sp           * clean off the stack
        clr.l   d0              * clear register d0
        moveq   #$38,d0         * put basepage address of fcb1 in d0
        bsr     movfcb          * put fcb2 in the basepage
*
*       now push rte stuff on stack
*       ---------------------------
        movea.l usrstk,a0       * get user stack pointer
        move.l  baspag,a1       * get basepage address
*sw     move.l  8(a1),-(sp)     * push address we want to jump to
*sw     move    sr,d0           * get the status register in d0
*sw     andi    #$5f00,d0       * mask trace,system bits,user flags
*sw     move.w  d0,-(sp)        * push it on stack
        move.l  a1,-(a0)        * push addr of basepage onto user stack
        move.l  #cmdrtn,-(a0)   * push return address onto user stack
        move.l  a0,usp          * set up user stack pointer
	move.l	8(a1),a0	*sw a0 -> User program epa
	jmp	gouser		*sw Jump to exit routine in exceptn.s
*sw     rte
*
*       load error
*       ----------
lderr:
        rts                     * return with error code in d0
cmdrtn:
        move    #reboot,d0      * reboot CPM
        trap    #2
movfcb:
        add.l   baspag,d0       * get offset into basepage 
        move.l  d0,a0           * move address into a0
        move.l  #_cmdfcb,a1     * a1 points to fcb to be moved
        clr.l   d0              * clear register d0
        moveq   #35,d0          * get length of fcb
mov1:
        move.b  (a1)+,(a0)+     * move a byte into the basepage
        dbf     d0,mov1         * if not done branch to mov1            
        rts


        .bss

        .even
*
* LOAD PARAMETER BLOCK
*
LPB:     .ds.l   1
lowadr:  .ds.l   1
hiadr:   .ds.l   1
baspag:  .ds.l   1
usrstk:  .ds.l   1
flags:   .ds.w   1

*
* TPA Parameter Block
*
        .even
TPAB:   .ds.w   1
low:    .ds.l   1
high:   .ds.l   1











        .end
                                                                       high:   .ds.l   1











        .end
                                                                       high:   .ds.l   1











        .end
                                                                       