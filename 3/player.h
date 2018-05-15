#define LISTENER true

#define ERR_ARGS 1

#define PLAYER_LIST_SOCK    20000

#define GAME_OVER   100
#define POTATO      101
#define NEIGH_ACK   102
#define NEIGH_INFO  103
#define PLAYER_NUM  104
#define LEFT        0

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MAX_SOCKET(A, B, C, D) (MAX(MAX(A, B), MAX(C, D)))

Potato potato;
unsigned int player_number;
unsigned int left_player_number;
unsigned int right_player_number;

Connection* listening_connection;
Connection* left_neighbor_connection;
Connection* right_neighbor_connection;
Connection* ringmaster_connection;

void init_player(int argc, char* argv[]);
void play();
void game_over();


void check_input(int argc, char* argv[]);
void setup_listening_connection();
void checkin(char* ringmaster_hostname, int port);
void receive_neighbor_info();
void send_neighbor_ack();
void start_game();
fd_set init_master_rfds();
Connection* get_ready_connection(fd_set read_fd_set);
void alert_ringmaster();
void wait_for_potato_or_gameover();
void update_potato();
//void send_potato(Connection* conn);
//void receive_potato(Connection* conn);
void toss_potato();
void send_game_over();
