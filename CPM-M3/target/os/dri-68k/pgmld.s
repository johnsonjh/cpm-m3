
*********************************
*                               *
*  Function 59 -- Program Load  *
*   Assembly language version   *
*                               *
*        June  8, 1982          *
*                               *
*********************************

.globl  _pgmld          * this routine is public

secsize = 128           * CP/M sector size

* d0 always contains the return parameter from pgmld
* d1 is the return register from local subroutines
* a0 contains the pointer to the Load Parm Block passed to pgmld

* Return parameters in d0 are:
*       00 - function successful
*       01 - insufficient memory or bad header in file
*       02 - read error on file
*       03 - bad relocation information in file


* Entry point for Program Load routine
_pgmld:
        movem.l d1-d7/a0-a6, -(sp) * save everything, just to be safe
        move.l  60(sp),a0       * get pointer to LPB
        clr.l   d0              * start with return parm cleared
        bsr     gethdr          * get header
        tst     d0
        bne     lddone          * if unsuccessful, return
        bsr     setaddr         * set up load addresses
        tst     d0
        bne     lddone          * if unsuccessful, return
        bsr     rdtxt           * read code and data text segments into mem
        tst     d0
        bne     lddone          * if unsuccessful, return
        move.l  tstart,d7
        cmp.l   cseg,d7
        beq     noreloc
        bsr     reloc           * do relocation if necessary
noreloc:
        tst     d0
        bne     lddone
        bsr     setrtn          * set up return parameters
lddone:
        move.l  64(sp), d1
        bsr     setdma          * restore dma address
        movem.l (sp)+,d1-d7/a0-a6
        rts

* Subroutines

readseq:
* CP/M read sequential function
        move.l  d0,-(sp)        * save return parm
        move.l  FCBPtr(a0),d1
        moveq   #20,d0          * read seq function
        trap    #2              * call bdos
        move.l  d0,d1           * return parm in d1
        move.l  (sp)+,d0
        rts


setdma:
* CP/M set dma function
        move.l  d0,-(sp)        * save return parm
        moveq   #26,d0          * set dma function
        trap    #2              * call bdos
        move.l  (sp)+,d0        * restore d0
        rts


gethdr:
* Get header into buffer in data segment
        move.l  LoAdr(a0),d1
        bsr     setdma
        bsr     readseq
        tst     d1              * read ok?
        bne     badhdr          * if no, return bad
        moveq   #18,d7
        movea.l LoAdr(a0),a5
        movea.l #hdr,a6
geth1:  move.w  (a5)+,(a6)+     * move header into hdr
        dbf     d7,geth1
        rts
badhdr: moveq   #2,d0
        rts


conflict:
* input parms: d2, d3 = 4 * segment nmbr
* if segment d2/4 overlaps segment d3/4, then return 1 in d1
* else return 0 in d1
* uses d7, a2, a3
        clr.l   d1              * assume it will work
        movea.l #cseg,a2        * a2 points to start of segment addresses
        movea.l #csize,a3       * a3 points to start of segment lengths
        move.l  0(a2,d2),d7     * get 1st seg start
        cmp.l   0(a2,d3),d7     * is 1st seg above 2nd seg?
        bge     conf1
        add.l   0(a3,d2),d7     * yes, find top of 1st seg
        cmp.l   0(a2,d3),d7     * above start of 2nd seg?
        bgt     confbd          * if yes, we have a conflict
        rts                     * else, return good
conf1:
        move.l  0(a2,d3),d7
        add.l   0(a3,d3),d7     * find top of 2nd seg
        cmp.l   0(a2,d2),d7     * above start of 1st seg?
        ble     confgd          * if no, we're ok
confbd: moveq.l #1,d1
confgd: rts


trymemtp:
* entry: d2 is a segment nmbr [0..4]
* try to fit it at top of memory
* uses d3, d6, d7, a5, a6
* returns 0 in d1 if ok
        move.l  d2,d6           * d6 is loop counter for chksegs
        subq    #1,d6
        lsl     #2,d2           * multiply d2 by 4
        move.l  HiAdr(a0),d7    * top of mem to d7

chksegs:
* entry: d2 = 4 * (segment nmbr to try)
*        d6 = (d2/4) - 1  (loop counter)
*        d7 = address below which to try it
* check for conflicts with segments [0..d6] and low memory boundary
* return 0 in d1 if no conflicts, else d1 = 1
* uses d3, a5, a6
        movea.l #cseg,a5
        movea.l #csize,a6
        sub.l   0(a6,d2),d7     * subtract size of segment to try
        bclr    #0,d7           * make it even address
        move.l  d7,0(a5,d2)     * insert address in segment table
        cmp.l   LoAdr(a0),d7    * check for conflict with low memory
        blt     confbd
        clr.l   d3              * check for conflicts with 0..d6
