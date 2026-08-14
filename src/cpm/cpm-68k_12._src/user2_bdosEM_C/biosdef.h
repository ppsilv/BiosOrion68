
/********************************************************
*                                                       *
*       BIOS definitions for CP/M-68K                   *
*                                                       *
*       Copyright (c) 1982 Digital Research, Inc.       *
*                                                       *
*       This include file simply defines the BIOS calls *
*                                                       *
********************************************************/

EXTERN UBYTE    bios1(int parm, ...);        /* used for character I/O functions */
/* parm1 is word, no return value   */
EXTERN WORD     bios2(WORD function_code, WORD parameter);
/* used for set dma only            */
EXTERN void     bios3(WORD function_code, BYTE *pointer_param);

                                /* parm1 is a pointer, no return    */
/* seldsk only, parm1 and parm2 are */
EXTERN struct dph *bios4(WORD function_code, WORD parameter1, WORD parameter2);
                                /*   words, returns a pointer to dph */
/*EXTERN UWORD    bios5();         for sectran and set exception    */
EXTERN ULONG    bios5(UWORD func, ...);
EXTERN BYTE     *bios6(UWORD func, ...);       /* for get memory segment table     */


#define bwboot()        bios1(1)        /* warm boot            */      
#define bconstat()      bios1(2)        /* console status       */
#define bconin()        bios1(3)        /* console input        */
#define bconout(parm)   bios2(4,parm)   /* console output parm  */
#define blstout(parm)   bios2(5,parm)   /* list device output   */
#define bpun(parm)      bios2(6,parm)   /* punch char output    */
#define brdr()          bios1(7)        /* reader input         */
#define bhome()         bios1(8)        /* recalibrate drive    */
#define bseldsk(parm1,parm2) bios4(9,parm1,parm2)
                                        /* select disk and return info */
#define bsettrk(parm)   bios2(10,parm)  /* set track on disk    */
#define bsetsec(parm)   bios2(11,parm)  /* set sector for disk  */
#define bsetdma(parm)   bios3(12,parm)  /* set dma address      */
#define bread()         bios1(13)       /* read sector from disk */
#define bwrite(parm)    bios2(14,parm)  /* write sector to disk */
#define blistst()       bios1(15)       /* list device status   */
#define bsectrn(parm1,parm2) bios5(16,parm1,parm2)
                                        /* sector translate     */
#define bgetseg()       bios6(18)        /* get memory segment tbl */
#define bgetiob()       bios1(19)       /* get I/O byte         */
#define bsetiob(parm)   bios2(20,parm)  /* set I/O byte         */
#define bflush()        bios1(21)       /* flush buffers        */
#define bsetvec(parm1,parm2) bios5(22,parm1,parm2)
                                        /* set exception vector */
