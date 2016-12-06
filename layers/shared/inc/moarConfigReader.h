//
// Created by svalov on 12/6/16.
//

#ifndef MOARSTACK_MOARCONFIGREADER_H
#define MOARSTACK_MOARCONFIGREADER_H

#include <hashTable.h>

#define CONFIG_SIZE 100

int configPrepare(hashTable_T* config);
int configRead(hashTable_T* config, char* fileName);

#endif //MOARSTACK_MOARCONFIGREADER_H
