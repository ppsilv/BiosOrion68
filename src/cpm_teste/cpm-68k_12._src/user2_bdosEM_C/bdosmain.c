/****************************************************************
*                                                               *
*               CP/M-68K BDOS Main Routine                      *
*                                                               *
*       This is the main routine for the BDOS for CP/M-68K      *
*       It has one entry point, _bdos, which is  called from    *
*       the assembly language trap handler found in bdosif.s.   *
*       The parameters are a function number (integer) and an   *
*       information parameter (which is passed from bdosif as   *
*       both an integer and a pointer).				*
*       The BDOS can potentially return a pointer, long word,   *
*       or word                                                 *
*                                                               *
*       Configured for Alcyon C on the VAX                      *
*                                                               *
****************************************************************/

#include "bdosinc.h"            /* Standard I/O declarations */
#include "bdosdef.h"            /* Type and structure declarations for BDOS */
#include "biosdef.h"            /* Declarations of BIOS functions */

/*  Declare EXTERN functions */






EXTERN void     warmboot(WORD action);

EXTERN BOOLEAN  constat();      /* Console status               */
EXTERN UBYTE    conin();        /* Console Input function       */
EXTERN void     tabout();       /* Console output with tab expansion */
EXTERN UBYTE    rawconio(UWORD parm);     /* Raw console I/O              */
EXTERN void     prt_line(UBYTE *string_ptr);  /* Print line until delimiter   */
EXTERN void     readline(UBYTE *buffer_ptr);     /* Buffered console read        */
EXTERN void     seldsk(int);       /* Select disk                  */
EXTERN BOOLEAN  openfile();     /* Open File                    */
EXTERN UWORD    close_fi(UBYTE *fcb_ptr);     /* Close File                   */
EXTERN UWORD    search(UBYTE *fcb_ptr, WORD search_type, void *temp_ptr);     /* Search first and next fcns   */
//EXTERN UWORD    dirscan(UWORD (*func)(), UBYTE *fcb_ptr, WORD flags);/* General directory scanning routine */
//EXTERN UWORD    bdosrw();       /* Sequential and Random disk read/write */
EXTERN UWORD    bdosrw(UBYTE *fcb_ptr, WORD read_flag, WORD mode);
EXTERN BOOLEAN  create();       /* Create file                  */
EXTERN BOOLEAN  delete();       /* Delete file                  */
EXTERN BOOLEAN  rename();       /* Rename file                  */
EXTERN BOOLEAN  set_attr();     /* Set file attributes          */
EXTERN void     getsize(UBYTE*);      /* Get File Size                */
EXTERN void     setran(UBYTE*);       /* Set Random Record            */
EXTERN void     free_sp(UWORD );      /* Get Disk Free Space          */
EXTERN UWORD    flushit();      /* Flush Buffers                */
EXTERN UWORD    dirscan(void *func, UBYTE *fcb_ptr, WORD flags);

EXTERN UWORD    pgmld(UBYTE *p1, BYTE *p2);   /* Program Load                 */
EXTERN UWORD    setexc(UBYTE*);       /* Set Exception Vector         */
EXTERN void     set_tpa(UBYTE*);      /* Get/Set TPA Limits           */
EXTERN void     move(BYTE *source, BYTE *destination, UWORD length);  /* general purpose byte mover   */

//#define GBL (*gblptr)
// ou
#define GBL gbls 

/*  Declare "true" global variables; i.e., those which will pertain to the
    entire file system and thus will remain global even when this becomes
    a multi-tasking file system */

GLOBAL UWORD    log_dsk;        /* 16-bit vector of logged in drives */
GLOBAL UWORD    ro_dsk;         /* 16-bit vector of read-only drives */
GLOBAL UWORD    crit_dsk;       /* 16-bit vector of drives in "critical"
                                   state.  Used to control dir checksums */
GLOBAL BYTE     *tpa_lp;        /* TPA lower boundary (permanent)       */
GLOBAL BYTE     *tpa_lt;        /* TPA lower boundary (temporary)       */
GLOBAL BYTE     *tpa_hp;        /* TPA upper boundary (permanent)       */
GLOBAL BYTE     *tpa_ht;        /* TPA upper boundary (temporary)       */


