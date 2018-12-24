char* charToPunch(char c, char* word) {
     
     int i;
     for (i = 7; i >= 0; i--) {
         if (c & (1<<i)) *(word++) = '*';
         else *(word++) = '-';
     }
     
     *(word++) = '\n';
    
     /* return the 1st available space in the string */
     return word;
}


/*return the first character of a string translated into punchcard language*/
/* "1" -> "*" , "0" -> "-" ES : 10110010 -> *-**--*-                       */
char* cStrToPunch(char* str, char* buf) {
    
    char* tmp = buf;
    
    if (*str == ' ') 
        *(tmp++) = '\n';
    else if (str && *str != '\0') {
        tmp = charToPunch(*str,tmp);
    }

    *(tmp) = '\0';
    
    return buf;

}
