#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>

#define IS_NULL(x) 			(x == NULL ? true : false)
#define IS_NOT_NULL(x) 		(x == NULL ? false : true)
#define HOSTENT_SIZE 		sizeof(struct hostent)
#define SEND_SOCK 			0
#define RECV_SOCK 			1
// #define LISTEN_PORT 		50000
// #define MASTER_PORT 		60000

#define ERR_ARGS    1
#define ERR_SEND    2
#define ERR_NEIGH   3


#define HOSTNAME_SIZE 64

typedef int SockDesc;
typedef int Port;
typedef struct hostent HostEntity;

typedef struct Connection {
	bool listens;
    bool set_host;
    bool in_player;
	char* hostname;
	HostEntity* host_entity;
	unsigned int port;
	SockDesc socket;
	int address_length;
	struct sockaddr_in socket_addr_info;
} Connection;

void set_host_info(Connection* net_info, char* h);
void setup_socket(Connection* net_info, int port);
void listen_for_incoming_connection(Connection* listener, Connection* new_con);
void accept_incoming_connection(Connection* listener, Connection* new_con);
void destroy_connection(Connection* n);
void error_out(char* err_str, int err_no);
Connection* create_connection(bool will_listen);

void send_all(void* to_send, unsigned int bytes_to_send, Connection* receiver);
char* receive_all(unsigned int bytes_to_read, Connection* sender);
