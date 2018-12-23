char* charToPunch(char c, char* word) {
     
     int i;
     for (i = 7; i >= 0; i--) {
         if (c & (1<<i)) *word = '*';
         else *word = '-';
         word++;
     }
     
     *(word++) = '\n';
    
     /* return the 1st available space in the string */
     return word;
}


/*create an unique string translating to PunchCard language*/
/* "1" -> "*" , "0" -> "-" ES : 1011 -> *-** */

char* strToPunch(char* str, char* buf) {
    
    char* tmp = buf;

    if (str && *str != '\0') {
        tmp = charToPunch(*str,tmp);
        str++;
    }

    *(tmp) = '\0';
    
    return buf;

}
