#include "socklib.h"

/*
 * get_host_info
 *
 * DESC:
 *     Attempts to get host information based on given hostname or current host. 
 *     We get the hostname either from what is passed in or from the host we are
 *     on. Then we do a deep copy and return the deep copy. 
 * PARM:
 *     char* h
 *         The hostname as a string. Make sure to allocate it first. 
 *     Connection* net_info 
 *         The connection information blob
 * RETN:
 *     -
 * MODS:
 *     struct hostent*
 *         gethostbyname() modifies static memory (requiring the deep copy)
 */ 
void set_host_info(Connection* net_info, char* h) {

    // If a hostname was not provided and we are not the listener
    if(IS_NULL(h) && !(net_info->listens)) {
        error_out("Hostname was not provided\n", 1);
    }

    net_info->set_host = true;

    // temporary hostent to operate on and set
    HostEntity* host_entity = (HostEntity*) malloc(sizeof(HostEntity*));
    net_info->host_entity = host_entity;

    // hostent that is pointed to after gethostbyname. Need to hold this to copy
    // data from since it is statically defined data.
    HostEntity* hp;
    net_info->hostname = (char*) malloc(HOSTNAME_SIZE);

    // If we are the listening proc
    if(net_info->listens) {
        gethostname(net_info->hostname, HOSTNAME_SIZE);
    }
    // else we are the talker
    else {
        net_info->hostname = memcpy(net_info->hostname, h, HOSTNAME_SIZE);
    }
    
    // Get the hostname and do a deep copy
    // The hostent pointer returned literally just points to static memory
    // so a deep copy is required to gether multiple hostents.
    // See netdb.h(0):NOTES
    hp = gethostbyname(net_info->hostname);

    if(IS_NULL(hp)) {
        error_out("Host info could not be gathered\n", 1);
    }

    // Begin copying member by member
    // iterators to help copy data.
    char** he_iterator;
    char** hp_iterator;
    int i;

    net_info->host_entity->h_name = strdup(hp->h_name);

    // count i aliases and copy each. 
    // Make the list null terminated
    for(i = 0; IS_NOT_NULL(hp->h_aliases[i]); i++);
    net_info->host_entity->h_aliases = (char**) malloc(sizeof(char*) * (i + 1));

    he_iterator = net_info->host_entity->h_aliases;
    hp_iterator = hp->h_aliases;
    while(IS_NOT_NULL(*hp_iterator)) {
        *he_iterator = strdup(*hp_iterator);
        he_iterator++;
        hp_iterator++;
    }
    *he_iterator = NULL;

    net_info->host_entity->h_addrtype = hp->h_addrtype;
    net_info->host_entity->h_length = hp->h_length;

    // Count i h_addr_list items and copy each
    // Make the list null terminated
    for(i = 0; IS_NOT_NULL(hp->h_addr_list[i]); i++);
    net_info->host_entity->h_addr_list = (char**) malloc(sizeof(char*) * (i + 1));

    he_iterator = net_info->host_entity->h_addr_list;
    hp_iterator = hp->h_addr_list;
    while(IS_NOT_NULL(*hp_iterator)) {
        *he_iterator = strdup(*hp_iterator);
        he_iterator++;
        hp_iterator++;
    }
    *he_iterator = NULL;
}


/*
 * setup_socket
 * 
 * DESC: 
 *     Creates socket information and stores the socket in net_info. 
 *     First creates the socket address information then either binds the socket
 *     to the information and begins listening or establishes a connection to 
 *     the socket on the remote server.
 * PARM: 
 *     Connection* net_info 
 *         Connection information blob that is assigned the socket created.
 *     int port
 *         port number used with port/IP and socket. 
 * RETN:
 *     -
 * MODS:
 *     Connection* net_info->socket_addr_info
 *         This is assigned the transport protocol and port number
 *     Connection* net_info->socket
 *         The newly created socket is placed here. 
 */
