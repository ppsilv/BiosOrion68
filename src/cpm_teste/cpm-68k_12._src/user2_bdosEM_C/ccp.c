/*--------------------------------------------------------------*\
 |	ccp.c	      CONSOLE COMMAND PROCESSOR          v1.1   |
 |		      =========================			|
 |								|
 |      CP/M 68k:  A CP/M derived operating system		|
 |								|
 |     *==================================================*	|
 |     *==================================================*	|
 |     *THIS IS THE DUAL PROCESSOR,ROMABLE CP/M-68K SYSTEM*	|
 |     *==================================================*	|
 |     *==================================================*	|
 |								|
 |      Description:					        |
 |	-----------						|
 |			The Console Command Processor is a      |
 |			distinct program which references       |
 |			the BDOS to provide a human-oriented    |
 |			interface for the console user to the   |
 |			information maintained by the BDOS on   |
 |			disk storage.				|
 |								|
 |	created by :    Tom Saulpaugh	Date created: 7/13/82	|
 |	----------			------------		|
 |      last modified:  03/17/84 sw	St. Patrick's Day!!!	|
 |      -------------	Chain hack	--------------------	|
 |     								|
 |	(c) COPYRIGHT  Digital Research 1983, 1984		|
 |	               all rights reserved			|
 |								|
\*--------------------------------------------------------------*/

/*--------------------------------------------------------------*\
 |		     CCP Macro Definitions			|
\*--------------------------------------------------------------*/
#include	"ccpdef.h"	  /* include CCP defines	*/

			
/*--------------------------------------------------------------*\
 |		    CP/M Builtin Command Table			|
\*--------------------------------------------------------------*/
struct _cmd_tbl
{
	BYTE	*ident;		/* command identifer field	*/
	UWORD	cmd_code;	/* command code field		*/
}
cmd_tbl[8] = 			/* declare CP/M built-in table	*/
{
	"DIR",DIRCMD,
       "DIRS",DIRSCMD,
       "TYPE",TYPECMD,
	"REN",RENCMD,
	"ERA",ERACMD,
       "USER",UCMD,
     "SUBMIT",SUBCMD,
	 NULL,-1
};		
/*--------------------------------------------------------------*\
 |		CP/M-68K COMMAND FILE LOADER TABLE		|
\*--------------------------------------------------------------*/
extern struct _filetyps
{
	BYTE *typ;
	UWORD (*loader) ();
	BYTE user_c;
	BYTE user_0;
}
load_tbl[];

/*--------------------------------------------------------------*\
 |		 Table of User Prompts and Messages		|
\*--------------------------------------------------------------*/
BYTE	msg[]  = "NON-SYSTEM FILE(S) EXIST$";
BYTE	msg2[] = "Enter Filename: $";
BYTE	msg3[] = "Enter Old Name: $";
BYTE	msg4[] = "Enter New Name: $";
BYTE	msg5[] = "File already exists$";
BYTE	msg6[] = "No file$";
BYTE	msg7[] = "No wildcard filenames$";
BYTE	msg8[] = "Syntax: REN Newfile=Oldfile$";
BYTE	msg9[] = "Confirm(Y/N)? $";
BYTE   msg10[] = "Enter User No: $";
BYTE   msg11[] = ".SUB file not found$";
BYTE   msg12[] = "User # range is [0-15]$";
BYTE   msg13[] = "Too many arguments: $";
BYTE  lderr1[] = "Insufficient memory or bad file header$";
BYTE  lderr2[] = "Read error on program load$";
BYTE  lderr3[] = "Bad relocation information bits$";
BYTE lderror[] = "Program load error$";
/*--------------------------------------------------------------*\
 |		    Global Arrays & Variables			|
\*--------------------------------------------------------------*/

				/********************************/
BYTE load_try;			/* flag to mark a load try	*/
BYTE first_sub;			/* flag to save current cmd ptr */
BYTE chain_sub;			/* submit chaining flag		*/
extern BYTE submit;		/* submit file flag		*/
BYTE end_of_file;		/* submit end of file flag	*/
BYTE dirflag;			/* used by fill_fcb(? or blanks)*/
BYTE subprompt;			/* submit file was prompted for */
extern BYTE morecmds;		/* command after warmboot flag  */
UWORD sub_index;		/* index for subdma buffer	*/
UWORD index;			/* index into cmd argument array*/ 
UWORD sub_user;			/* submit file user number      */
UWORD user;			/* current default user number  */
UWORD cur_disk;			/* current default disk drive   */
BYTE subcom[CMD_LEN+1];		/* submit command buffer	*/
BYTE subdma[CMD_LEN];		/* buffer to fill from sub file */
extern BYTE usercmd[CMD_LEN+2];	/* user command buffer		*/
BYTE *user_ptr;			/* next user command to execute */
BYTE *glb_index;		/* points to current command	*/
BYTE save_sub[CMD_LEN+1];	/* saves cur cmd line for submit*/
BYTE subfcb[FCB_LEN];		/* global fcb for sub files	*/
BYTE cmdfcb[FCB_LEN];		/* global fcb for 68k files	*/
BYTE *tail;			/* pointer to command tail      */
extern BYTE autost;		/* autostart flag		*/
BYTE autorom;			/* needed for ROM system autost */
BYTE dma[DMA_LEN+3];		/* 128 byte dma buffer		*/
BYTE parm[MAX_ARGS][ARG_LEN];	/* cmd argument array		*/
BYTE *chainp;			/*sw -> User-specified command  */
BYTE del[] =	 		/* CP/M-68K set of delimeters   */
{'>','<','.',',','=','[',']',';','|','&','/','(',')','+','-','\\'};
				/********************************/

/*--------------------------------------------------------------*\
 |		      Function Definitions			|
\*--------------------------------------------------------------*/

				/********************************/
