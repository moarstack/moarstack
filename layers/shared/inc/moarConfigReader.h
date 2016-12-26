//
// Created by svalov on 12/6/16.
//

#ifndef MOARSTACK_MOARCONFIGREADER_H
#define MOARSTACK_MOARCONFIGREADER_H

#include <hashTable.h>

#define CONFIG_SIZE 100
#define LINE_SIZE 255

#define STRVAL(str) (*((char**)str))

__BEGIN_DECLS

extern int configInit(hashTable_T* config);
extern int configFree(hashTable_T* config);
extern int configRead(hashTable_T* config, char* fileName);
extern int configMerge(hashTable_T* dest, hashTable_T* source);

__END_DECLS

#endif //MOARSTACK_MOARCONFIGREADER_H
