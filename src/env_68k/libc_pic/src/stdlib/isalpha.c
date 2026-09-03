
char isalpha(char c){
    if( (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A) )return 1;
    return 0;
}
