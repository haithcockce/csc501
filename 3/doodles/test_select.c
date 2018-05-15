#include "socklib.h"

int main() {
    Connection* listening_con = create_connection(true);
    Connection* sending_con = create_connection(false);

    set_host_info(listening_con, NULL);
    set_host_info(sending_con, "smetana");

    setup_socket(listening_con, 50000);
    setup_socket(sending_con, 50000); 

    Connection* receiving_con = create_connection(false); 
    accept_incoming_connection(listening_con, receiving_con);

    HostEntity* h = gethostbyaddr((char*) &(receiving_con->socket_addr_info.sin_addr), sizeof(struct in_addr), AF_INET);
    printf(">>>>>> Connected to %s\n", h->h_name);


    fd_set read_fd_set;
    FD_ZERO(&read_fd_set);
    FD_SET(receiving_con->socket, &read_fd_set);

    send_all("test", 5, sending_con);

    select(1, &read_fd_set, NULL, NULL, NULL);

    if(FD_ISSET(receiving_con->socket, &read_fd_set))
        printf("The connection was readable\n");

    char* buff = receive_all(5, receiving_con);
    printf(buff);

    destroy_connection(receiving_con);
    destroy_connection(sending_con);

    destroy_connection(listening_con);

    return 0;
}
