#include "hash.h"

void Init_Hash(int H[]){
        for(int i = 0; i < HASH_CONSTANT; i++)
                H[i] = HASH_EMPTY;
}

int Hash(int data){
        int key = data;
        key = (key + ~(key << 16)) % HASH_CONSTANT;
        key = (key ^ (key >> 5)) % HASH_CONSTANT;
        key = (key + (key << 3)) % HASH_CONSTANT;
        key = (key ^ (key >> 13)) % HASH_CONSTANT;
        key = (key + ~(key << 9)) % HASH_CONSTANT;
        key = (key ^ (key >> 17)) % HASH_CONSTANT;
        return key;
}

int Rehash(int pos){
        return (pos + 1) % HASH_CONSTANT;
}

int Search_Hash(int H[], int data){
        int key = Hash(data);
        while(1){
                if (H[key] == HASH_EMPTY)
                        return -1;
                if (H[key] == data)
                        return key;
                key = Rehash(key);
        }
}

int Add_Hash(int H[], int data){
        int key;
        if (Search_Hash(H, data) != -1)
                return -1;
        key = Hash(data);
        while(1){
                if (H[key] == HASH_EMPTY || H[key] == HASH_DELETED){
                        H[key] = data;
                        return key;
                }
                key = Rehash(key);
        }
}

int Delete_Hash(int H[], int data){
        int key = Hash(data);
        while(1){
                if (H[key] == HASH_EMPTY)
                        return -1;
                if (H[key] == data){
                        H[key] = HASH_DELETED;
                        return key;
                }
                key = Rehash(key);
        }
}