//extern UWORD bdos();		/* this returns a word          */
//extern UWORD load68k();		/* this returns a word(1-3)	*/
//BYTE *scan_cmd();		/* this returns a ptr to a byte */
//UWORD strcmp();			/* this returns a word		*/
//UWORD decode(BYTE CMD);			/* this returns a word		*/
//UWORD delim();			/* this returns a word		*/
//BYTE true_char();		/* this returns a byte		*/
//UWORD fill_fcb();		/* this returns a word		*/
//UWORD too_many();		/* this returns a word		*/	
//UWORD find_colon();		/* this returns a word		*/
//UWORD chk_colon();		/* this returns a word		*/
//UWORD user_cmd();		/* this returns a word		*/
//UWORD cmd_file();		/* this returns a word		*/
//UWORD sub_read();		/* this returns a word		*/
//UWORD dollar();			/* this returns a word		*/
//UWORD comments();		/* this returns a word		*/
//UWORD submit_cmd();		/* this returns a word		*/
				/********************************/
/* Chamada principal do BDOS a partir do CCP */
UWORD bdos(UWORD func, ULONG param);
/* Carrega arquivo executável .68K na memória */
UWORD load68k(struct fcb *fcbp, ULONG *loadaddr);
/* Analisa a linha de comando e avança o ponteiro */
BYTE *scan_cmd(BYTE *p);
/* Compara duas strings */
UWORD strcmp( BYTE *s1,  BYTE *s2);
/* Decodifica caractere/comando */
UWORD decode(BYTE *cmd);
/* Verifica se o caractere é um delimitador */
UWORD delim(BYTE * c);
/* Retorna caractere normalizado/maiúsculo */
BYTE true_char(BYTE *ch)	;
/* Preenche o FCB a partir da string digitada */
UWORD fill_fcb(UWORD which_parm,BYTE *fcb);
/* Checa excesso de parâmetros na linha de comando */
UWORD too_many(void);
/* Localiza dois pontos (unidade de disco) */
UWORD find_colon();
/* Valida sintaxe da unidade de disco */
UWORD chk_colon(UWORD j);
/* Executa comando interno de troca de usuário */
UWORD user_cmd();
/* Tenta executar arquivo de comando (.68K) */
UWORD cmd_file(UWORD mode);
/* Lê lote do arquivo de SUBMIT ($$.SUB) */
UWORD sub_read(void);
/* Processa parâmetros com cifrão ($1, $2, etc) no SUBMIT */
UWORD dollar(UWORD k,UWORD mode,BYTE *com_index);	
/* Ignora comentários em scripts SUBMIT */
UWORD comments(UWORD k, BYTE *com_index);
/* Processa e inicializa o arquivo SUBMIT */
UWORD submit_cmd(BYTE *p);

UWORD (*ldrpgm)(void*);

				/********************************/
			/*   print a CR and a Linefeed	*/
				/********************************/
void cr_lf(){
	bdos(CONSOLE_OUTPUT,(BYTE)CR);
	bdos(CONSOLE_OUTPUT,(BYTE)LF);
}
				/********************************/
				/*  copy source to destination  */
				/********************************/
void cpy(BYTE *source,BYTE *dest)		
{
	while(*dest++ = *source++);
}


				/********************************/

UWORD strcmp(s1,s2)	 				
REG BYTE *s1,*s2;
{		
	while(*s1)
	{
		if(*s1 > *s2)
			return(1);
		if(*s1 < *s2)
			return(-1);
		s1++; s2++;
	}
	return((*s2 == NULL) ? 0 : -1);
}
				/********************************/
void copy_cmd(com_index)	/*  Save the command which	*/
				/*  started a submit file       */
				/*  Parameter substituion will  */
				/*  need this command tail.	*/
				/*  The buffer save_sub is used */
				/*  to store the command.	*/
				/********************************/ 
REG BYTE *com_index;
{
	REG BYTE *t1,*temp;

	temp = save_sub;
	if(subprompt)
	{
		t1 = (char *)parm;
		while(*t1)
			*temp++ = *t1++;
		*temp++ = ' ';
		subprompt = FALSE;
	}
	while(*com_index && *com_index != EXLIMPT)
		*temp++ = *com_index++;
	*temp = NULL;
}

				/********************************/
void prompt()  			/*   print the CCP prompt       */
				/********************************/
{
	REG UWORD cur_drive,cur_user_no;
	BYTE      buffer[3];

	cur_user_no = bdos(GET_USER_NO,(long)255);
	cur_drive  = bdos(RET_CUR_DISK,(long)0);		
	cur_drive += 'A';
	cr_lf();
	if(cur_user_no)
	{
		if(cur_user_no >= 10)
		{
			buffer[0] = '1';
			buffer[1] = ((cur_user_no-10) + '0');
			buffer[2] = '$';
		}
		else
		{
			buffer[0] = (cur_user_no + '0');
			buffer[1] = '$';
		}
		bdos(PRINT_STRING,(BYTE)buffer);
	}
	bdos(CONSOLE_OUTPUT,(long)cur_drive);
	bdos(CONSOLE_OUTPUT,(BYTE)ARROW);
}



				/********************************/
	/* echo any multiple commands   */
				/* or any illegal commands	*/
				/********************************/
void echo_cmd(cmd,mode)					
REG BYTE *cmd;
REG UWORD mode;
{
	if(mode == GOOD && (!(autost && autorom)))
		prompt();
	while(*cmd && *cmd != EXLIMPT)
		bdos(CONSOLE_OUTPUT,(long)*cmd++);
	if(mode == BAD)
		bdos(CONSOLE_OUTPUT,(long)'?');
	else
		cr_lf();
}

/********************************/	
/* Recognize the command as:	*/
/* ---------			*/
/* 1. Builtin			*/
/* 2. File			*/
/********************************/				