void setup_socket(Connection* net_info, int port) {

    // Create the new socket
    net_info->socket = socket(AF_INET, SOCK_STREAM, 0);

    // Sanity check the socket
    if(net_info->socket < 0) {
        perror("socket:");
        error_out("Socket could not be generated\n", net_info->socket);
    }

    // Assign socket networking information
    net_info->socket_addr_info.sin_family = AF_INET;
    net_info->socket_addr_info.sin_port = htons(port);
    memcpy(&net_info->socket_addr_info.sin_addr, 
            net_info->host_entity->h_addr_list[0],
            net_info->host_entity->h_length);

    int status;

    // If we are the client
    if(net_info->listens) {

        // attempt to bind to the new socket
        do {
            status = bind(net_info->socket, 
                            (struct sockaddr*)&net_info->socket_addr_info, 
                            sizeof(net_info->socket_addr_info));

            // Try a different port if it is already taken
            if(status < 0) {
                port++;    
                net_info->socket_addr_info.sin_port = htons(port);
            }
        } while(status < 0);
        /*
            if(status < 0) {
                perror("bind:");
                error_out("Could not bind\n", status);
            }
        }
        */

        // Then attempt to listen on the new socket
        status = listen(net_info->socket, 5);

        // Sanity check the listen
        if(status < 0) {
            perror("listen:");
            error_out("Could not listen\n", status);
        }
    }

    // Otherwise we are the server
    else {
        status = connect(net_info->socket, 
                         (struct sockaddr*)&net_info->socket_addr_info,
                         sizeof(net_info->socket_addr_info));

        // Sanity check the connect attempt
        if(status < 0) {
            perror("connect:");
            error_out("Could not establish connection\n", status);
        }
    }
    net_info->port = port;

}

void accept_incoming_connection(Connection* listener, Connection* new_con) {
    unsigned int len = sizeof(listener->socket_addr_info);
    new_con->socket = accept(listener->socket, 
                             (struct sockaddr*)&(new_con->socket_addr_info), 
                             &len);
    if(new_con->socket < 0) {
        perror("accept:");
        error_out("Failed to accept a new connection\n", new_con->socket);
    }


}

void talk_on_sock(Connection* connection, char buffer[], unsigned int size) {

}




/**
 * HELPER FUNCTIONS
 */

void error_out(char* err_str, int err_no) {
    fprintf(stderr, err_str);
    exit(err_no);
}

Connection* create_connection(bool will_listen) {
    Connection* to_return = (Connection*) malloc(sizeof(Connection));
    to_return->listens = will_listen;
    to_return->set_host = false;
    return to_return;
}

// FUCK THIS
void send_all(void* to_send, unsigned int bytes_to_send, Connection* receiver) {
    
    // Prepare a buffer to send stuffs
    char buffer[bytes_to_send];
    char* buffer_ptr = buffer;
    memset(buffer, 0, bytes_to_send);
    memcpy(buffer, to_send, bytes_to_send);

    unsigned int bytes_sent = 0;
    unsigned int delta;

    // Loop until we have consumed at lease BYTES_TO_SEND amount of bytes
    do {
        delta = send(receiver->socket, buffer_ptr, (bytes_to_send - bytes_sent), 0);
        if(delta < 0) {
            printf("Failed to send data\n");
            exit(ERR_SEND);
        }
        bytes_sent += delta;
        buffer_ptr += delta;
    } while(bytes_sent < bytes_to_send);
}

// FUCK THAT
char* receive_all(unsigned int bytes_to_read, Connection* sender) {
    char buffer[bytes_to_read];
    char* buffer_ptr = buffer;
    memset(buffer, 0, bytes_to_read);

    unsigned int delta;
    unsigned int bytes_read = 0;
    do {
        delta = recv(sender->socket, buffer_ptr, (bytes_to_read - bytes_read), 0);
        if(delta < 0) {
            printf("Failed to receive data\n");
            exit(ERR_SEND);
        }
        bytes_read += delta;
        buffer_ptr += delta;
    } while(bytes_read < bytes_to_read && delta != 0);
    char* to_return = (char*) malloc(bytes_to_read);
    memcpy(to_return, buffer, bytes_to_read);
    return to_return;
}


void destroy_connection(Connection* n) {
    shutdown(n->socket, SHUT_RDWR);
    close(n->socket);

    if(n->set_host) {
        //free(n->hostname); //<<<<<<<<<<<<<<<<<<<<<< WTF 
        free(n->host_entity->h_name);
        char** he_iterator = n->host_entity->h_aliases;
        while(IS_NOT_NULL(*he_iterator)) {
            free(*he_iterator);
            he_iterator++;
        }
        he_iterator = n->host_entity->h_addr_list;
        while(IS_NOT_NULL(*he_iterator)) {
            free(*he_iterator);
            he_iterator++;
        }
    }

}

