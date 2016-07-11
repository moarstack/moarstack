#define HASH_CONSTANT 4096
#define HASH_EMPTY -1
#define HASH_DELETED -2

void Init_Hash(int H[]);

int Hash(int data);

int Rehash(int pos);

int Search_Hash(int H[], int data);

int Add_Hash(int H[], int data);

int Delete_Hash(int H[], int data);