UWORD decode(BYTE *cmd){
	REG UWORD i,n;


	/****************************************/
	/* Check for a CP/M builtin command	*/
	/****************************************/
	for(i = 0; i < 7;i++)
		if (strcmp((BYTE*)cmd,(BYTE*)cmd_tbl[i].ident) == MATCH)
			return(cmd_tbl[i].cmd_code);
	/********************************************************/
	/*	Check for a change of disk drive command	*/
	/********************************************************/
	i = 0;
	while(i < (ARG_LEN-1) && parm[0][i] != ':')
		i++;
	if(i == 1 && parm[0][2] == NULL && parm[1][0] == NULL)
		if((parm[0][0] >= 'A') && (parm[0][0] <= 'P')) 
				return(CH_DISK);
	if(i == 1 && ((parm[0][0] < 'A') || (parm[0][0] > 'P')))
		return(-1);
	if(i != 1 && parm[0][i] == ':')
		return(-1);
	/*****************************************************/
	/* Check for Wildcard Filenames			     */
	/* Check Filename for a Delimeter 		     */
	/*****************************************************/
	if(fill_fcb(0,cmdfcb) > 0)
		return(-1);
	if(i == 1)
		i = 2;
	else
		i = 0;
	if(delim(&parm[0][i]))
		return(-1);
	for(n = 0;n < ARG_LEN-1 && parm[0][n];n++)
		if(parm[0][n] < ' ')
			return(-1);
	return(FILE);
}
					/************************/
void check_cmd(tcmd)			/*  Check end of cmd	*/
					/*  for an '!' which    */
					/*  starts another cmd  */
REG BYTE *tcmd;				/************************/
{
	while(*tcmd && *tcmd != EXLIMPT)
		tcmd++;
	/*----------------------------*/
	/* check for multiple command */
	/*   in case of a warmboot    */	
	/*----------------------------*/
	if(*tcmd++ == EXLIMPT && *tcmd)
	{
		morecmds = TRUE;
		while(*tcmd == ' ')
			tcmd++;
		user_ptr = tcmd;
	}
	else
		if(submit)	/* check original cmd line */
		{
			if(!(end_of_file))
				morecmds = TRUE;
				/*--------------------------*/
			else	/* restore cmd to where user*/
				/* ptr points to. User_ptr  */
				/* always points to next    */
				/* console command to exec  */
			{	/*--------------------------*/
				submit = FALSE;
				if(*user_ptr)
					morecmds = TRUE;
			}
		}
		else
			morecmds = FALSE;		
}
					/************************/
        /*   Read in a command  */
					/*Strip off extra blanks*/
					/************************/
void get_cmd(BYTE *cmd,long max_chars)						
{
	REG BYTE *c;

	//max_chars += (cmd - 1);
	max_chars += (long)(cmd - 1);
	dma[0] = CMD_LEN;	  /* set maximum chars to read  */
	bdos(READ_CONS_BUF,(BYTE)dma);  /* then read console		*/
	if(dma[1] != 0 && dma[2] != ';')
		cr_lf();
	dma[((UWORD)dma[1] & 0xFF)+2] = '\n'; /* tack on end of line char   */ 
	if(dma[2] == ';')	  /* ';' denotes a comment	*/	
		dma[2] = '\n';
	c = &dma[2];
	while(*c == ' ' || *c == TAB)
		c++;
	while(*c != '\n' && cmd < max_chars)
	{			
		*cmd++ = toupper(*c);
		if(*c == ' ' || *c == TAB)
			while(*++c == ' ' || *c == TAB);
		else
			c++;
	}
	*cmd = NULL;	     /* tack a null character on the end*/
}
					/************************/
BYTE *scan_cmd(com_index)		/* move ptr to next cmd */
					/* in the command line	*/
					/************************/
REG BYTE *com_index;
{
	while((*com_index != EXLIMPT) && 
	      (*com_index))
		 com_index++;
	while(*com_index == EXLIMPT || *com_index == ' ' ||
	      *com_index == TAB)
		com_index++;
	return(com_index);
}

					/************************/
void get_parms(cmd)			/* extract cmd arguments*/
					/* from command line	*/
REG BYTE *cmd;				/************************/
{



	/************************************************/
	/*	This function parses the command line   */
	/* read in by get_cmd().  The expected command  */
	/* from that line is put into parm[0].  All     */
	/* parmeters associated with the command are put*/
	/* in in sequential order in parm[1],parm[2],   */
	/* and parm[3]. A command ends at a NULL or   	*/
	/* an exlimation point.				*/
	/************************************************/


	REG BYTE *line;		/* pointer to parm array   */
	REG UWORD    i;		/* Row Index   		   */
	REG UWORD    j;		/* Column Index		   */

	line = (char *)parm;
	for(i = 0; i < (MAX_ARGS * ARG_LEN); i++)
		*line++ = NULL;

	i = 0;
	/***************************************************/
	/*  separate command line at blanks,exlimation pts */
	/***************************************************/

        while(*cmd != NULL    &&
	      *cmd != EXLIMPT &&
	      i < MAX_ARGS)
	{
		j = 0;
		while(*cmd != EXLIMPT &&
		      *cmd != ' '     &&
		      *cmd != TAB     &&	
		      *cmd)
		{
			if(j < (ARG_LEN-1))
				parm[i][j++] = *cmd;
			cmd++;
		}
		parm[i++][j] = NULL;
		if(*cmd == ' ' || *cmd == TAB)
			cmd++;
		if(i == 1)
			tail = cmd; /* mark the beginning of the tail */
	}
}

					/************************/
							/* check ch to see	*/ 
					/* if it's a delimeter  */
					/************************/
UWORD delim(BYTE *ch){
	REG UWORD i;

	if(*ch <= ' ')
		return(TRUE);
	for(i = 0;i < sizeof (del);i++)
		if(*ch == del[i]) return(TRUE);
	return(FALSE);
}



					/************************/
							/* return the desired	*/
					/* character for fcb	*/
					/************************/
BYTE true_char(BYTE *ch)	

{
	if(*ch == '*') return('?');	/* wildcard		*/

	if(!delim(ch)) 			/* ascii character	*/
	{
		index++;		/* increment cmd index	*/
		return(*ch);
	}

	return(' ');			/* pad field with blank */
}
					/************************/
		/* fill the fields of	*/
							/* the file control blk */
					/************************/

