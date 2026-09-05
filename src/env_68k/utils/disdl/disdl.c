/*----------------------------------------------------------------------------
 *
 * DESCRIPTION
 *	disdl  - disassemble a .dl file produced by the 68000 C compiler.
 *
 * USAGE
 *	disdl [-acd] <file>
 *
 * PROCEDURE
 *	* Get the filename from the command line. Open the .dl file.
 *	* Get the formatting options from the command line. "ca" is default.
 *	* Read the .dl file into a buffer, converting it into binary form.
 *	* Call dis68() to disassemble the binary code and print the results.
 *	
 * HISTORY
 * 14-Sept-87 Dan Christian (dac) at Carnegie-Mellon U.
 *      Added dynamic memory allocation and improved error messages.
 *      Extra output verbosity was removed.
 *
 * 13-Aug-87  Dan Christian (dac) at Carnegie-Mellon University
 *     fixed bug in print_machine code in dis68.c and increased the size
 *     of the buffer for the binary code.
 *
 * 11-Jun-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Added interpretation of S2 records and removed offset parameter.
 *	This should now implement the normal VERSABUG download format.
 *
 * 10-Jun-82  John Schlag (jfs) at Carnegie-Mellon University
 *	Rewrote from Fortran/C. Changed the command line syntax to accept
 *	the input parameters.
 *
 *----------------------------------------------------------------------------
 */

#include <stdio.h>

main(argc, argv, envp)
  int argc;
  char *argv[], *envp[];
{
    int	i,
    addr,			/* starting address of the code	*/
    nwords;			/* number of binary code words	*/
    char	*p,
    options[20],		/* output format options	*/
    myname[40],		/* name of this program		*/
    dlfile[40];		/* name of the .dl file		*/
    unsigned short *code;	/* buffer for the binary code	*/
    FILE	*fp;			/* file pointer to .dl file	*/
    
    /* Copy the program name to global storage. */
    
    sprintf(myname, argv[0]);
    
    /* Check the argument count and print usage if necessary. */
    
    if (argc == 1 || argc > 3)
     {
	 fprintf(stderr, "Usage: %s [-acd] <.dl file>\n", myname);
	 exit(-1);
     }
    
    /* Get the formatting options. */
    
    if (argv[1][0] == '-')
     {
	 sprintf(options, argv[1]+1);
	 p = options;
	 for ( ; *p ; p++)
	  if (*p == 'a' || *p == 'c' || *p == 'd')
	   continue;
	  else	
	   {
	       fprintf(stderr, "%s: Bad formatting option: %c\n",
		       myname,*p);
	       exit(-1);
	   }
     }
    else
     sprintf(options, "ac");
    
    /* Open the .dl file for input. */
    
    sprintf(dlfile, (argc == 2) ? argv[1] : argv[2]);
    if ( (fp = fopen(dlfile,"r")) == NULL )
     {
	 fprintf(stderr, "%s: Can't open input file: %s\n",
		 myname, dlfile);
	 exit(-1);
     }
    
    /* Load the download file. */
    
    addr = rd_dlfile(fp, &code, &nwords); /* code array is allocated */
				/* in rd_dlfile. */
    fprintf (stderr, "Read: %d words\n", nwords);
    if (addr < 0)
     {
	 fprintf(stderr,
		 "%s: No starting address in .dl file.\n", myname);
	 exit(-1);
     }
    
    /* Call dis68() to disassemble the binary code. */
    
    dis68 (code, nwords, addr, options, stdout);
}