/*  Declare the "state variables".  These are globals for the single-thread
    version of the file system, but are put in a structure so they can be
    based, with a pointer coming from the calling process               */

GLOBAL struct stvars gbls;

struct tempstr
{
      UBYTE     tempdisk;
      BOOLEAN   reselect;
      struct fcb *fptr;
};

/* temporarily select disk pointed to by fcb */
void tmp_sel(struct tempstr *temp_ptr){
    REG struct fcb *fcbp;
    REG UBYTE tmp_dsk;
    BSETUP

    fcbp = temp_ptr->fptr;        /* get local copy of fcb pointer */     
    tmp_dsk = (temp_ptr->tempdisk = fcbp->drvcode);
    seldsk( tmp_dsk ? tmp_dsk - 1 : GBL.dfltdsk );

    fcbp->drvcode = GBL.user;
    temp_ptr->reselect = TRUE;
;
}




/****************************************************************
*                                                               *
*               _bdos MAIN ROUTINE                              *
*                                                               *
*       Called with  _bdos(func, info, infop)                   *
*                                                               *       
*       Where:                                                  *
*               func    is the BDOS function number (d0.w)      *
*               info    is the word parameter (d1.w)            *
*               infop   is the pointer parameter (d1.l)         *
*                       note that info is the word form of infop*
*                                                               *
****************************************************************/