UWORD fill_fcb(UWORD which_parm,BYTE *fcb)
{
	REG BYTE  *ptr;
	REG BYTE  fillch;
	REG UWORD j,k;

	*fcb = 0;
	for(k = 12;k <= 35; k++)        /* fill fcb with zero	*/
		fcb[k] = ZERO;
	for(k = 1;k <= 11;k++)
		fcb[k] = BLANK;	/* blank filename+type  */
	
	/*******************************************/
	/* extract drivecode,filename and filetype */
	/*	 from parmeter blk		   */
	/*******************************************/

	if(dirflag)
		fillch = '?';
	else
		fillch = ' ';

	index = ZERO;
	ptr = fcb;
	if(parm[which_parm][index] == NULL) /* no parmemters  */
	{
		ptr++;
		for(j = 1;j <= 11;j++)
			*ptr++ = fillch; 
		*fcb = (bdos(RET_CUR_DISK,(long)0)+1);
		if(dirflag)
			return(11);
		else
			return(0);
	}
	if(parm[which_parm][index+1] == ':')
	{
		*ptr = parm[which_parm][index] - 'A' + 1;
		index += 2;
		if(parm[which_parm][index] == NULL)
		{
			ptr = &fcb[1];
			for(j = 1;j <= 11;j++)
				*ptr++ = fillch;
			if(dirflag)
				return(11);
			else
				return(0);
		}
	}							
	else	/* fill drivecode with the default disk		*/	
	             *fcb = (bdos(RET_CUR_DISK,(long)0) + 1);

	ptr = fcb;
	ptr++;			/* set pointer to fcb filename */
	for(j = 1;j <= 8;j++)	/* get filename */
	  *ptr++ = true_char(&parm[which_parm][index]);
	while((!(delim(&parm[which_parm][index])))) index++;
	if(parm[which_parm][index] == PERIOD)
	{
	  	index++;
       	  	for(j = 1;j <= 3;j++)	/* get extension */
	  	 	*ptr++ = true_char(&parm[which_parm][index]);
	}
	k = 0;
	for(j = 1;j <= 11;j++)
		if(fcb[j] == '?') k++;

	return(k);	/* return the number of question marks	*/
}
					/************************/
UWORD too_many()			/* too many args ?      */
{					/************************/
	if(parm[2][0])
	{
		bdos(PRINT_STRING,(BYTE)msg13);
		echo_cmd(&parm[2][0],BAD);
		return(TRUE);
	}
	return(FALSE);
}
					/************************/
								/*  search for a colon  */
					/************************/			
UWORD find_colon(){					
	REG UWORD i;

	i = 0;
	while(parm[1][i] && parm[1][i] != ':') i++;
	return(i);
}
					/************************/
/* check the position of*/
				/* the colon and for    */
UWORD chk_colon(UWORD j){					/* a legal drive letter */
	if(parm[1][j] == ':')		/************************/
	{
		if(j != 1 || parm[1][0] < 'A' || parm[1][0] > 'P')
		{
			echo_cmd(&parm[1][0],BAD);
			return(FALSE);
		}
	}
	return(TRUE);
}

					/************************/
	       		/*     print out a	*/
					/*  directory listing   */
					/*----------------------*/
					/* attrib->1 (sysfiles) */
					/* attrib->0 (dirfiles) */
					/************************/
void dir_cmd(UWORD attrib){
	BYTE		needcr_lf;
	REG UWORD	dir_index,file_cnt;
 	REG UWORD	save,j,k,curdrive,exist;

	exist = FALSE; needcr_lf = FALSE;
	if(too_many()) return;
	j = find_colon();
	if(!chk_colon(j)) return;
	fill_fcb(1,cmdfcb);
	curdrive = (cmdfcb[0] + 'A' - 1);
	dir_index = bdos(SEARCH_FIRST,(BYTE)cmdfcb);
	if(dir_index == 255)
		bdos(PRINT_STRING,(BYTE)msg6);
	save = (32 * dir_index) + 1;
	file_cnt = 0;		
	while(dir_index != 255)
	{
      		if(((attrib) && (dma[save+9] & 0x80)) ||
		  (!(attrib) && (!(dma[save+9] & 0x80))))
	      	{
			if(needcr_lf)
			{
				cr_lf();
				needcr_lf = FALSE;
			}	
			if(file_cnt == 0)
				bdos(CONSOLE_OUTPUT,(long)curdrive);
	      	}
		else
			{
				exist = TRUE;
				dir_index = bdos(SEARCH_NEXT,(BYTE)0L);
				save = (32 * dir_index) + 1;
				continue;
			}
		dir_index = (32 * dir_index) + 1;
		bdos(CONSOLE_OUTPUT,(BYTE)COLON);
		bdos(CONSOLE_OUTPUT,(BYTE)BLANKS);
		j = 1;
		while(j <= 11)
		{
			if(j == 9)
				bdos(CONSOLE_OUTPUT,BLANKS);
			bdos(CONSOLE_OUTPUT,(long)(dma[dir_index++] & CMASK));
			j++;
		}
		bdos(CONSOLE_OUTPUT,BLANKS);
		dir_index = bdos(SEARCH_NEXT,0L);
		if(dir_index == 255)
			break;
		file_cnt++;
		save = (32 * dir_index) + 1;
		if(file_cnt == 5)
		{
			file_cnt = 0;
			if((attrib && (dma[save+9] & 0x80)) ||
			  (!(attrib) && (!(dma[save+9] & 0x80))))
				cr_lf();
			else
				needcr_lf = TRUE;
		}	 

	}		/*----------------------------------------*/
	if(exist)	/* if files exist that were not displayed */
			/* print out a message to the console	  */
	{		/*----------------------------------------*/
		cr_lf();
		if(attrib)
			bdos(PRINT_STRING,(BYTE)msg);
		else
			bdos(PRINT_STRING,(BYTE)&msg[4]);
	}
}

					/************************/
			/*   type out a file	*/
					/*   to the console 	*/
					/************************/
