/* burte test program */
#include <stdio.h>

static int printed;

int print_out(), start_up();

main()
{
    int i;

    puts ("1> local_main: entered.\n");
    i = create (start_up, 9, 0);
    start (i);
    puts ("2>local_main: start_up created.\n");
    wait (i);
    puts ("8> local_main: bye  bye\n");
};

start_up()
{
    short i;
    int pr;

    printed = TRUE;
    puts ("3>start_up:  Next create...\n");

    pr = create (print_out, 9, 0);
    start (pr);

    puts ("4>start_up:  print_out created.\n");

    post (&printed, FALSE);

    pend (&printed, FALSE, P_SET);

    puts ("6>start_up:  Done printing\n");
    exit ();
}

print_out() {
    short i;

    pend (&printed, TRUE, 0);
    puts
      ("5>print_out:  This the first print that you will receive.\007\n");
  
    fputsx (0x12345678, 1, stdout); fputcn (stdout);
    fputsx (0x12345678, 2, stdout); fputcn (stdout);
    fputsx (0x12345678, 3, stdout); fputcn (stdout);
    fputsx (0x12345678, 5, stdout); fputcn (stdout);
    fputsx (0x12345678, 7, stdout); fputcn (stdout);
    fputsx (0x12345678, 8, stdout); fputcn (stdout);
    fputx (0x12345678, stdout); fputcn (stdout);

    post (&printed, TRUE);

    puts ("\n7>print_out:  Back to me again\n");
}
