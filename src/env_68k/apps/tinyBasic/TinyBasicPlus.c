/******************************************************************
*                                                                *
*               Tiny BASIC for the Motorola MC68000              *
*                                                                *
* Derived from Palo Alto Tiny BASIC as published in the May 1976 *
* issue of Dr. Dobb's Journal.  Adapted to the 68000 by:         *
*       Gordon Brandly                                           *
*       12147 - 51 Street                                        *
*       Edmonton AB  T5W 3G8                                     *
*       Canada                                                   *
*       (updated mailing address for 1996)                       *
*                                                                *
* This version is for MEX68KECB Educational Computer Board I/O.  *
*                                                                *
******************************************************************
*    Copyright (C) 1984 by Gordon Brandly. This program may be   *
*    freely distributed for personal use only. All commercial    *
*                      rights are reserved.
* picoVGA comandos
* 16744509₁₀  RUN_CMD
* 164  CLEARSCREEN
******************************************************************/
#include <stdio.h>
#include  <string.h>
//#include <stdlib.h>

#define PICO_VGA_BASE 0x00FF8000
#define RUN_CMD        (*((volatile unsigned char *)(0x00FF8000 + 0x3D)))
#define REG_X_HIGH     (*((volatile unsigned char *)(PICO_VGA_BASE + 0x2D)))
#define REG_X_LOW      (*((volatile unsigned char *)(PICO_VGA_BASE + 0x2F)))
#define REG_Y_HIGH     (*((volatile unsigned char *)(PICO_VGA_BASE + 0x31)))
#define REG_Y_LOW      (*((volatile unsigned char *)(PICO_VGA_BASE + 0x33)))
#define CMD_CLEAR_SCREEN    0xA4
#define CMD_SET_CUR_POS     0xA3
#define CMD_SET_TXT_COLOR   0xA2
#define CMD_GO_HOME         0xA1


// size of our program ram
#define BasicRamSize   64*1024 /* arbitrary - not dependant on libraries */

// memory alignment
// Align memory addess x to an even page
#define ALIGN_UP(x) ((unsigned char*)(((unsigned int)(x + 1) >> 1) << 1))
#define ALIGN_DOWN(x) ((unsigned char*)(((unsigned int)x >> 1) << 1))

#ifndef boolean 
  #define boolean int
  #define true 1
  #define false 0
#endif

//static boolean runAfterLoad = false;
static boolean triggerRun = false;

////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR	'\r'
#define NL	'\n'
#define LF      0x0a
#define TAB	'\t'
#define BELL	'\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x03
#define CTRLH	0x08
#define CTRLS	0x13
#define CTRLX	0x18

typedef short unsigned LINENUM;

#define PROGMEM

//IMPLEMENTACAO WHILE
#define STACK_WHILE_FLAG 0x57  // 'W'

unsigned char exit = 0;

struct stack_while_frame {
    unsigned char frame_type;
    unsigned char *condition_pos;
    unsigned char *txtpos;
    unsigned char *current_line;
};

static unsigned char program[BasicRamSize];
static const char *  sentinel = "HELLO";
static unsigned char *txtpos,*list_line, *tmptxtpos;
static unsigned char expression_error;
static unsigned char *tempsp;
static void set_cursor_position(int x,int y);
static int orion_load(void);
static int file_read(char *filename, char * program, int size, int *bytes_read);
static int file_write(char *filename,char * program, int size, int *bytes_written);
static int get_filename(char *buf, int max_len);

/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
const static unsigned char keywords[]  = {
  'L','I','S','T'+0x80,
  'L','O','A','D'+0x80,
  'N','E','W'+0x80,
  'R','U','N'+0x80,
  'S','A','V','E'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'G','O','T','O','X','Y'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'R','E','M'+0x80,
  'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,
  'P','R','I','N','T'+0x80,
  'P','O','K','E'+0x80,
  'S','T','O','P'+0x80,
  'B','Y','E'+0x80,
  'F','I','L','E','S'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,
  'A','W','R','I','T','E'+0x80,
  'D','W','R','I','T','E'+0x80,
  'D','E','L','A','Y'+0x80,
  'E','N','D'+0x80,
  'R','S','E','E','D'+0x80,
  'C','H','A','I','N'+0x80,
  'W','H','I','L','E'+0x80,
  'W','E','N','D'+0x80,
  'H','E','X'+0x80,
  'H','E','X','P','E','E','K'+0x80,
  'C','L','S'+0X80,
  0
};