void type_cmd()	{
	REG 	UWORD i;

	if(parm[1][0] == NULL)		/*prompt user for filename*/
	{
		bdos(PRINT_STRING,(BYTE)msg2);
		get_cmd(&parm[1][0],(long)(ARG_LEN-1));
	}
	if(too_many())
		return;
	i = find_colon();
	if(!chk_colon(i)) return;
	i = fill_fcb(1,cmdfcb);		/*fill a file control block*/
	if(i == 0 && parm[1][0] && (bdos(OPEN_FILE,(BYTE)cmdfcb) <= 3))
	{
		while(bdos(READ_SEQ,(BYTE)cmdfcb) == 0)
		{
			for(i = 0;i <= 127;i++)
				if(dma[i] != EOF)
				    bdos(CONSOLE_OUTPUT,(long)dma[i]);
				else
					break;
		}
		bdos(RESET_DRIVE,(long)cmdfcb[0]);
	} else
		if(parm[1][0])
		{
			if(i > 0)
				bdos(PRINT_STRING,(BYTE)msg7);
			else
				bdos(PRINT_STRING,(BYTE)msg6);
		}
}

					/************************/						
				/*    rename a file	*/
					/************************/
void ren_cmd()
{
	BYTE    	new_fcb[FCB_LEN];
	REG UWORD	i,j,k,bad_cmd;

	bad_cmd = FALSE;	       /*-------------------------*/	
	if(parm[1][0] == NULL)	       /*prompt user for filenames*/
	{			       /*-------------------------*/	
		bdos(PRINT_STRING,(BYTE)msg3); 
		get_cmd(&parm[3][0],(long)(ARG_LEN-1));
		if(parm[3][0] == NULL)
			return;
		bdos(PRINT_STRING,(BYTE)msg4);  
		get_cmd(&parm[1][0],(long)(ARG_LEN-1));
		parm[2][0] = '=';
	}			/*--------------------------------*/
	 else		        /*check for correct command syntax*/
	 {			/*--------------------------------*/
		i = 0;
		while(parm[1][i] != '=' && parm[1][i]) i++;
		if(parm[1][i] == '=')
		{
			if(!(i > 0 && parm[1][i+1] &&
			     parm[2][0] == NULL))
				bad_cmd = TRUE;
		}
		else
			if(!(parm[2][0] == '='  &&
			     parm[2][1] == NULL &&
			     parm[3][0]))
				bad_cmd = TRUE;
		if(!bad_cmd && parm[1][i] == '=')
		{
			parm[1][i] = NULL;
			i++;
			j = 0;
			while((parm[3][j++] = parm[1][i++]) != NULL);
			parm[2][0] = '=';
		}
	}
	for(j = 1;j < 4;j += 2)
	{
		k = 0;
		while(parm[j][k] != ':' && parm[j][k])
			k++;
		if(k > 1 && parm[j][k] == ':')
			bad_cmd = TRUE;
		for(i = 0;i < sizeof del;i++)
			if(parm[j][0] == del[i])
			{
				echo_cmd(&parm[j][0],BAD);
				return;
			}
	}
	if(!bad_cmd && parm[1][0] && parm[3][0])
	{
		i = fill_fcb(1,new_fcb);
		j = fill_fcb(3,cmdfcb);
		if(i == 0 && j == 0)
		{
			if(new_fcb[0] != cmdfcb[0])
			{
				if(parm[1][1] == ':' && parm[3][1] != ':')
					cmdfcb[0] = new_fcb[0];
				else
				if(parm[1][1] != ':' && parm[3][1] == ':')
					new_fcb[0] = cmdfcb[0];
				else
				bad_cmd = TRUE;
			}
			if(new_fcb[0] < 1 || new_fcb[0] > 16)
				bad_cmd = TRUE;
			if(!(bad_cmd) && bdos(SEARCH_FIRST,(BYTE)new_fcb) != 255)
				bdos(PRINT_STRING,(BYTE)msg5);
			else{
				k = 0;
				for(i = 16;i <= 35;i++)
					cmdfcb[i] = new_fcb[k++];
				if(cmdfcb[0] < 0 || cmdfcb[0] > 15)
					bad_cmd = TRUE;
				if(!(bad_cmd) &&
				bdos(RENAME_FILE,(BYTE)cmdfcb) > 0)
					bdos(PRINT_STRING,(BYTE)msg6);
			    }
		}
		else
		 bdos(PRINT_STRING,(BYTE)msg7);
	}
	if(bad_cmd)
	         bdos(PRINT_STRING,(BYTE)msg8);
}

					/************************/
				/*  erase a file from	*/
					/*  the directory       */
					/************************/
void era_cmd(){
	REG 	UWORD i;
				        /*----------------------*/
	if(parm[1][0] == NULL)		/* prompt for a file	*/
	{				/*----------------------*/
		bdos(PRINT_STRING,(BYTE)msg2);
		get_cmd(&parm[1][0],(long)(ARG_LEN-1));
	}
	if(parm[1][0] == NULL)
		return;
	if(too_many())
		return;
	i = find_colon();
	if(!chk_colon(i))
		return;
	else
		if(parm[1][1] == ':' && parm[1][2] == NULL)
		{
			echo_cmd(&parm[1][0],BAD);
			return;
		}
	i = fill_fcb(1,cmdfcb);		/* fill an fcb	   */
	if(i > 0 && !(submit))		/* no confirmation */
	{				/* if submit file  */
		bdos(PRINT_STRING,(BYTE)msg9);
		parm[2][0] = bdos(CONIN,(long)0);
		parm[2][0] = toupper(parm[2][0]);
		cr_lf();
		if(parm[2][0] != 'N' && parm[2][0] != 'Y')
			return;
	}
	if(parm[2][0] != 'N')
		if(bdos(DELETE_FILE,(BYTE)cmdfcb) > 0)
			bdos(PRINT_STRING,(BYTE)msg6);
}
					/************************/
			/* change user number	*/
					/************************/
