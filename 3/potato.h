#include <time.h>
#include <stdlib.h>


#define DEFAULT_PATH_LENGTH 128

#define GAME_OVER   100
#define POTATO      101
#define ACK         111

//#define RANDOM_NUMBER(limit) (srand(clock()), (rand() % limit))
#define RANDOM_NUMBER(limit) (rand() % limit)
// #define POTATO_BASE_SIZE_BYTES sizeof(unsigned char) + (2 * sizeof(unsigned int))
// Can't use sizeof in preprocessing

unsigned long POTATO_BASE_SIZE_BYTES;
unsigned long POTATO_SIZE_BYTES;
unsigned long path_size_bytes;

typedef struct Potato {
    unsigned char flag;
    unsigned int hops; 
    unsigned int current_hop;
    unsigned int* path;
} Potato;


// #define POTATO_SIZE_BYTES (POTATO_BASE_SIZE + (sizeof(unsigned int) * hops))
//
void send_potato(Connection* conn);
void receive_potato(Connection* conn);