// by moving the command list to an enum, we can easily remove sections 
// above and below simultaneously to selectively obliterate functionality.
enum {
  KW_LIST = 0,
  KW_LOAD, KW_NEW, KW_RUN, KW_SAVE,
  KW_NEXT, KW_LET, KW_IF,
  KW_GOTOXY,
  KW_GOTO, KW_GOSUB, KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_INPUT, KW_PRINT,
  KW_POKE,
  KW_STOP, KW_BYE,
  KW_FILES,
  KW_MEM,
  KW_QMARK, KW_QUOTE,
  KW_AWRITE, KW_DWRITE,
  KW_DELAY,
  KW_END,
  KW_RSEED,
  KW_CHAIN,
  KW_WHILE,
  KW_WEND,
  KW_HEX,
  KW_HEXPEEK,
  KW_CLS,
  KW_DEFAULT /* always the final one*/
};

struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

const static unsigned char func_tab[] PROGMEM = {
  'P','E','E','K'+0x80,
  'A','B','S'+0x80,
  'A','R','E','A','D'+0x80,
  'D','R','E','A','D'+0x80,
  'R','N','D'+0x80,
  0
};
#define FUNC_PEEK    0
#define FUNC_ABS     1
#define FUNC_AREAD   2
#define FUNC_DREAD   3
#define FUNC_RND     4
#define FUNC_UNKNOWN 5

const static unsigned char to_tab[] PROGMEM = {
  'T','O'+0x80,
  0
};

const static unsigned char step_tab[] PROGMEM = {
  'S','T','E','P'+0x80,
  0
};

const static unsigned char relop_tab[] PROGMEM = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  0
};

#define RELOP_GE		0
#define RELOP_NE		1
#define RELOP_GT		2
#define RELOP_EQ		3
#define RELOP_LE		4
#define RELOP_LT		5
#define RELOP_NE_BANG		6
#define RELOP_UNKNOWN	7

const static unsigned char highlow_tab[] PROGMEM = { 
  'H','I','G','H'+0x80,
  'H','I'+0x80,
  'L','O','W'+0x80,
  'L','O'+0x80,
  0
};
#define HIGHLOW_HIGH    1
#define HIGHLOW_UNKNOWN 4

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes

static unsigned char *stack_limit;
static unsigned char *program_start;
static unsigned char *program_end;
static unsigned char *stack; // Software stack for things that should go on the CPU stack
static unsigned char *variables_begin;
static unsigned char *current_line;
static unsigned char *sp;
#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static LINENUM linenum;

static const unsigned char okmsg[]            PROGMEM = "OK";
static const unsigned char whatmsg[]          PROGMEM = "Sintaxe error! ";
static const unsigned char howmsg[]           PROGMEM =	"Execution error!";
static const unsigned char sorrymsg[]         PROGMEM = "Sorry!";
static const unsigned char initmsg[]          PROGMEM = "TinyBasic m68000 V1.0 2025 ";
static const unsigned char memorymsg[]        PROGMEM = " bytes free.";
#ifdef ARDUINO
#ifdef ENABLE_EEPROM
static const unsigned char eeprommsg[]        PROGMEM = " EEProm bytes total.";
static const unsigned char eepromamsg[]       PROGMEM = " EEProm bytes available.";
#endif
#endif
static const unsigned char breakmsg[]         PROGMEM = "break!";
static const unsigned char unimplimentedmsg[] PROGMEM = "Unimplemented";
static const unsigned char backspacemsg[]     PROGMEM = "\b \b";
static const unsigned char indentmsg[]        PROGMEM = "    ";
static const unsigned char sderrormsg[]       PROGMEM = "SD card error.";
static const unsigned char sdfilemsg[]        PROGMEM = "SD file error.";
static const unsigned char dirextmsg[]        PROGMEM = "(dir)";
static const unsigned char slashmsg[]         PROGMEM = "/";
static const unsigned char spacemsg[]         PROGMEM = " ";

const unsigned char stackmsg[] = "STACK FULL";
// E também adicione estas para o WHILE:
const unsigned char nowhile_msg[] = "NO WHILE";
const unsigned char nowend_msg[] = "NO WEND";

static int inchar(void);
static void outchar(unsigned char c);
static void line_terminator(void);

//static short int expression(void);
static unsigned char breakcheck(void);
static unsigned char peek(unsigned long a);
static unsigned long expression(void);


#include <stdint.h>

#define NULL 0

static uint32_t rand_seed = 1;

// Implementação da função srand para 68000
void srand(unsigned int seed)
{
    rand_seed = seed;
}

// Implementação da função rand correspondente (para referência)
int rand(void)
{
    // Algoritmo linear congruencial simples
    rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return (int)(rand_seed >> 16) & 0x7FFF;
}

/***************************************************************************/
static void ignore_blanks(void)
{
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}