UWORD user_cmd(){
	REG UWORD i;

	if(parm[1][0] == NULL)		/* prompt for a number	*/
	{
		bdos(PRINT_STRING,(BYTE)msg10);
		get_cmd(&parm[1][0],(long)(ARG_LEN-1));
	}
	if(parm[1][0] == NULL)
		return(TRUE);
	if(too_many())
		return(TRUE);
	if(parm[1][0] < '0' || parm[1][0] > '9')
		return(FALSE);
	i = (parm[1][0] - '0');
	if(i > 9)
		return(FALSE);
	if(parm[1][1])
		i = ((i * 10) + (parm[1][1] - '0'));
	if(i < 16 && parm[1][2] == NULL)
		bdos(GET_USER_NO,(long)i);
	else
		return(FALSE);
	return(TRUE);
}


					/************************/
					/*			*/
					/*      SEARCH ORDER	*/
					/*	============	*/
					/*			*/
					/* 1. 68K type on the   */
					/*    current user #	*/
					/* 2. BLANK type on     */
					/*    current user #	*/
					/* 3. SUB type on the   */
					/*    current user #	*/
					/* 4. 68K type on	*/
					/*    user 0		*/
					/* 5. BLANK type on the */
					/*    user 0	        */
					/* 6. SUB type on 	*/
					/*    user 0		*/
					/*			*/
					/*----------------------*/
					/* 			*/
					/*   If a filetype is   */
					/*   specified then I   */
					/*   search the current */
					/*   user # then user 0 */
					/*			*/
					/************************/
UWORD cmd_file(UWORD mode)
{
	BYTE done,open,sub_open;
	BYTE found;
	REG UWORD i,n;
	UWORD (*ldrpgm) (void *);
	REG BYTE *top;
	REG struct _filetyps *p;

	dirflag = FALSE;
	load_try = TRUE;
	done = FALSE;
	found = FALSE;
	sub_open = FALSE;
	open = FALSE;

	user = bdos(GET_USER_NO,(long)255);
	cur_disk = bdos(RET_CUR_DISK,(long)0);
	if(mode == SEARCH) i = 0; else i = 1;
	i = fill_fcb(i,cmdfcb);
	if(i > 0)
	{
		bdos(PRINT_STRING,(BYTE)msg7);
		return(FALSE);
	}
	p = load_tbl;
	top = p->typ;
	if(cmdfcb[9] == ' ')
	{
	   while(*(p->typ)) /* clear flags in table */
	   {
		p->user_c = p->user_0 = FALSE;
		p++;
	   }
	   bdos(SELECT_DISK,(long)(cmdfcb[0]-1));
	   cmdfcb[0] = '?'; cmdfcb[12] = NULL;
	   i = bdos(SEARCH_FIRST,(BYTE)cmdfcb);
	   while(i != 255 && !(done))
	   {
	   	i *= 32;
		if(dma[i] == 0 || dma[i] == user)
		{
		   for(n = 9;n <= 11;n++) dma[i+n] &= CMASK;
		   dma[i+12] = NULL;
		   p = load_tbl;
		   while(*(p->typ))
		   {
		      cpy((BYTE*)p->typ,(BYTE*)&cmdfcb[9]);
		      if(strcmp(&cmdfcb[1],&dma[i+1]) == MATCH)
		      {
		         found = TRUE;
			 if(dma[i] == user) p->user_c = TRUE;
			 else		   p->user_0 = TRUE;
			 if(mode == SEARCH &&
			   strcmp(p->typ,top) == MATCH && p->user_c)
				done = TRUE;
		      }
		      p++;
		   }
		}
		i = bdos(SEARCH_NEXT,(BYTE) 0L);
	   }
	   if(!(found))
	   {
		if(mode == SUB_FILE) bdos(PRINT_STRING,(BYTE)msg11);
		dirflag = TRUE; load_try = FALSE;
		bdos(SELECT_DISK,(long)cur_disk); return(FALSE);
	   }
	   if(mode == SEARCH)
		fill_fcb(0,cmdfcb);
	   else
	   	fill_fcb(1, (BYTE *)cmdfcb);
		//fill_fcb(1,cmdfcb);
	   p = load_tbl;
	   while(*(p->typ))
	   {
		if(p->user_c || p->user_0) 
		{
		   if(mode == SEARCH) break;
		   if(strcmp(p->typ,"SUB") == MATCH) break;
		}
		p++;
	   }
	   if(*(p->typ))
	   {
	   	if(!(p->user_c)) bdos(GET_USER_NO,(long)0);
	   	cpy(p->typ,&cmdfcb[9]);
	   }
	}
	bdos(SELECT_DISK,(long)cur_disk);
	while(1)
	{
	   if(bdos(OPEN_FILE,(BYTE)cmdfcb) <= 3)
	   {
		for(n = 9;n <= 11;n++) cmdfcb[n] &= CMASK;
		if(cmdfcb[9] == 'S' && cmdfcb[10] == 'U' && cmdfcb[11] == 'B')
		{
		   sub_open = TRUE;
		   sub_user = bdos(GET_USER_NO,(long)255);
		   if(submit) chain_sub = TRUE;
		   else	      first_sub = TRUE;
		   for(i = 0;i < FCB_LEN;i++)
			subfcb[i] = cmdfcb[i];
		   if(mode == SEARCH) subprompt = FALSE;
		   submit = TRUE;
		   end_of_file = FALSE;
		}
		else if(mode != SUB_FILE)
		   open = TRUE;
		break;
	   }
	   else if(bdos(GET_USER_NO,(long)255) == 0) break;
	        else bdos(GET_USER_NO,(long)0);
	}
	if(open)
	{
	   check_cmd(glb_index);
	   if(!(found))
	   {
		p = load_tbl;
		while(*(p->typ))
		   if(strcmp(p->typ,&cmdfcb[9]) == MATCH) break;
		   else p++;
	   }
	   if(*(p->typ))
		ldrpgm = (void*)p->loader;
	   else
		ldrpgm =(void*) load68k;  /* default */

	   /* ATTEMPT THE PROGRAM LOAD */

	   
	   switch( (*ldrpgm)( (void*)glb_index ) )
	   {
		case 1:	bdos(PRINT_STRING,(BYTE)lderr1); break;
		case 2: bdos(PRINT_STRING,(BYTE)lderr2); break;
		case 3: bdos(PRINT_STRING,(BYTE)lderr3); break;
	     default  : bdos(PRINT_STRING,(BYTE)lderror);
	   }
	}
	if(!(sub_open) && mode == SUB_FILE)
	   bdos(PRINT_STRING,(BYTE)msg11);
	bdos(GET_USER_NO,(long)user);
	dirflag = TRUE;
	load_try = FALSE;
	morecmds = FALSE;
	return((sub_open || open));
}

					/************************/