chk1:
        bsr     conflict
        addq.l  #4,d3
        tst.l   d1              * conflict with this seg?
        dbne    d6,chk1         * if no, try next
        rts


fndseg:
* entry: d2 is a segment nmbr [0..4]
* try to fit segment d2 directly below segments 0..(d2-1)
* uses d3-d7, a5, a6
        move.l  d2,d5           * d5 is loop counter to find fit
        subq.l  #1,d5
        move.l  d5,temp
        lsl.l   #2,d2           * multiply segment by 4
        clr.l   d4              * d4 is segment to try to fit below
fnd1:
        move.l  temp,d6         * d6 is loop counter for chksegs
        movea.l #cseg,a5
        move.l  0(a5,d4),d7     * segment address to d7
        bsr     chksegs         * check for conflicts
        addq.l  #4,d4
        tst.l   d1
        dbeq    d5,fnd1         * if conflict, try next
        rts


setaddr:
* Set up load addresses for cseg, dseg, bss, basepg, and stack
        move.w  magic,d6
        andi.w  #$fffe,d6
        cmpi.w  #$601a,d6
        bne     badadr          * if magic nmbr <> 601a or 601b, skip
        move.l  bpsize,symsize
        move.l  #256,d7
        move.l  d7,bpsize       * base page is 256 bytes
        lea     stksize,a2
        cmp     (a2),d7
        blt     set0            * if stack size < 256, set to 256
        move.l  d7,(a2)
set0:   cmpi.w  #$601b,magic
        beq     seta
        tst.w   rlbflg
        beq     set1
seta:   move.l  tstart,cseg     * if not relocatable or hdr = $601b,
        bra     set2            * cseg starts at tstart
set1:   btst    #0,Flags(a0)
        bne     sldhi
* relocatable, load low
        move.l  LoAdr(a0),d7
        add.l   #$101,d7        * leave room for base page
        bclr    #0,d7
        move.l  d7,cseg         * cseg is bottom of mem + $100 (even boundary)
        bra     set2
sldhi:
* relocatable, load high
        move.l  HiAdr(a0),d7
        sub.l   csize,d7
        sub.l   dsize,d7
        sub.l   bsize,d7
        subq.l  #4,d7
        bclr    #0,d7           * put cseg at next even address below
        move.l  d7,cseg         *  high memory - (sum of sizes)
set2:
* Cseg has been set up.  Now do dseg, bseg
        cmpi.w  #$601b,magic
        bne     set3
* if magic # = 601b, take addr from hdr
        move.l  dstart,dseg
        move.l  bstart,bseg
        bra     set4
set3:
* if short header, dseg and bseg follow cseg
        move.l  cseg,d7
        add.l   csize,d7
        addq.l  #1,d7
        bclr    #0,d7
        move.l  d7,dseg
        add.l   dsize,d7
        addq.l  #1,d7
        bclr    #0,d7
        move.l  d7,bseg
set4:
* cseg, dseg, bseg set up
* now find a place for the base page and stack
        moveq.l #3,d2
        bsr     fndseg          * try to fit base page below cseg, dseg, bseg
        tst.l   d1
        beq     set5            * if found, skip
        moveq.l #3,d2
        bsr     trymemtp        * else, try top of memory
        tst.l   d1
        bne     badadr          * if fail, exit
set5:   moveq.l #4,d2
        bsr     trymemtp        * try to fit stack at top of memory
        tst.l   d1
        beq     set6            * if ok, skip
        moveq.l #4,d2
        bsr     fndseg          * else, try to fit below other segs
        tst.l   d1
        bne     badadr
set6:
* now check all segments for conflicts with low and high memory boundaries
        movea.l #cseg,a5
        movea.l #csize,a6
        clr.l   d2
        moveq   #4,d3           * loop counter
set7:   move.l  0(a5,d2),d7     * get segment base
        cmp.l   LoAdr(a0),d7    * above bottom of memory?
        blt     badadr
        add.l   0(a6,d2),d7     * find top of segment
        cmp.l   HiAdr(a0),d7    * below top of memory?
        bgt     badadr
        addq.l  #4,d2           * point to next segment
        dbf     d3,set7
        rts
badadr: moveq.l #1,d0
        rts


movebuf:
* move (d3) bytes from the base page buffer to (a2)
* uses d6
        movea.l basepg,a1
        move.l  #secsize,d6
        sub.w   bufbyts,d6      * address to move from =
        adda.w  d6,a1           *       (basepg) + secsize - (bufbyts)
        sub.w   d3,bufbyts      * update # bytes buffered
        bra     moveb2