UWORD _bdos(func,info,infop)
REG WORD func;          /* BDOS function number */
REG UWORD info;         /* d1.w word parameter  */
REG UBYTE *infop;       /* d1.l pointer parameter */
{
    REG UWORD rtnval;
    LOCAL struct tempstr temp;
    struct fcb *fcb_ptr;
    BSETUP
    fcb_ptr = (struct fcb *)infop;

        temp.reselect = FALSE;
        temp.fptr =(struct fcb *) infop;
        rtnval = 0;

        switch (func)   /* switch on function number */
        {
          case 0:   warmboot(0);                /* warm boot function */
                    /* break; */

          case 1:   return((UWORD)conin());     /* console input function */
                    /* break; */

          case 2:   tabout(); // tabout((UBYTE)info);        /* console output with  */
                    break;                      /*    tab expansion     */

          case 3:   return((UWORD)brdr());      /* get reader from bios */
                    /* break; */

          case 4:   bpun((UBYTE)info);          /* punch output to bios */
                    break;

          case 5:   blstout((UBYTE)info);       /* list output from bios */
                    break;

          case 6:   return((UWORD)rawconio(info)); /* raw console I/O */
                    /* break; */

          case 7:   return(bgetiob());          /* get i/o byte */
                    /* break; */

          case 8:   bsetiob(info);              /* set i/o byte function */
                    break;

          case 9:   prt_line(infop);            /* print line function */
                    break;

          case 10:  readline(infop);            /* read buffered con input */
                    break;

          case 11:  return((UWORD)constat());   /* console status */
                    /* break; */

          case 12:  return(VERSION);            /* return version number */
                    /* break; */

          case 13:  log_dsk = 0;                /* reset disk system */
                    ro_dsk  = 0;
                    crit_dsk= 0;
                    GBL.curdsk = 0xff;
                    GBL.dfltdsk = 0;
                    break;

          case 14:  seldsk((UBYTE)info);        /* select disk */
                    GBL.dfltdsk = (UBYTE)info;
                    break;

    case 15:  tmp_sel((struct tempstr *)&temp);             /* open file */
          fcb_ptr->extent = 0;        /* Mudado infop para fcb_ptr */
          fcb_ptr->s2 = 0;            /* Mudado infop para fcb_ptr */
          rtnval = dirscan(openfile, infop, 0);
          break;

          case 16:  tmp_sel((struct tempstr *)&temp);             /* close file */
                    rtnval = close_fi(infop);
                    break;


          case 17:  GBL.srchp = (struct fcb *)infop;      /* CORRIGIDO: Adicionado (struct fcb *) */
                    rtnval = search(infop, 0, &temp);
                    break;

          case 18:  infop = (UBYTE *)GBL.srchp;           /* CORRIGIDO: Adicionado (UBYTE *) */
                    temp.fptr = (struct fcb *)infop;      /* CORRIGIDO: Adicionado (struct fcb *) */
                    rtnval = search(infop, 1, &temp);
                    break;



          case 19:  tmp_sel((struct tempstr *)&temp);             /* delete file */
                    rtnval = dirscan(delete, infop, 2);
                    break;

          case 20:  tmp_sel((struct tempstr *)&temp);             /* read sequential */
                    rtnval = bdosrw(infop, TRUE, 0);
                    break;

          case 21:  tmp_sel((struct tempstr *)&temp);             /* write sequential */
                    rtnval = bdosrw(infop, FALSE, 0);
                    break;

case 22:  tmp_sel((struct tempstr *)&temp);             /* create file */
          fcb_ptr->extent = 0;        /* Mudado infop para fcb_ptr */
          fcb_ptr->s1 = 0;            /* Mudado infop para fcb_ptr */
          fcb_ptr->s2 = 0;            /* Mudado infop para fcb_ptr */
          fcb_ptr->rcdcnt = 0;        /* Mudado infop para fcb_ptr */
          rtnval = dirscan(create, infop, 8);
          break;

          case 23:  tmp_sel((struct tempstr *)&temp);             /* rename file */
                    rtnval = dirscan(rename, infop, 2);
                    break;

          case 24:  return(log_dsk);            /* return login vector */
                    /* break; */

          case 25:  return(UBWORD(GBL.dfltdsk)); /* return current disk */
                    /* break; */

          case 26:  GBL.dmaadr = infop;         /* set dma address */
                    break;

          /* No function 27 -- Get Allocation Vector */

          case 28:  ro_dsk |= 1<<GBL.dfltdsk;   /* set disk read-only */
                    break;

          case 29:  return(ro_dsk);             /* get read-only vector */
                    /* break; */

          case 30:  tmp_sel((struct tempstr *)&temp);             /* set file attributes */
                    rtnval = dirscan(set_attr, infop, 2);
                    
                    break;

          case 31:  if (GBL.curdsk != GBL.dfltdsk) seldsk(GBL.dfltdsk);
                    move( (BYTE *)(GBL.parmp), infop, sizeof *(GBL.parmp) );
                    break;              /* return disk parameters */

          case 32:  if ( (info & 0xff) <= 15 )  /* get/set user number */
                        GBL.user = (UBYTE)info;
                    return(UBWORD(GBL.user));
                    /* break; */

          case 33:  tmp_sel((struct tempstr *)&temp);             /* random read */
                    rtnval = bdosrw(infop, TRUE, 1);
                    break;

          case 34:  tmp_sel((struct tempstr *)&temp);             /* random write */
                    rtnval = bdosrw(infop, FALSE, 1);
                    break;

          case 35:  tmp_sel((struct tempstr *)&temp);             /* get file size */
                    getsize(infop);
                    break;

          case 36:  tmp_sel((struct tempstr *)&temp);             /* set random record */
                    setran(infop);
                    break;

          case 37:  info = ~info;               /* reset drive */
                    log_dsk &= info;
                    ro_dsk  &= info;
                    crit_dsk &= info;
                    break;

          case 40:  tmp_sel((struct tempstr *)&temp);             /* write random with 0 fill */
                    rtnval = bdosrw(infop, FALSE, 2);
                    break;

          case 46:  free_sp(info);              /* get disk free space */
                    break;

          case 47:  chainp = GBL.dmaadr;    	/*sw chain to program */
                    warmboot(0);                /* terminate calling program */
                    /* break; */

          case 48:  return( flushit() );        /* flush buffers        */
                    /* break; */

          case 59:  return(pgmld(infop,GBL.dmaadr));  /* program load */
                    /* break; */

          case 61:  return(setexc(infop));      /* set exception vector */
                    /* break; */

          case 63:  set_tpa(infop);             /* get/set TPA limits   */
                    break;

          default:  return(-1);                 /* bad function number */
                    /* break; */

        };                                      /* end of switch statement */
        if (temp.reselect) fcb_ptr->drvcode = temp.tempdisk; 
                                /* if reselected disk, restore it now */

        return(rtnval);                 /* return the BDOS return value */
}                                       /* end _bdos */