UWORD sub_read()			/* Read the submit file */
{					/************************/
	if(bdos(READ_SEQ,(BYTE)subfcb))
	{
		end_of_file = TRUE;
		return(FALSE);
	}
	return(TRUE);
}

					/************************/	
		/*    Translate $n to   */ 
					/*  nth argument on the */
					/*  command line	*/
					/************************/
UWORD dollar(UWORD k,UWORD mode,BYTE *com_index)					
{
	REG UWORD n,j,p_index;
	REG BYTE *p1;

	j = sub_index;
	if(k >= CMD_LEN)
	{
		k = 0;
		if(!sub_read())
			return(k);
	}
	if((subdma[k] >= '0') && (subdma[k] <= '9'))
	{
		p_index = (subdma[k] - '0');
		p1 = com_index;
		if(*p1++ == 'S' &&
		   *p1++ == 'U' &&
		   *p1++ == 'B' &&
		   *p1++ == 'M' &&
	  	   *p1++ == 'I' &&
		   *p1++ == 'T' &&
		   *p1   == ' ')
			p_index++;
		p1 = com_index;
		for(n = 1; n <= p_index; n++)
		{
			while(*p1 != ' ' && *p1)
				p1++;
			if(*p1 == ' ')
				p1++;
		}
		while(*p1 != ' ' && *p1 && j < CMD_LEN)
			if(mode == FILL)
				subcom[j++] = *p1++;
			else
				bdos(CONSOLE_OUTPUT,(long)*p1++);
		k++;
	}
	else
	{
		if(mode == FILL)
			subcom[j++] = '$';
		else
			bdos(CONSOLE_OUTPUT,(long)'$');
		if(subdma[k] == '$')
			k++;
	}
	sub_index = j;
	if(k >= CMD_LEN)
	{
		k = 0;
		sub_read();
	}
	return(k);
}
					/************************/
	/* Strip and echo submit*/
					/* file comments	*/
					/************************/					
UWORD comments(UWORD k,BYTE *com_index)	
{
	REG UWORD done;

	done = FALSE;
	prompt();
	do
	{
		while(k < CMD_LEN     &&
		     subdma[k] != EOF && 
		     subdma[k] != Cr)
		{
			if(subdma[k] == '$')
			{
				k++;
				k = dollar(k,NOFILL,(BYTE*)com_index);
			}
			else
				bdos(CONSOLE_OUTPUT,(long)subdma[k++]);
		}
		if(k == CMD_LEN && subdma[k] != EOF && subdma[k] != Cr)
		{
			k = 0;
			if(!sub_read()) done = TRUE;
		}
		else
		{
			if(subdma[k] == Cr)
			{
				k += 2;
				if(k >= CMD_LEN)
				{
					k = 0;
					sub_read();
				}
			}
			else
				end_of_file = TRUE;
			done = TRUE;
		}
	}while(!(done));
	return(k);
}

					/************************/
void translate(com_index)		/* TRANSLATE the subfile*/
					/* and fill sub buffer. */
					/************************/
REG BYTE *com_index;
{
	REG BYTE *p1;
	REG UWORD j,n,k,p_index;

	j = 0;
	k = sub_index;
	while(!(end_of_file) && j < CMD_LEN
		&& subdma[k] != Cr && subdma[k] != EXLIMPT)
	{
		switch(subdma[k])
		{
			case ';': k = comments(k,(BYTE*)com_index);
				  break; 
			case TAB:
			case ' ': if(j > 0)
				  	subcom[j++] = subdma[k++];
	blankout:		  while(k < CMD_LEN && 
				       (subdma[k] == ' ' || subdma[k] == TAB))
						k++;
				  if(k >= CMD_LEN)
				  {
					k = 0;
					if(!sub_read());
					else
						goto blankout;		
				  }
				  break;
			case '$': k++;
				  sub_index = j;
				  k = dollar(k,FILL,(BYTE*)com_index);
				  j = sub_index;
				  break;
			case Lf:  k++;
				  if(k >= CMD_LEN)
				  {
					k = 0;
					sub_read();
			          }			
				  break;
			case EOF: 
				  end_of_file = TRUE;
				  break;
                         default: subcom[j++] = subdma[k++];
				  if(k >= CMD_LEN)
				  {
					k = 0;
					sub_read();
			          }
		}
	}
	/*------------------------------------------------------------------*/
	/*	       TRANSLATION OF A COMMAND IS COMPLETE		    */
	/*	       -Now move sub_index to next command-          	    */
        /*------------------------------------------------------------------*/

	if(subdma[k] == Cr || subdma[k] == EXLIMPT)
	do
	{
		while(k < CMD_LEN && 
		     (subdma[k] == Cr ||
		      subdma[k] == Lf ||
		      subdma[k] == EXLIMPT))
				k++;	
		if(k == CMD_LEN)
		{
			k = 0;
			if(!sub_read())
				break;
		}
		else
		{
			if(subdma[k] == EOF)
				end_of_file = TRUE;
			break;
		}
	}while(TRUE);
	sub_index = k;
}

					/************************/
UWORD submit_cmd(com_index)		/* fill up the subcom   */
					/*      buffer 		*/
					/************************/
/*--------------------------------------------------------------*\
 |								|
 |	Submit_Cmd is a Procedure that returns exactly		|
 |	one command from the submit file.  Submit_Cmd is	|
 |	called only when the end of file marker has not 	|
 |	been read yet.  Upon leaving submit_cmd,the variable    |
 |	sub_index points to the beginning of the next command   |
 |	to translate and execute.  The buffer subdma is used    |
 |      to hold the UN-translated submit file contents.  	|
 |	The buffer subcom holds a translated command.		|
 |	Comments are echoed to the screen by the procedure      |
 |	"comments".  Parameters are substituted in comments     |
 |      as well as command lines.				|
 |								|
\*--------------------------------------------------------------*/

