#include "socklib.h"

void main() {
    NetInfo* n1 = create_net_info(true);
    set_host_info(n1, NULL);
    printf("n1: hostname: %s\n", n1->host_entity->h_name);

    NetInfo* n2 = create_net_info(false);
    set_host_info(n2, "Bravo");
    printf("h2: hostname: %s\n", n2->host_entity->h_name);

    NetInfo* n3 = create_net_info(false);
    set_host_info(n3, "NURPE");
    printf("h3: hostname: %s\n", n3->host_entity->h_name);


    free(n1);
    free(n2);
    free(n3);
}
