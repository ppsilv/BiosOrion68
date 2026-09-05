int atoi(s)
  char *s;
{
    int  n=0;
    short f=0;
    
    while (*s) {
	if ((*s != ' ') && (*s != '\t')) break;
	s++;
    }

    if(*s == '-') {
	f++;
	s++;
    }

    while(*s && (*s >= '0') && (*s <= '9'))
      n = n*10 + *s++ - '0';
    
    if(f)
      n = -n;
    return(n);
}
