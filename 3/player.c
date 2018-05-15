#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "list.h"
#include "socklib.h"
#include "potato.h"
#include "player.h"

int main(int argc, char* argv[]) {
    init_player(argc, argv);
    play();
    game_over();
    return 0;
}

void init_player(int argc, char* argv[]) {
    check_input(argc, argv);
    checkin(argv[1], atoi(argv[2]));
    setup_listening_connection();
    receive_neighbor_info();
}

void check_input(int argc, char* argv[]) {
        // Check if the amount of args is sane
    if(argc != 3) {
        printf("ERROR: Incorrect number of args\n");
        printf("ERROR: usage is:\n");
        printf("           ./player <master-machine-name> <port-number>\n");
        fflush(stdout);
        exit(ERR_ARGS);
    }

    // Check if the args are actually numbers
    const char* arg_ptr = argv[2];
    bool valid = TRUE;
    while(*arg_ptr && valid) {
        if(!isdigit(*arg_ptr)) {
            valid = FALSE;
        }
        arg_ptr++;
    }

    // We should fall out here if anything was invalid
    if(!valid) {
        printf("ERROR: Port number needs to be a postive integer\n");
        exit(ERR_ARGS);
    }

    // Check if the amount of players is legitimate
    int port = atoi(argv[2]);
    if(port < 1) {
        printf("ERROR: Port number needs to be a postive integer\n");
        exit(ERR_ARGS);
    }
}

void checkin(char* ringmaster_hostname, int port) {
    ringmaster_connection = create_connection(false);
    set_host_info(ringmaster_connection, ringmaster_hostname);
    setup_socket(ringmaster_connection, port);
    char* player_num_buffer = receive_all(sizeof(player_number), ringmaster_connection);

    srand(clock());
    player_number = atoi(player_num_buffer);
    free(player_num_buffer);
    printf("Connected as player %u\n", player_number);
    fflush(stdout);
}


void setup_listening_connection() {

    // Create and setup the socket
    listening_connection = create_connection(true);
    set_host_info(listening_connection, NULL);
    setup_socket(listening_connection, PLAYER_LIST_SOCK);

    // Alert ringmaster what we are listening on
    char port_buffer[8];
    memset(port_buffer, 0, 8);
    sprintf(port_buffer, "%u", listening_connection->port);
    send_all(port_buffer, 8, ringmaster_connection);

}


void receive_neighbor_info() {
    char* hostname_buff;
    char* neighbor_port_buff;
    char* left_player_number_buff;
    char* right_player_number_buff;

    // The ring master will send the socket number first
    neighbor_port_buff = receive_all(8, ringmaster_connection);

    // The ring master will then send the hostname
    hostname_buff = receive_all(HOSTNAME_SIZE, ringmaster_connection);

    left_player_number_buff = receive_all(8, ringmaster_connection);
    right_player_number_buff = receive_all(8, ringmaster_connection);

    left_player_number = atoi(left_player_number_buff);
    right_player_number = atoi(right_player_number_buff);

    right_neighbor_connection = create_connection(false);
    left_neighbor_connection = create_connection(false);

    char neighbor_ack_buff[8];
    memset(neighbor_ack_buff, 0, 8);
    sprintf(neighbor_ack_buff, "%u", NEIGH_ACK);
    send_all(neighbor_ack_buff, 8, ringmaster_connection);

    // If we are the first player
    if(player_number == 0) {

        // Initiate a connection to the right...
        set_host_info(right_neighbor_connection, hostname_buff);
        setup_socket(right_neighbor_connection, atoi(neighbor_port_buff));

        // Then receive a connection from the left
        accept_incoming_connection(listening_connection, left_neighbor_connection);

    }
    // Otherwise, we need to wait then initiate
    else {
        // Receive a connection from the left...
        accept_incoming_connection(listening_connection, left_neighbor_connection);

        // Then initiate a connection to the right
        set_host_info(right_neighbor_connection, hostname_buff);
        setup_socket(right_neighbor_connection, atoi(neighbor_port_buff));
    }
    free(neighbor_port_buff);
    free(hostname_buff);
    free(left_player_number_buff);
    free(right_player_number_buff);
}

void play() {

    // Setup the master read fd set since select seems to modify it
    // along with the other stuff for the select()
    fd_set master_rfds = init_master_rfds();
    fd_set read_fd_set;
    unsigned int maxfds = MAX_SOCKET(
                              listening_connection->socket,
                              left_neighbor_connection->socket,
                              right_neighbor_connection->socket,
                              ringmaster_connection->socket
                          ) + 1;

    read_fd_set = master_rfds;

    path_size_bytes = 0;
    POTATO_SIZE_BYTES = 0;
    POTATO_BASE_SIZE_BYTES = sizeof(unsigned char) + (2 * sizeof(unsigned int));

    // Listen for incoming stuff.
    while(true) {
        select(maxfds, &read_fd_set, NULL, NULL, NULL);

        // Pull the potato off the line
        Connection* ready_conn = get_ready_connection(read_fd_set);
        read_fd_set = master_rfds;  // gotta reset this
        receive_potato(ready_conn);
        //char* potato_base_buff = receive_all(POTATO_BASE_SIZE_BYTES, ready_conn);
        //memcpy(&potato, potato_base_buff, POTATO_BASE_SIZE_BYTES);
        if(path_size_bytes == 0) {
            path_size_bytes = (sizeof(unsigned int) * potato.hops);
        }
        if(POTATO_SIZE_BYTES == 0) {
            POTATO_SIZE_BYTES = (POTATO_BASE_SIZE_BYTES + (sizeof(unsigned int) * potato.hops));
        }
        // If a GAME_OVER was sent, fall out to clean up
        if(potato.flag == GAME_OVER) {
            return;
        }

        // Otherwise, read in the path and update.
        free(potato.path);
        char* potato_path_buff = receive_all((sizeof(unsigned int) * potato.hops), ready_conn);
        potato.path = (unsigned int*) potato_path_buff;

        potato.path[potato.current_hop] = player_number;
        potato.current_hop++;

        // If we were "it", let the ringmaster know. We continue since the
        // ringmaster will walkthrough its player list sending GAME_OVERs.
        if(potato.current_hop == potato.hops) {
            alert_ringmaster();
            send_game_over();
        }

        // Now determine the next player and send off the potato
        unsigned int choice = RANDOM_NUMBER(2);
        Connection* next_player = choice == LEFT ? left_neighbor_connection : right_neighbor_connection;
        printf("Sending potato to %u\n", (choice == LEFT) ? left_player_number : right_player_number);
        fflush(stdout);
        //memset(potato_base_buff, 0, POTATO_BASE_SIZE_BYTES);
        //memcpy(potato_base_buff, &potato, POTATO_BASE_SIZE_BYTES);
        //send_all(potato_base_buff, POTATO_BASE_SIZE_BYTES, next_player);
        send_potato(next_player);
        send_all(potato_path_buff, path_size_bytes, next_player);

    }
}


