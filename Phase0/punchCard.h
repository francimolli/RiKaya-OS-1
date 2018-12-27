char* charToPunch (char c, char* word);
/* Questa funzione riceve in input un carattere da tradurre e un arry in cui scrivere il risultato della traduzione. 
 * Quando il carattere viene preso come operando dall'operatore & per far  l' AND bit a bit, esso viene convertito in binario in modo implicito dal compilatore, essendo un intero. Dal momento che un carattere occupa al massimo 8 bit, per capire quale sequenza di '-' e '*' scrivere, l' & bit a bit viene fatto con i valori 10000000, 01000000, ..., 00000001. In tal modo si riesce ad identificare quale posizione è un 1, e quindi un '*', e quale è uno 0, ovvero '-' */

char* cStrToPunch (char* str, char* buf);
/* cStrToPunch passa a charToPunch i caratteri da tradurre. Prima del passaggio dei caratteri viene fatto un controllo: se il carattere passato corrisponde ad uno spazio vuoto allora non avviene nessuna traduzione e nell' array contenente la traduzione è messo uno '\n' in modo da andare a capo(scelta fatta per migliorare la leggibilità della traduzione; nell' output ogni carattere occuperà una riga). */
