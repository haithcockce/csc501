#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "list.h"
#include "socklib.h"
#include "potato.h"
#include "ringmaster.h"

int main(int argc, char* argv[]) {
    init_ring_master(argc, argv);
    init_game(argv);
    play_game();
    game_over();
    return 0;
}

void init_ring_master(int argc, char* argv[]) {
    check_input(argc, argv);
    setup_listening_connection(atoi(argv[1]));
}

void init_game(char* argv[]) {
    srand(clock());
    setup_player_list();
    send_neighbor_info();
    init_potato(atoi(argv[3]));
}
void play_game() {
    fire_potato();
    wait_for_game_over();
}



void game_over() {
    // send the potato base back out since it already contains the game over
    // to all the players.
}

void check_input(int argc, char* argv[]) {

    // Check if the amount of args is sane
    if(argc != 4) {
        printf("ERROR: Incorrect number of args\n");
        printf("ERROR: usage is:\n");
        printf("           ./master <port-number> <number-of-players> <hops>\n");
        fflush(stdout);
        exit(ERR_ARGS);
    }

    // Check if the args are actually numbers
    const char* arg_ptr;
    bool valid = TRUE;
    int i;
    for(i = 1; (i < argc && valid); i++) {
        arg_ptr = argv[i];

        while(*arg_ptr && valid) {
            if(!isdigit(*arg_ptr)) {
                valid = FALSE;
            }
            arg_ptr++;
        }
    }

    // We should fall out here if anything was invalid
    if(!valid) {
        printf("ERROR: Argument at position %d is not a positive integer\n", i - 1);
        exit(ERR_ARGS);
    }

    // Check if the amount of players is legitimate
    player_count = atoi(argv[2]);
    if(player_count < 2) {
        printf("ERROR: You can not play a multiplayer game with less than 2 people\n");
        exit(ERR_ARGS);
    }
    int port = atoi(argv[1]);
    if(port < 1) {
        printf("ERROR: Port number needs to be a positive integer.\n");
        exit(ERR_ARGS);
    }

    // Now I have to print the master startup
    char hostname_buffer[HOSTNAME_SIZE];
    gethostname(hostname_buffer, HOSTNAME_SIZE);
    printf("Potato Master on %s\n", hostname_buffer);
    printf("Players = %u\n", player_count);
    printf("Hops = %u\n", atoi(argv[3]));
    fflush(stdout);
}


void setup_listening_connection(int port) {
    listening_connection = create_connection(LISTENER);
    set_host_info(listening_connection, NULL);
    setup_socket(listening_connection, port);
}


void setup_player_list() {
    player_list = new_linked_list(sizeof(PlayerReference), NULL);
    unsigned int connected_players = 0;
    while(connected_players < player_count) {

        // Accept the incoming connection
        Connection* new_connection = create_connection(!LISTENER);
        accept_incoming_connection(listening_connection, new_connection);

        // Grab the set all the host information
        PlayerReference* new_player = (PlayerReference*) malloc(sizeof(PlayerReference));
        new_player->number = connected_players;
        new_player->connection = new_connection;
        HostEntity* h = gethostbyaddr(
                            &new_connection->socket_addr_info.sin_addr,
                            sizeof(new_connection->socket_addr_info.sin_addr),
                            AF_INET
                        );
        set_host_info(new_connection, h->h_name);

        // Print info requested for assignment
        printf("player %u is on %s\n", new_player->number, new_player->connection->hostname);
        fflush(stdout);

        // Send player number
        char player_count_buffer[8];
        memset(player_count_buffer, 0, 8);
        sprintf(player_count_buffer, "%u", connected_players);
        send_all(player_count_buffer, sizeof(connected_players), new_connection);

        // Receive the listening port number
        char* listening_port_buff = receive_all(8, new_connection);
        new_player->listening_port = atoi(listening_port_buff);
        free(listening_port_buff);

        ll_append(player_list, new_player);
        connected_players++;
    }
}


void send_neighbor_info() {
    ll_node* list_iterator = player_list->head;

    // For readable code
    PlayerReference* curr_player;
    PlayerReference* right_player;
    Connection* curr_player_conn;
    Connection* right_player_conn;
    unsigned int left_player_number;
    unsigned int right_player_number;

    // Now we can actually send out host info
    list_iterator = player_list->head;
    while(list_iterator != NULL) {
        // Get the player we are informing
        curr_player = (PlayerReference*)list_iterator->data;
        curr_player_conn = curr_player->connection;

        // Hostnames to send to the player in order to make connections
        char* right_neighbor_hname;

        // Getting right neighbor
        // if we are the tail, right neighbor is head
        if(list_iterator == player_list->tail) {
            right_player = (PlayerReference*)player_list->head->data;
        }
        // Otherwise simply next
        else {
            right_player = (PlayerReference*)list_iterator->next->data;
        }
        if(list_iterator == player_list->head) {
            left_player_number = ((PlayerReference*)player_list->tail->data)->number;
        }
        else {
            left_player_number = ((PlayerReference*)list_iterator->prev->data)->number;
        }
        right_player_number = right_player->number;
        right_player_conn = right_player->connection;
        right_neighbor_hname = right_player_conn->host_entity->h_name;

        char hostname_buffer[HOSTNAME_SIZE];
        memset(hostname_buffer, 0, HOSTNAME_SIZE);
        memcpy(hostname_buffer, right_neighbor_hname, HOSTNAME_SIZE);

        char player_port_buff[8];
        memset(player_port_buff, 0, 8);
        sprintf(player_port_buff, "%u", right_player->listening_port);

        char left_player_number_buff[8];
        memset(left_player_number_buff, 0, 8);
        sprintf(left_player_number_buff, "%u", left_player_number);

        char right_player_number_buff[8];
        memset(right_player_number_buff, 0, 8);
        sprintf(right_player_number_buff, "%u", right_player_number);
        // Need to first send the right player port number since we use it to
        // determine listening port
        send_all(player_port_buff, 8, curr_player_conn);

        // Now send the hostname
        if(hostname_buffer[HOSTNAME_SIZE - 1] != '\0') {
            hostname_buffer[HOSTNAME_SIZE - 1] = '\0';
        }

        // Send the hostname information and the player numbers
        send_all(hostname_buffer, HOSTNAME_SIZE, curr_player_conn);
        send_all(left_player_number_buff, 8, curr_player_conn);
        send_all(right_player_number_buff, 8, curr_player_conn);

        char* ack_buffer = receive_all(8, curr_player_conn);
        if(atoi(ack_buffer) != NEIGH_ACK) {
            exit(3);
        }
        free(ack_buffer);
        list_iterator = list_iterator->next;
    }
}