void send_game_over() {
    // Send Game over potato to right neighbor then close the connection
    potato.flag = GAME_OVER;
    //char game_over_buff[POTATO_BASE_SIZE_BYTES];
    //memset(game_over_buff, 0, POTATO_BASE_SIZE_BYTES);
    //memcpy(game_over_buff, &potato, POTATO_BASE_SIZE_BYTES);
    //send_all(game_over_buff, POTATO_BASE_SIZE_BYTES, right_neighbor_connection);
    send_potato(right_neighbor_connection);
    //char* throwaway = receive_all(POTATO_BASE_SIZE_BYTES, left_neighbor_connection);
    receive_potato(left_neighbor_connection);
    char ack_buff[4];
    memset(ack_buff, 0, 4);
    sprintf(ack_buff, "%u", ACK);
    send_all(ack_buff, 4, ringmaster_connection);
    
    //free(throwaway);
    destroy_connection(listening_connection);
    destroy_connection(left_neighbor_connection);
    destroy_connection(right_neighbor_connection);
    destroy_connection(ringmaster_connection);
    exit(0);
}

void game_over() {
    // we know this came from the left neighbor always
    // We should also already have the game over potato
    destroy_connection(left_neighbor_connection);
    //char game_over_buff[POTATO_BASE_SIZE_BYTES];
    //memset(game_over_buff, 0, POTATO_BASE_SIZE_BYTES);
    //memcpy(game_over_buff, &potato, POTATO_BASE_SIZE_BYTES);
    //send_all(game_over_buff, POTATO_BASE_SIZE_BYTES, right_neighbor_connection);
    send_potato(right_neighbor_connection);
    destroy_connection(right_neighbor_connection);
    destroy_connection(ringmaster_connection);
    destroy_connection(listening_connection);
    exit(0);
}

void alert_ringmaster() {
    printf("I'm it\n");
    potato.flag = GAME_OVER;
    //char potato_base_buff[POTATO_BASE_SIZE_BYTES];
    //memset(potato_base_buff, 0, POTATO_BASE_SIZE_BYTES);
    //memcpy(potato_base_buff, &potato, POTATO_BASE_SIZE_BYTES);
    //send_all(potato_base_buff, POTATO_BASE_SIZE_BYTES, ringmaster_connection);
    send_potato(ringmaster_connection);
    send_all(potato.path, path_size_bytes, ringmaster_connection);   
}

fd_set init_master_rfds() {
    fd_set master_rfds;
    FD_ZERO(&master_rfds);
    FD_SET(listening_connection->socket, &master_rfds);
    FD_SET(left_neighbor_connection->socket, &master_rfds);
    FD_SET(right_neighbor_connection->socket, &master_rfds);
    FD_SET(ringmaster_connection->socket, &master_rfds);
    return master_rfds;
}

Connection* get_ready_connection(fd_set read_fd_set) {
    if(FD_ISSET(left_neighbor_connection->socket, &read_fd_set)) {
        return left_neighbor_connection;
    }
    else if(FD_ISSET(right_neighbor_connection->socket, &read_fd_set)) {
        return right_neighbor_connection;
    }
    else if(FD_ISSET(ringmaster_connection->socket, &read_fd_set)) {
        return ringmaster_connection;
    }
    return NULL;
}


void send_potato(Connection* conn) {
    char potato_buff[8];
    memset(potato_buff, 0, 8);
    sprintf(potato_buff, "%u", potato.flag);
    send_all(potato_buff, 8, conn);

    memset(potato_buff, 0, 8);
    sprintf(potato_buff, "%u", potato.hops);
    send_all(potato_buff, 8, conn);

    memset(potato_buff, 0, 8);
    sprintf(potato_buff, "%u", potato.current_hop);
    send_all(potato_buff, 8, conn);
}

void receive_potato(Connection* conn) {
    char* potato_buff = receive_all(8, conn);
    potato.flag = atoi(potato_buff);
    free(potato_buff);

    potato_buff = receive_all(8, conn);
    potato.hops = atoi(potato_buff);
    free(potato_buff);

    potato_buff = receive_all(8, conn);
    potato.current_hop = atoi(potato_buff);
    free(potato_buff);
}
