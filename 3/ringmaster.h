#define LISTENER true

#define GAME_OVER   100
#define POTATO      101
#define NEIGH_ACK   102
#define NEIGH_INFO  103
#define PLAYER_NUM  104

#define ERR_ARGS    1
#define ERR_SEND    2
#define ERR_NEIGH   3

#define MAX(A, B) (((A) > (B)) ? (A) : (B))

Potato potato;
GenericLinkedList* player_list;
int player_count;

Connection* listening_connection;

typedef struct PlayerReference {
    unsigned int number;
    Connection* connection;
    unsigned int listening_port;
} PlayerReference;

void init_ring_master(int argc, char* argv[]);
void init_game(char* argv[]);
void start_game();
void game_over();

void check_input(int argc, char* argv[]);
void setup_listening_connection(int port);
void setup_player_list();
void send_neighbor_info();
void init_potato(unsigned int hops);
void play_game();
void determine_first_player();
void fire_potato();
void wait_for_game_over();
//void send_potato(Connection* conn);
//void receive_potato(Connection* conn);
void clean_up();
