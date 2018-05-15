#include "socklib.h"

int main() {
    Connection* listening_con = create_connection(true);
    Connection* sending_con = create_connection(false);

    set_host_info(listening_con, NULL);
    set_host_info(sending_con, "Bravo");

    setup_socket(listening_con, 50000);
    setup_socket(sending_con, 50000); 

    Connection* receiving_con = create_connection(false); 
    accept_incoming_connection(listening_con, receiving_con);

    HostEntity* h = gethostbyaddr((char*) &(receiving_con->socket_addr_info.sin_addr), sizeof(struct in_addr), AF_INET);
    printf(">>>>>> Connected to %s\n", h->h_name);



    unsigned int numb_buff[3];
    memset(numb_buff, 0, 3 * sizeof(unsigned int));
    numb_buff[0] = 0;
    send_all(numb_buff, 5, sending_con);

    char* buff = receive_all(5, receiving_con);
    unsigned int received_numbers[3];
    memset(received_numbers, 0, 3 * sizeof(unsigned int));
    memcpy(received_numbers, buff, 3 * sizeof(unsigned int));
    printf(buff);

    destroy_connection(receiving_con);
    destroy_connection(sending_con);

    destroy_connection(listening_con);

    return 0;
}
