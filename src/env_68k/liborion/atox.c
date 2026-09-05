int atox (s)
  char *s;
/*
 * Decode hexadecimal characters into an integer value.
 * Folds case for A-F.
 * Assumes string is terminated by a non-hex character.
 * Decodes at most 8 characters.  Scans forward over leading white space.
 * Ignores leading 0x or 0X.
 */
{
    short i;
    long result;

    result = 0;

    while (*s) {
	if ((*s != ' ') && (*s != '\t')) break;
	s++;
    }

    if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X'))
	s += 2;

    for (i = 0; i < 8; i++) {
	if ((*s >= '0') && (*s <= '9')) {
	    result <<= 4;
	    result += *s++ - '0';
	}
	else if ((*s >= 'A') && (*s <= 'F')) {
	    result <<= 4;
	    result += *s++ - 'A' + 10;
	}
	else if ((*s >= 'a') && (*s <= 'f')) {
	    result <<= 4;
	    result += *s++ - 'a' + 10;
	}
	else
	    break;
    }
    return (result);
}