moveb1: move.b  (a1)+,(a2)+     * do the move
moveb2: dbf     d3,moveb1
        rts


rdtxt:
* Read code and data text into memory
* during this routine, a2 is always the load address,
*                      d2 is number of bytes left to load
        moveq   #63,d7
        movea.l LoAdr(a0),a5
        movea.l basepg,a6
rdtxt1: move.w  (a5)+,(a6)+     * move header sector to base page
        dbf     d7,rdtxt1
        move.w  #secsize-28,d7
        cmpi.w  #$601a,magic    * short header?
        beq     rdtxt2
        subq.w  #8,d7
rdtxt2: move.w  d7,bufbyts      * indicate # bytes of text in buffer
        move.w  #2,loop         * do for code, data segments
        move.l  cseg,a2         * start at cseg
        move.l  csize,d2        * for csize bytes
rdtxt3:
        clr.l   d3
        move.w  bufbyts,d3
        cmp.l   d2,d3           * # bytes in buffer >= # bytes to load?
        blt     rdtxt4
        move.l  d2,d3
        bsr     movebuf         * if yes, move # bytes to load
        bra     finrd
rdtxt4:
        sub.l   d3,d2           * if no, update # bytes to load
        bsr     movebuf         * move remainder of buffer
        move.l  #secsize,d3     * d3 = secsize fo following loop
rdtxt5:
        cmp.l   d3,d2           * have at least one more full sector?
        blt     rdtxt6
        move.l  a2,d1
        bsr     setdma          * if yes, set up dma address
        bsr     readseq         * read next sector
        tst.w   d1
        bne     rdbad           * if no good, exit
        sub.l   d3,d2           * decrement # bytes to load
        adda.l  #secsize,a2     * increment dma address
        bra     rdtxt5
rdtxt6:
        tst.l   d2              * any more bytes to read?
        beq     finrd
        move.l  basepg,d1
        bsr     setdma
        bsr     readseq         * if yes, read into base page
        tst.w   d1
        bne     rdbad
        move.w  d3,bufbyts      * indicate that we've buffered a sector
        move.l  d2,d3
        bsr     movebuf         * move remainder of segment
finrd:
        move.l  dseg,a2         * set up to load data segment
        move.l  dsize,d2
        sub.w   #1,loop
        bne     rdtxt3
        move.l  bseg,a2         * clear the bss segment
        move.l  bsize,d2
        beq     rdtxt8
rdtxt7: clr.b   (a2)+
        subq.l  #1,d2
        bne     rdtxt7
rdtxt8: rts

rdbad:  moveq.l #2,d0
        rts


relocword:
* relocate word at (a2) based on reloc bits at (a3)
* lsb of d2 indicates whether previous word was 1st half of long-word
        move.w  (a3)+,d7        * get relocation info
        andi.w  #7,d7           * strip off symbol table bits
        lsl     #1,d7           * multiply by 2
        jmp     2(pc,d7)

        bra     relabs
        bra     reldata
        bra     relcode
        bra     relbss
        bra     relbad
        bra     rellong
        bra     relbad
        bra     relop

relbad: move.l  (sp)+,d0        * pop return address
        moveq   #3,d0           * return bad relocation to main routine
        rts

relabs:
relop:  bclr    #0,d2           * reset long word flag
        tst.w   (a2)+           * point to next word of segment
        rts

rellong:
        bset    #0,d2           * set long word flag
        tst.w   (a2)+           * point to next word of segment
        rts

reldata:
relbss:
relcode:
        bclr    #0,d2           * long word flag set?
        bne     relc1           * if yes, skip
        move.w  (a2),d6
        add.w   d5,d6
        move.w  d6,(a2)+
        rts

relc1:  tst.w   -(a2)           * point to first word of long
        move.l  (a2),d6
        add.l   d5,d6
        move.l  d6,(a2)+        * note that a2 points past long word
        rts


reloc:
* Modify address references of code and data segments based on relocation bits
* During this routine,
* a2 points to text file to relocate
* a3 points to relocation word in basepg
* lsb of d2 is long word flag (set on reloc type 5, reset on next word)
* d3 is # words in relocation buffer
* d4 is nmbr of words left to relocate
* d5 is relocation offset

        move.l  basepg,d1
        bsr     setdma          * we will always read into base page
* skip past the symbol table
        move.l  symsize,d7
        divu    #secsize,d7     * calculate how many sectors to skip
