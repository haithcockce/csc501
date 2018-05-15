#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#define LISTEN_PORT 		50000
#define MASTER_PORT 		60000
#define IS_LEFT_SOCK(x) 	(x == LEFT_SOCK ? true : false)
#define IS_RIGHT_SOCK(x) 	(x == RIGHT_SOCK ? true : false)


#define HOSTNAME_SIZE 64

typedef int SockDesc;
typedef int Port;
typedef struct hostent HostEntity;

typedef struct Connection {
	bool listens;
	char* hostname;
	HostEntity* host_entity;
	int port;
	SockDesc socket; 
	int address_length;
	struct sockaddr_in socket_addr_info;
} Connection;

void set_host_info(Connection* net_info, char* h);
void setup_socket(Connection* net_info, int port);
void listen_for_incoming_connection(Connection* listener, Connection* new_con);
void accept_incoming_connection(Connection* listener, Connection* new_con);
void cleanup();
void error_out(char* err_str, int err_no);
Connection* create_connection(bool will_listen);