void init_potato(unsigned int hops) {
    potato.flag = POTATO;
    potato.hops = hops;
    potato.current_hop = 0;
    //unsigned int buff[hops];
    path_size_bytes = (sizeof(unsigned int) * hops);
    potato.path = (unsigned int*) malloc(path_size_bytes);
    memset(potato.path, 0, path_size_bytes);
    POTATO_BASE_SIZE_BYTES = sizeof(unsigned char) + (2 * sizeof(unsigned int));
    POTATO_SIZE_BYTES = (POTATO_BASE_SIZE_BYTES + (sizeof(unsigned int) * hops));
}

void fire_potato(PlayerReference* first_player) {

    // Determine the first player to send the potato to
    ll_node* list_iterator = player_list->head;
    unsigned int chosen = RANDOM_NUMBER(player_list->length);
    printf("All players present, sending potato to player %u\n", chosen);
    fflush(stdout);
    for(; chosen > 0; (list_iterator = list_iterator->next, chosen--));  // for loop voodoo

    // Gather the data to send
    //char potato_buffer[POTATO_BASE_SIZE_BYTES];
    //memset(potato_buffer, 0, POTATO_BASE_SIZE_BYTES);
    //memcpy(potato_buffer, &potato, POTATO_BASE_SIZE_BYTES);
    Connection* first_player_conn = ((PlayerReference*)list_iterator->data)->connection;

    // Send the potato header and then path
    //send_all(potato_buffer, POTATO_BASE_SIZE_BYTES, first_player_conn);
    send_potato(first_player_conn);
    send_all(potato.path, path_size_bytes, first_player_conn);
}
void wait_for_game_over() {

    // setup for the select
    ll_node* list_iterator = player_list->head;
    SockDesc max_socket = ((PlayerReference*)list_iterator->data)->connection->socket;
    SockDesc socket;
    fd_set master_rfds;
    FD_ZERO(&master_rfds);

    // add each socket to the set and grab the max one
    while(list_iterator != NULL) {
        socket = ((PlayerReference*)list_iterator->data)->connection->socket;
        FD_SET(socket, &master_rfds);
        if(socket > max_socket) {
            max_socket = socket;
        }
        list_iterator = list_iterator->next;
    }

    // Listen for a connection and then determine who sent it.
    select(max_socket + 1, &master_rfds, NULL, NULL, NULL);
    list_iterator = player_list->head;
    PlayerReference* gameover_player;
    while(list_iterator != NULL) {
        if(FD_ISSET(((PlayerReference*)list_iterator->data)->connection->socket, &master_rfds)) {
            gameover_player = (PlayerReference*)list_iterator->data;
            break;
        }
        list_iterator = list_iterator->next;
    }

    //char* potato_base_buff = receive_all(POTATO_BASE_SIZE_BYTES, gameover_player->connection);
    //memcpy(&potato, potato_base_buff, POTATO_BASE_SIZE_BYTES);
    //free(potato_base_buff);
    receive_potato(gameover_player->connection);
    if(potato.flag == GAME_OVER) {
        // get the path and print it off
        char* potato_path_buff = receive_all(path_size_bytes, gameover_player->connection);
        //memcpy(potato.path, potato_path_buff, path_size_bytes);
        potato.path = (unsigned int*) potato_path_buff;
        printf("Trace of potato:\n");
        fflush(stdout);
        int i;
        for(i = 0; i < potato.hops; i++) {
            printf("%u,", potato.path[i]);
        }
        free(potato_path_buff);
        fflush(stdout);

        
        // Break connection with all
        char* can_close_buff = receive_all(4, gameover_player->connection);
        unsigned int can_close = atoi(can_close_buff);
        free(can_close_buff);
        if(can_close != ACK) {
            printf("Should have received the go ahead to close\n");
            exit(ACK);
        }
        list_iterator = player_list->head;
        while(list_iterator != NULL) {
            destroy_connection(((PlayerReference*)list_iterator->data)->connection);
            list_iterator = list_iterator->next;
        }
        ll_destroy(player_list);
       
        return;
    }
    else {
        printf("\n\nDEBUG: wait_for_game_over: We should have received a game over\n\n");
    }

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
