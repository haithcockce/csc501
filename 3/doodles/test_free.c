#include "socklib.h"

int main() {

    Connection* listening_con = create_connection(true);
    Connection* sending_con = create_connection(false);

    set_host_info(listening_con, NULL);
    set_host_info(sending_con, "eden");

    destroy_connection(sending_con);

    destroy_connection(listening_con);

    /*
    char* s = "test";
    char* h = (char*)malloc(4);
    memcpy(h, s, 4);
    printf("%s\n", h);
    free(h);
    */
    return 0;
}