* note that max # symbols is 8k, which is 896 sectors of 128 bytes
        move.w  d7,d6           * d6 is nmbr sectors to skip
        swap    d7              * d7 is nmbr bytes to skip
        move.w  bufbyts,d3
        sub.w   d7,d3           * subtract bytes to skip from buffer
        bge     skip1
        addi    #secsize,d3     *if amt in buffer < # bytes to skip,
        addq    #1,d6           *  read in 1 extra sector
skip1:  move.l  basepg,a3
        adda    #secsize,a3
        suba.w  d3,a3           * set up a3 to point to buffer
        lsr     #1,d3           * d3 is nmbr words in buffer
        bra     skip3
skip2:
        bsr     readseq         * read next symbol table sector
        tst.w   d1
        bne     rdbad
skip3:  dbf     d6,skip2
* we got past symbol table
* a3, d3 are set up
        move.l  cseg,d5
        move.l  d5,a2           * relocate cseg first
        sub.l   tstart,d5       * d5 contains the relocation offset
        move.l  csize,d4        * nmbr of bytes to relocate
        move.w  #2,loop         * we're going to relocate 2 segments
reloc1:
* relocate one segment
        clr.l   d2              * clear long word flag
        lsr.l   #1,d4           * make d4 indicate # words
        bra     reloc4
reloc2:
        subq.w  #1,d3
        bpl     reloc3
        bsr     readseq         * if no more words in buffer, refill it
        tst.w   d1
        bne     rdbad
        move.l  basepg,a3
        move.w  #(secsize/2)-1,d3
reloc3:
        bsr     relocword       * relocate one word
        subq.l  #1,d4
reloc4:
        tst.l   d4              * any more to relocate in this segment?
        bne     reloc2          * if yes, do it
        move.l  dseg,a2         * else, set up for dseg
        move.l  dsize,d4
        sub.w   #1,loop
        bne     reloc1
        rts


setrtn:
* Set up the return parameters in Ld Parm Blk and Base Page
        move.l  basepg,BasPage(a0)
        move.l  stk,d7
        add.l   stksize,d7
        bclr    #0,d7
        move.l  d7,Stack(a0)
        move.l  basepg,a1
        move.l  LoAdr(a0),(a1)+
        move.l  HiAdr(a0),(a1)+
        move.l  cseg,(a1)+
        move.l  csize,(a1)+
        move.l  dseg,(a1)+
        move.l  dsize,(a1)+
        move.l  bseg,(a1)+
        move.l  bsize,(a1)
* find size of free memory after bss segment
        move.l  HiAdr(a0),d7    * d7 contains next segment above bss
        move.l  -4(a1),d6
        add.l   (a1)+,d6        * d6 points to start of free mem after bss
        movea.l #cseg,a6        * a6 points to segment to try
        moveq   #4,d5           * try for all segments
        clr.l   bseg            *    but force bss not to appear
setb1:  cmp.l   (a6),d6         * segment above bss?
        bhi     setb2
        cmp.l   (a6),d7         * segment is above bss. Is it below previous?
        bls     setb2
        move.l  (a6),d7
setb2:  tst.l   (a6)+           * point to next segment
        dbf     d5,setb1
        sub.l   d6,d7           * diff between bss top and next segment abv
        move.l  d7,(a1)+
*                       now put disk number that we loaded from into base page
        movea.l FCBPtr(a0),a2
        move.b  (a2),d0         * get disk select byte
        bne     setb3           * if not auto-select, skip
        move    #25,d0
        trap    #2              * get default disk
        addq    #1,d0           * we want it in range of 1..16
setb3:  move.b  d0,(a1)+        * move disk number into base page
        clr.l   d0              * function OK
        rts


        .bss

* offsets from start of parameter block
FCBPtr  = 0
LoAdr   = 4
HiAdr   = 8
BasPage = 12            * return parameters
Stack   = 16
Flags   = 21

hdr:
                                * load file header is read into here
magic:          .ds.w   1
csize:          .ds.l   1
dsize:          .ds.l   1
bsize:          .ds.l   1
bpsize:         .ds.l   1       * symb tbl size is swapped with base page size
stksize:        .ds.l   1
tstart:         .ds.l   1
rlbflg:         .ds.w   1
dstart:         .ds.l   1
bstart:         .ds.l   1

cseg:           .ds.l   1
dseg:           .ds.l   1
bseg:           .ds.l   1
basepg:         .ds.l   1
stk:            .ds.l   1

symsize:        .ds.l   1
temp:           .ds.l   1
loop:           .ds.w   1
bufbyts:        .ds.w   1

        .end
                                                                           w   1
bufbyts:        .ds.w   1

        .end
                                                                           w   1
bufbyts:        .ds.w   1

        .end
                                                                           w   1
bufbyts:        .ds.w   1

        .end
                                                                           