#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdint.h>

#include <pthread.h>

#include <windows.h>

#include <ws2tcpip.h>


#define MAX_LINE 256
#define MAX_COMMAND 256
#define MAX_COMMAND_ARG 256
#define PORT 60000

#define IP "127.0.0.1"

#define MAX_PACKET_SIZE 1 << 10

enum NET_ACTIONS {
    NET_CLEAR,
    NET_CREATE,
    NET_LOGIN,
    NET_CREATE_PASSWORD,
    NET_ENTER_PASSWORD,
    NET_LOGOUT,
    NET_GET_BAL,
    NET_LEADERBOARD,
    NET_ADD_BAL,
    NET_SUB_BAL,
    NET_SET_BAL,
    NET_ENTER_AMMOUNT,
    NET_GET_SHOP,
    NET_ADD_TO_SHOP,
    NET_REMOVE_FROM_SHOP,
    NET_BUY_REQUEST,
    NET_BUY,
    NET_GET_ORDERS,
    NET_ACK,
    NET_NACK,
    NUM_NET_ACTIONS
};

enum IO_STATE {
    STATE_COMMANDS,
    STATE_ENTER_PASSWORD,
    STATE_CREATE_PASSWORD,
    STATE_ENTER_AMMOUNT,
    STATE_YES_NO
};

struct packet {
    uint32_t data_len;
    uint8_t action;
    uint8_t buf[0];
};

int state; 
SOCKET client_sock;

uint8_t recv_buf[MAX_PACKET_SIZE];
fd_set select_me;