/***************************************************************************/
static void scantable(const unsigned char *table)
{
  int i = 0;
  table_index = 0;
  while(1)
  {
    // Run out of table entries?
    if( *table  == 0)
      return;

    // Do we match this character?
    if(txtpos[i] ==  *table )
    {
      i++;
      table++;
    }
    else
    {
      // do we match the last character of keywork (with 0x80 added)? If so, return
      if(txtpos[i]+0x80 == *table )
      {
        txtpos += i+1;  // Advance the pointer to following the keyword
        ignore_blanks();
        return;
      }

      // Forward to the end of this keyword
      while((*table  & 0x80) == 0)
        table++;

      // Now move on to the first character of the next word, and reset the position index
      table++;
      table_index++;
      ignore_blanks();
      i = 0;
    }
  }
}

/***************************************************************************/
static void pushb(unsigned char b)
{
  sp--;
  *sp = b;
}

/***************************************************************************/
static unsigned char popb()
{
  unsigned char b;
  b = *sp;
  sp++;
  return b;
}

/***************************************************************************/
void printnum(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

void printUnum(unsigned int num)
{
  int digits = 0;

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return	num;
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}


/***************************************************************************/
void printmsgNoNL(const unsigned char *msg)
{
  while( *msg != 0 ) {
    outchar( *msg++ );
  };
}

/***************************************************************************/
void printmsg(const unsigned char *msg)
{
  printmsgNoNL(msg);
  line_terminator();
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
      case 0x1B:
        exit=1;
        return;
    case NL:
      //break;
    case CR:
      line_terminator();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      printmsg(backspacemsg);
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}

/***************************************************************************/
static unsigned char *findline(void)
{
  unsigned char *line = program_start;
  while(1)
  {
    if(line == program_end)
      return line;

    if(((LINENUM *)line)[0] >= linenum)
      return line;

    // Add the line length onto the current address, to get to the next line;
    line += line[sizeof(LINENUM)];
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  printnum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;

  // Start looking for next line on even page
  if (ALIGN_UP(list_line) != list_line)
    list_line++;

  line_terminator();
}

static unsigned char peek(unsigned long a)
{
    unsigned long *p;
    unsigned char b;
    p = (unsigned long *)a;

    // Pega byte específico (0 = primeiro, 1 = segundo, etc)
    b = ((*p) >> (3 * 8)) & 0xFF;

    //printf("Voce chamou honoravel peek a = [%lu]  p = [%lu] result [%x]\n", a, (unsigned long)p, b);
    return b;
}

/***************************************************************************/
static unsigned long expr4(void)
{
  // fix provided by Jurg Wullschleger wullschleger@gmail.com
  // fixes whitespace and unary operations
  ignore_blanks();

  if( *txtpos == '-' ) {
    txtpos++;
    return -expr4();
  }
  // end fix

  if(*txtpos == '0')
  {
    txtpos++;
    return 0;
  }

  if(*txtpos >= '1' && *txtpos <= '9')
  {
    unsigned long a = 0;
    do 	{
      a = a*10 + *txtpos - '0';
      txtpos++;
    } 
    while(*txtpos >= '0' && *txtpos <= '9');
    return a;
  }

  // Is it a function or variable reference?
  if(txtpos[0] >= 'A' && txtpos[0] <= 'Z')
  {
    unsigned long a;
    // Is it a variable reference (single alpha)
    if(txtpos[1] < 'A' || txtpos[1] > 'Z')
    {
      a = ((short int *)variables_begin)[*txtpos - 'A'];
      txtpos++;
      return a;
    }

    // Is it a function with a single parameter
    scantable(func_tab);
    if(table_index == FUNC_UNKNOWN)
      goto expr4_error;

    unsigned char f = table_index;

    if(*txtpos != '(')
      goto expr4_error;

    txtpos++;

    a = expression();
    if(*txtpos != ')')
      goto expr4_error;
    txtpos++;
    switch(f)
    {
    case FUNC_PEEK:
      return peek(a);
      
    case FUNC_ABS:
      if(a < 0)
        return -a;
      return a;


    case FUNC_RND:
      return( rand() % a );

    }
  }

  if(*txtpos == '(')
  {
    unsigned long a;
    txtpos++;
    a = expression();
    if(*txtpos != ')')
      goto expr4_error;

    txtpos++;
    return a;
  }

expr4_error:
  expression_error = 1;
  return 0;

}


/***************************************************************************/
static unsigned long expr3(void)
{
  unsigned long a,b;

  a = expr4();

  ignore_blanks(); // fix for eg:  100 a = a + 1

  while(1)
  {
    if(*txtpos == '*')
    {
      txtpos++;
      b = expr4();
      a *= b;
    }
    else if(*txtpos == '/')
    {
      txtpos++;
      b = expr4();
      if(b != 0)
        a /= b;
      else
        expression_error = 1;
    }
    else
      return a;
  }
}

/***************************************************************************/
static unsigned long expr2(void)
{

  unsigned long a,b;

  if(*txtpos == '-' || *txtpos == '+')
    a = 0;
  else
    a = expr3();

  while(1)
  {
    if(*txtpos == '-')
    {
      txtpos++;
      b = expr3();
      a -= b;
    }
    else if(*txtpos == '+')
    {
      txtpos++;
      b = expr3();
      a += b;
    }
    else
      return a;
  }
}

/***************************************************************************/
static unsigned long expression(void)
{
  unsigned long a,b;

  a = expr2();

  // Check if we have an error
  if(expression_error)	return a;

  scantable(relop_tab);
  if(table_index == RELOP_UNKNOWN)
    return a;

  switch(table_index)
  {
  case RELOP_GE:
    b = expr2();
    if(a >= b) return 1;
    break;
  case RELOP_NE:
  case RELOP_NE_BANG:
    b = expr2();
    if(a != b) return 1;
    break;
  case RELOP_GT:
    b = expr2();
    if(a > b) return 1;
    break;
  case RELOP_EQ:
    b = expr2();
    if(a == b) return 1;
    break;
  case RELOP_LE:
    b = expr2();
    if(a <= b) return 1;
    break;
  case RELOP_LT:
    b = expr2();
    if(a < b) return 1;
    break;
  }
  return 0;
}

/***************************************************************************/
void loop()
{
  unsigned char *start;
  unsigned char *newEnd;
  unsigned char linelen;
  boolean isDigital;
  boolean alsoWait = false;
  int val;


  program_start = program;
  program_end = program_start;
  sp = program+sizeof(program);  // Needed for printnum

  // Ensure these memory blocks start on even pages
  stack_limit = ALIGN_DOWN(program+sizeof(program)-STACK_SIZE);
  variables_begin = ALIGN_DOWN(stack_limit - 27*VAR_SIZE);

  // memory free
  printnum(variables_begin-program_end);
  printmsg(memorymsg);

warmstart:
  // this signifies that it is running in 'direct' mode.
  current_line = 0;
  sp = program+sizeof(program);
  printmsg(okmsg);

prompt:
  if( triggerRun ){
    triggerRun = false;
    current_line = program_start;
    goto execline;
  }

  getln( '>' );
  if( exit){
    return;
  }
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);


  // Find the end of the freshly entered line
  while(*txtpos != NL)
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;

  // Line starts should always be on 16-bit pages
  if (ALIGN_DOWN(txtpos) != txtpos)
  {
    txtpos--;
    linelen++;
    // As the start of the line has moved, the data should move as well
    unsigned char *tomove;
    tomove = txtpos + 3;
    while (tomove < txtpos + linelen - 1)
    {
      *tomove = *(tomove + 1);
      tomove++;
    }
  }

  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;


  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    }	
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) // If the line has no txt, it was just a delete
    goto prompt;



  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen > 0)
  {	
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;


    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  printmsg(unimplimentedmsg);
  goto prompt;

qhow:	
  printmsg(howmsg);
  goto prompt;

qwhat:	
  printmsgNoNL(whatmsg);
  if(current_line != NULL)
  {
    unsigned char tmp = *txtpos;
    if(*txtpos != NL)
      *txtpos = '^';
    list_line = current_line;
    printline();
    *txtpos = tmp;
  }
  line_terminator();
  goto prompt;

qsorry:	
  printmsg(sorrymsg);
  goto warmstart;

run_next_statement:
  while(*txtpos == ':')
    txtpos++;
  ignore_blanks();
  if(*txtpos == NL)
    goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
  if(breakcheck())
  {
    printmsg(breakmsg);
    goto warmstart;
  }

  scantable(keywords);

  switch(table_index)
  {
  case KW_DELAY:
      goto unimplemented;
//  case KW_FILES:
//    goto files;
  case KW_LIST:         //  ✔️
    goto list;
//  case KW_CHAIN:
//    goto chain;
  case KW_LOAD:
    goto load;
  case KW_MEM:         //  ✔️
    goto mem;
  case KW_NEW:         //  ✔️
    if(txtpos[0] != NL)
      goto qwhat;
    program_end = program_start;
    goto prompt;
  case KW_RUN:         //  ✔️
    current_line = program_start;
    goto execline;
  case KW_SAVE:
    goto save;
  case KW_NEXT:         //  ✔️
    goto next;
  case KW_LET:         //  ✔️
    goto assignment;
  case KW_IF:         //  ✔️
    short int val;
    expression_error = 0;
    val = expression();
    if(expression_error || *txtpos == NL)
      goto qhow;
    if(val != 0)
      goto interperateAtTxtpos;
    goto execnextline;

  case KW_GOTO:         //  ✔️
    expression_error = 0;
    linenum = expression();
    if(expression_error || *txtpos != NL)
      goto qhow;
    current_line = findline();
    goto execline;

  case KW_GOSUB:         //  ✔️
    goto gosub;
  case KW_RETURN:         //  ✔️
    goto gosub_return; 
  case KW_REM:
  case KW_QUOTE:
    goto execnextline;	// Ignore line completely
  case KW_WHILE:         //  ✔️
    goto whileloop;
  case KW_WEND:         //  ✔️
    goto wend;
  case KW_FOR:         //  ✔️
    goto forloop;
  case KW_INPUT:
    goto input; 
  case KW_PRINT:         //  ✔️
  case KW_QMARK:
    goto print;
  case KW_POKE:         //  ✔️
    goto poke;
  case KW_END:
  case KW_STOP:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    if(txtpos[0] != NL)
      goto qwhat;
    current_line = program_end;
    goto execline;
  case KW_BYE:         //  ✔️
    // Leave the basic interperater
    return;

//  case KW_AWRITE:  // AWRITE <pin>, HIGH|LOW
//    isDigital = false;
//    goto awrite;
//  case KW_DWRITE:  // DWRITE <pin>, HIGH|LOW
//    isDigital = true;
//    goto dwrite;
  case KW_RSEED:
    goto rseed;
  case KW_HEX:           //  ✔️
    {
      expression_error = 0;
      unsigned long value = expression();
      char hex_str[9];
      if( 0xFF > value )
        printf("%02X", value);
      else if ( 0xFFFF > value )
        printf("%4X", value);
      else
        printf("%8X", value);
      //push_string(hex_str);
    }
    break;
    // Implementação:
  case KW_HEXPEEK:
    {
        ignore_blanks();
        if(*txtpos != '(') goto qwhat;
        txtpos++;
        expression_error = 0;
        unsigned long addr = expression();
        if(expression_error) goto qwhat;

        ignore_blanks();
        if(*txtpos != ')') goto qwhat;
        txtpos++;

        unsigned char val = peek(addr);
        printf("%02X\n", val);
    }
    break;
  case KW_GOTOXY:
    goto gotoxy;
  case KW_CLS:
    goto cls;
  case KW_DEFAULT:
    goto assignment;
  default:
    break;
  }

execnextline:
  if(current_line == NULL)		// Processing direct commands?
    goto prompt;
  current_line +=	 current_line[sizeof(LINENUM)];

execline:
  if(current_line == program_end) // Out of lines to run
    goto warmstart;
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;


cls:
  RUN_CMD = 0xA4;
  goto run_next_statement;

gotoxy:
    expression_error = 0;
    int x = expression(); /* Avalia a 1ª expressão (X) */
    if(expression_error) goto qwhat;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    // Now get the data to assign
    expression_error = 0;
    int y = expression(); /* Avalia a 2ª expressão (Y) */
    if(expression_error) goto qwhat;

    /* Aqui você aplica no seu hardware/display */
    set_cursor_position(x, y);
    goto run_next_statement;


input:
  {
    unsigned char var;
    int value;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
inputagain:
    tmptxtpos = txtpos;
    getln( '?' );
    toUppercaseBuffer();
    txtpos = program_end+sizeof(unsigned short);
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto inputagain;
    ((short int *)variables_begin)[var-'A'] = value;
    txtpos = tmptxtpos;

    goto run_next_statement;
  }


short find_wend(void) {
    unsigned char *saved_txtpos = txtpos;      // SALVA estado global
    unsigned char *saved_line = current_line;
    unsigned char *search_pos = txtpos;
    int while_count = 1;
    int safety = 0;

    while(while_count > 0 && safety++ < 1000) {  // Prevenção loop infinito
        // Avança até fim de linha ou do programa
        while(*search_pos != NL && *search_pos != '\0') {
            // Verifica se é WHILE
            if(strncmp(search_pos, "WHILE", 5) == 0) {
                while_count++;
                search_pos += 4;  // Avança "WHILE"
            }
            // Verifica se é WEND
            else if(strncmp(search_pos, "WEND", 4) == 0) {
                while_count--;
                if(while_count == 0) {
                    txtpos = saved_txtpos;      // RESTAURA estado global
                    current_line = saved_line;
                    return 1;                   // WEND encontrado!
                }
                search_pos += 3;  // Avança "WEND"
            }
            search_pos++;
        }

        // Fim de linha, vai para próxima
        if(*search_pos == NL) {
            search_pos++;  // Pula o NL
        }
        else if(*search_pos == '\0') {
            break;  // Fim do programa
        }
    }

    txtpos = saved_txtpos;      // RESTAURA estado global
    current_line = saved_line;
    return 0;  // WEND não encontrado
}

whileloop:
{
    // Salva a posição da condição
    unsigned char *condition_start = txtpos;

    // Avalia a condição
    expression_error = 0;
    short condition = expression();
    if(expression_error)
        goto qwhat;

    // Verifica sintaxe
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
        goto qwhat;

    // Verifica espaço na pilha
    if(sp - sizeof(struct stack_while_frame) < stack_limit) {
        printmsg(stackmsg);
        goto run_next_statement;
    }
    // DEBUG: Verifique se esta condição está bloqueando
    //printf("Espaço na pilha? sp=%lX, limit=%lX, diff=%ld\n",    (unsigned long)sp, (unsigned long)stack_limit, sp - stack_limit);

    if(sp - sizeof(struct stack_while_frame) < stack_limit) {
        //printf("PILHA CHEIA! Não pode criar frame.\n");
        printmsg(stackmsg);
        goto run_next_statement;
    }
    // Cria stack frame
        // DEBUG: Mostra sp antes e depois

    sp -= sizeof(struct stack_while_frame);

    struct stack_while_frame *w = (struct stack_while_frame *)sp;
    w->frame_type = STACK_WHILE_FLAG;
    w->condition_pos = condition_start;
    w->txtpos = txtpos;
    w->current_line = current_line;

    // Se condição falsa, pula para o WEND
    if(condition == 0) {
        if(!find_wend()) {
            printmsg(nowend_msg);
            goto run_next_statement;
        }
    }
    // DEBUG
    //printf("WHILE: cond=%d, sp=%lX, frame=%lX\n",   condition, (unsigned long)sp, (unsigned long)w);

    // Se condição falsa, pula para o WEND
    if(condition == 0) {
        //printf("WHILE FALSE - buscando WEND\n");
        if(!find_wend()) {
            printmsg(nowend_msg);
            goto run_next_statement;
        }
    }
    //else {
    //    printf("WHILE TRUE - entrando no loop\n");
    //}
}
goto run_next_statement;


wend:
{

    struct stack_while_frame *w = NULL;
    unsigned char *stack_ptr = sp;
    int count = 0;


    // CORREÇÃO: Inverta a condição!
    while(stack_ptr >= stack_limit) {  // ← AGORA CORRETO!
        //printf("Buscando em %lX: valor=%02X\n", (unsigned long)stack_ptr, *stack_ptr);

        if(*stack_ptr == STACK_WHILE_FLAG) {
            w = (struct stack_while_frame *)stack_ptr;
            //printf("FRAME ENCONTRADO em %lX!\n", (unsigned long)w);
            break;
        }
        stack_ptr += sizeof(struct stack_while_frame);
        count++;

        if(count > 10) break;
    }

// Salva posição atual para debug
unsigned char *debug_before = txtpos;

// Volta para reavaliar a condição
txtpos = w->condition_pos;

// Mostra O QUE será avaliado
/*
unsigned char *temp = txtpos;
while(*temp != NL && *temp != ':') {
    putchar(*temp);
    temp++;
}
*/
// Reavalia a condição
expression_error = 0;
short condition = expression();



// Restaura para continuar (se necessário)
txtpos = debug_before;

    if(w == NULL) {
  //      printf("WEND: Nenhum frame após %d tentativas\n", count);
        printmsg(nowhile_msg);
        goto run_next_statement;
    }


    while(stack_ptr < stack_limit) {
        if(*stack_ptr == STACK_WHILE_FLAG) {
            w = (struct stack_while_frame *)stack_ptr;
            break;
        }
        stack_ptr++;  // Ajuste conforme sua estrutura de stack
    }

    if(w == NULL) {
        printmsg(nowhile_msg);
        goto run_next_statement;
    }

    // Reavalia a condição
    txtpos = w->condition_pos;
    expression_error = 0;
     condition = expression();

    if(condition != 0) {
        // Volta para o loop
        txtpos = w->txtpos;
        current_line = w->current_line;
    } else {
        // Sai do loop
        sp += sizeof(struct stack_while_frame);
    }

}
goto run_next_statement;

forloop:
  {
    unsigned char var;
    short int initial, step, terminal;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qwhat;
    var = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial = expression();
    if(expression_error)
      goto qwhat;

    scantable(to_tab);
    if(table_index != 0)
      goto qwhat;

    terminal = expression();
    if(expression_error)
      goto qwhat;

    scantable(step_tab);
    if(table_index == 0)
    {
      step = expression();
      if(expression_error)
        goto qwhat;
    }
    else
      step = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;


    if(!expression_error && *txtpos == NL)
    {
      struct stack_for_frame *f;
      if(sp + sizeof(struct stack_for_frame) < stack_limit)
        goto qsorry;

      sp -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)sp;
      ((short int *)variables_begin)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  goto qhow;

gosub:
  expression_error = 0;
  linenum = expression();
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(sp + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    sp -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)sp;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_blanks();
  if(*txtpos != ':' && *txtpos != NL)
    goto qwhat;

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = sp;
  while(tempsp < program+sizeof(program)-1)
  {
    switch(tempsp[0])
    {
    case STACK_GOSUB_FLAG:
      if(table_index == KW_RETURN)
      {
        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
        current_line	= f->current_line;
        txtpos			= f->txtpos;
        sp += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
      // Flag, Var, Final, Step
      if(table_index == KW_NEXT)
      {
        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
        // Is the the variable we are looking for?
        if(txtpos[-1] == f->for_var)
        {
          short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A'; 
          *varaddr = *varaddr + f->step;
          // Use a different test depending on the sign of the step increment
          if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
          {
            // We have to loop so don't pop the stack
            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
          // We've run to the end of the loop. drop out of the loop, popping the stack
          sp = tempsp + sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:
      //printf("Stack is stuffed!\n");
      goto warmstart;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;

    ignore_blanks();

    if (*txtpos != '=')
      goto qwhat;
    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
    *var = value;
  }
  goto run_next_statement;
poke:
  {
    short int data;
    unsigned long value;
    unsigned char *address;

    // Work out where to put it
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;
    address = (unsigned char *)value;

    // check for a comma
    ignore_blanks();
    if (*txtpos != ',')
      goto qwhat;
    txtpos++;
    ignore_blanks();

    // Now get the data to assign
    expression_error = 0;
    data = expression();
    if(expression_error)
      goto qwhat;

    printf("Poke %x data %i\n",address, (unsigned char)data);
    *address=(unsigned char)data;

    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':')
      goto qwhat;
  }
  goto run_next_statement;

list:
  linenum = testnum(); // Retuns 0 if no line found.

  // Should be EOL
  if(txtpos[0] != NL)
    goto qwhat;

  // Find the line
  list_line = findline();
  while(list_line != program_end)
    printline();
  goto warmstart;

print:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    line_terminator();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_blanks();
    if(print_quoted_string())
    {
      ;
    }
    else if(*txtpos == '"' || *txtpos == '\'')
      goto qwhat;
    else
    {
      short int e;
      expression_error = 0;
      e = expression();
      if(expression_error)
        goto qwhat;
      printnum(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++;	// Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
    {
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':')
    {
      line_terminator();	// The end of the print statement
      break;
    }
    else
      goto qwhat;	
  }
  goto run_next_statement;

mem:
  // memory free
  printnum(variables_begin-program_end);
  printmsg(memorymsg);
  goto run_next_statement;


  /*************************************************/

pinmode: // PINMODE <pin>, I/O
awrite: // AWRITE <pin>,val
dwrite:
  goto unimplemented;

  /*************************************************/
//files:
  // display a listing of files on the device.
  // version 1: no support for subdirectories
//   goto run_next_statement;



//chain:
//  runAfterLoad = true;
//    goto run_next_statement;

load:
{
  int bytes_read;
  program_end = program_start;
  //memset(program, 0, BasicRamSize);

  char filename[32];

  expression_error = 0;
  if (!get_filename(filename, sizeof(filename)))
      goto qwhat;

  printf("Loading file: %s\n", filename);

  file_read(filename, program, BasicRamSize - 1, &bytes_read);

  program_end = program_start + bytes_read;

  goto run_next_statement;
}


save:
{
  int bytes_written;
  expression_error = 0;
  char filename[32];

  if (!get_filename(filename, sizeof(filename)))
      goto qwhat;

  printf("Saving file: %s\n", filename);

  /* Se a sua versão usa um ponteiro de fim (ex: program_end): */
  size_t tamanho = (size_t)(program_end - program_start);

  file_write(filename, program, tamanho, &bytes_written);

  goto run_next_statement;
}

rseed:
  {
    short int value;

    //Get the pin number
    expression_error = 0;
    value = expression();
    if(expression_error)
      goto qwhat;

    goto run_next_statement;
  }

}

// Copia o nome do arquivo da linha de comando para o buffer
static int get_filename(char *buf, int max_len)
{
    ignore_blanks();

    char delim = *txtpos;
    int has_quotes = (delim == '"' || delim == '\'');

    if (has_quotes)
        txtpos++; // Pula a aspa inicial

    int i = 0;
    // Se tinha aspas, lê até a aspa final. Se não tinha, lê até espaço, ':' ou NL.
    while (*txtpos != NL && *txtpos != '\0' && *txtpos != ':')
    {
        if (has_quotes && *txtpos == delim) break;
        if (!has_quotes && *txtpos == ' ') break;

        if (i < max_len - 1) {
            buf[i++] = *txtpos;
        }
        txtpos++;
    }

    if (has_quotes && *txtpos == delim)
        txtpos++; // Pula a aspa final

    buf[i] = '\0'; // Finaliza a string em C
    return (i > 0); // Retorna 1 se leu algum caractere
}

static int file_read(char *filename, char * program, int size, int *bytes_read) {
    int result;

    asm volatile (
        "move.l #1, %%d1\n\t"   /* D1: Syscall Read (1) */
        "move.l %1, %%a0\n\t"   /* A0: filename */
        "move.l %2, %%a1\n\t"   /* A1: program (buffer) */
        "move.l %3, %%d0\n\t"   /* D0: size */
        "move.l %4, %%a2\n\t"   /* A2: bytes_read */
        "trap #12\n\t"           /* Executa a Syscall */
        "move.l %%d0, %0\n\t"   /* D0: Retorno (FRESULT) -> result */
        : "=r" (result)
        : "g" (filename), "g" (program), "g" (size), "g" (bytes_read)
        : "d0", "d1", "a0", "a1", "a2", "cc", "memory"
    );

    return result & 0xff;
}

static int file_write(char *filename,char * program, int size, int*bytes_written){
    int result;
    asm volatile (
        "move.l #2, %%d1\n\t"   /* D1: Syscall Read (1) */
        "move.l %1, %%a0\n\t"   /* A0: filename */
        "move.l %2, %%a1\n\t"   /* A1: program (buffer) */
        "move.l %3, %%d0\n\t"   /* D0: size */
        "move.l %4, %%a2\n\t"   /* A2: bytes_read */
        "trap #12\n\t"           /* Executa a Syscall */
        "move.l %%d0, %0\n\t"   /* D0: Retorno (FRESULT) -> result */
        : "=r" (result)
        : "g" (filename), "g" (program), "g" (size), "g" (bytes_written)
        : "d0", "d1", "a0", "a1", "a2", "cc", "memory"
    );
    return result & 0xff ;
}
static void set_cursor_position(int x, int y) {
    unsigned char xh, xl, yh, yl;

    xh = (x >> 8) & 0xFF;  /* Bitwise Shift Right (>>) */
    xl = x & 0xFF;
    yh = (y >> 8) & 0xFF;  /* Bitwise Shift Right (>>) */
    yl = y & 0xFF;
    REG_X_HIGH = xh;
    REG_X_LOW  = xl;
    REG_Y_HIGH = yh;
    REG_Y_LOW  = yl;
    RUN_CMD=CMD_SET_CUR_POS;
}
// returns 1 if the character is valid in a filename
static int isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; // number
  if( c >= 'A' && c <= 'Z' ) return 1; // LETTER
  if( c >= 'a' && c <= 'z' ) return 1; // letter (for completeness)
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  // Window~1.txt

  return 0;
}

unsigned char * filenameWord(void)
{
  // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
  unsigned char * ret = txtpos;
  expression_error = 0;

  // make sure there are no quotes or spaces, search for valid characters
  //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    expression_error = 1;
    return ret;
  }

  // now, find the next nonfnchar
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

  // set the error code if we've got no string
  if( *ret == '\0' ) {
    expression_error = 1;
  }

  return ret;
}

/***************************************************************************/
static void line_terminator(void)
{
  outchar(NL);
  //outchar(CR);
}


/***********************************************************/
/*
static unsigned char breakcheck(void)
{

#ifdef __CONIO__
  if(kbhit())
    return getch() == CTRLC;
  else
    return getchar();
#endif
    return 0;

}
*/
static unsigned char breakcheck(void)
{
  int got;
    got = getchar();
    if ( got < 255) {
        if ( got == CTRLC) {
            return 1;
        }
    }
    return 0;
}
/***********************************************************/
static int inchar()
{
  // otherwise. desktop!
  int got = getchar();
  while(got > 255){
    got = getchar();
    //printf("%08x\n",got);
  }
  // translation for desktop systems
  if( got == LF ) got = CR;

  return got;
}


/***********************************************************/
static void outchar1(unsigned char c)
{
  putchar(c);

}

