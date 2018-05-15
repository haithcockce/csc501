#include "socklib.h"

int main() {
    Connection* receiver = create_net_info(true);
    Connection* sender = create_net_info(false);

    set_host_info(receiver, NULL);
    set_host_info(sender, "eden");

    setup_socket(receiver, LISTEN_PORT);
    setup_socket(sender, LISTEN_PORT); 
    return 0;
}