REG BYTE *com_index;
{
	REG UWORD i,cur_user;

	for(i = 0;i <= CMD_LEN;i++)
		subcom[i] = NULL;
	cur_user = bdos(GET_USER_NO,(long)255);
	bdos(GET_USER_NO,(long)sub_user);
	bdos(SET_DMA_ADDR,(BYTE)subdma);
	if(first_sub || chain_sub)
	{
		for(i = 0;i < CMD_LEN;i++)
			subdma[i] = NULL;
		sub_read();
		sub_index = 0;
	}
	if(!(end_of_file))
		translate(com_index);
	for(i = 0;i < CMD_LEN;i++)
		subcom[i] = toupper(subcom[i]);
	bdos(SET_DMA_ADDR,(BYTE)dma);
	bdos(GET_USER_NO,(long)cur_user);
}
					/************************/	
		/*    branch to		*/
		 			/* appropriate routine	*/
					/************************/

void execute_cmd(BYTE *cmd){

	REG UWORD i,flag;

	switch( decode((BYTE*)cmd) )
	{
		case  DIRCMD:	dir_cmd(0);
				break;
		case DIRSCMD:	dir_cmd(1);
				break;
		case TYPECMD:	type_cmd();
				break;
		case  RENCMD:	ren_cmd();
				break;
		case  ERACMD:	era_cmd();
				break;
		case    UCMD:	if(!(user_cmd()))
					bdos(PRINT_STRING,(BYTE)msg12);
				break;
		case CH_DISK:   bdos(SELECT_DISK,(long)(parm[0][0]-'A'));
				break;
		case  SUBCMD:   flag = SUB_FILE;
				if(parm[1][0] == NULL)
				{
					bdos(PRINT_STRING,(BYTE)msg2);
					get_cmd(subdma,(long)(CMD_LEN-1));
					i = 0;
					while(subdma[i] != ' ' &&
					      subdma[i] &&
					      i < ARG_LEN-1 )
						parm[1][i] = subdma[i++];
					parm[1][i] = NULL;
					if(i != 0) subprompt = TRUE;
					else break;
				}
				else
					subprompt = FALSE;
				goto gosub; 
		case    FILE:   flag = SEARCH;
gosub:				if(cmd_file((UWORD)flag))
					break;
				if(flag == SUB_FILE)
					break;
		default	    :	echo_cmd(parm,BAD);
	}
}

void main()
{					 /*---------------------*/
	REG BYTE *com_index;		 /* cmd execution ptr   */ 	
					 /*---------------------*/
	dirflag = TRUE;		         /* init fcb fill flag  */
	bdos(SET_DMA_ADDR,(BYTE)dma);          /* set system dma addr */
					 /*---------------------*/
	if(load_try)
	{
		bdos(SELECT_DISK,(long)cur_disk);
		bdos(GET_USER_NO,(long)user);
		load_try = FALSE;
	}
	if(chainp)			 /*sw Chain?		*/
	{				 /*sw Yes.		*/
	  com_index = chainp + 1;	 /*sw Set-um pointer	*/
	  com_index[((WORD)(*chainp))&0xff] = NULL; /*sw Add null*/
	  chainp = NULL;		 /*sw Clear chain flag	*/
	}				 /*sw *******************/
	else 				 /*sw Submit or normal  */
	{ 				 /*---------------------*/
	  if(morecmds)			 /* if a warmboot 	*/
	  {				 /* occurred & there were*/
					 /* more cmds to do	*/
		if(submit)		 /*---------------------*/
		{
			com_index = subcom;
			submit_cmd(save_sub);
			while(!*com_index)
			{
				if(end_of_file)
				{
				   com_index = user_ptr;
				   submit = FALSE;
				   break;
				}
				else submit_cmd(save_sub);
			}
		}
		else
			com_index = user_ptr;
		morecmds = FALSE;
		if(*com_index)
			echo_cmd(com_index,GOOD);
	  }
	  else
	  {				/*----------------------*/
               prompt();		/* prompt for command   */
	       com_index = usercmd;	/* set execution pointer*/
	       if(autost && autorom)	/* check for COLDBOOT cm*/
	       {			/*			*/
		echo_cmd(usercmd,GOOD); /* echo the command     */
		autorom = FALSE;	/* turn off .bss flag   */
	       }			/*			*/
	       else			/* otherwise.......	*/
		get_cmd(usercmd,(long)(CMD_LEN));/*read a cmd   */
	  }				/************************/
	}				/*sw if(chainp)		*/
					/************************/

/*--------------------------------------------------------------*\
 |								|
 |		       MAIN CCP PARSE LOOP			|
 |		       ===================			|
 |								|
\*--------------------------------------------------------------*/
			

	while(*com_index)
	{				  /*--------------------*/
		glb_index = com_index;	  /* save for use in    */
					  /* check_cmd call	*/
		get_parms(com_index);	  /* parse command line */
		if(parm[0][0] && parm[0][0] != ';')/*-----------*/
		execute_cmd((char *)parm); 	  /* execute command    */
					  /*--------------------*/
		if(!(submit))
		com_index = scan_cmd(com_index);/* inc pointer  */
		else					  
		{
			com_index = subcom;
			if(first_sub || chain_sub)
			{
				if(subprompt)
					copy_cmd(subdma);
				else
					copy_cmd(glb_index);
				if(first_sub)
					user_ptr = scan_cmd(glb_index);
				submit_cmd(save_sub);
				first_sub = chain_sub = FALSE;
			}
			else
				*com_index = NULL;
			while(*com_index == NULL)
			{
				if(end_of_file)
				{
					com_index = user_ptr;
					submit = FALSE;
					break;
				}
				else
					submit_cmd(save_sub);
			}
		}
		if(*com_index)		
			echo_cmd(com_index,GOOD);
	}
}