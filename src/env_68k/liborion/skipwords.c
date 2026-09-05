/* skip over n words in str, and return a pointer to a non-white char */
/* skips of 0 just skip initial whitespace */
/* 1 Jan 88	Dan Christian created */

char *skip_words (str, n)
  char *str;
  int n;
{
    char c;
    int j;

    while (c = *str) {
     if ((c != ' ') && (c != '\t')) break;
     str++;
     }

    for (j = 0; j < n; j++) {
	while (c = *str) { /* skip a word */
	    if ((c == ' ') || (c == '\t')) break;
	    str++;
	}
	while (c = *str) { /* skip trailing whitespace */
	    if ((c != ' ') && (c != '\t')) break;
	    str++;
	}
    }
    return (str);
}
