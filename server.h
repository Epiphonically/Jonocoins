#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdint.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <unistd.h>

#define MAX_SOCK_QUEUE 10
#define MAX_USERNAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define PORT "60000"

#define MAX_PACKET_SIZE 1 << 10
#define MAX_NAME_LEN 32
#define MAX_PASSWORD_LEN 32

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

struct net_flag {
    int set;
    char *buf;
};

struct User {
    int sockfd;
    
    int logged_in;
    char *name;
    char *password;
    int user_id;

    int is_admin;

    struct net_flag net_flags[NUM_NET_ACTIONS];
    
    struct User *next;
    struct User *prev;
};

struct Shop_Item {
    char *name;
    int price;
};

struct Order {
    char *name;
    struct Shop_Item item;
};

struct user_pass_tuple {
    char username[MAX_NAME_LEN + 1];
    char password[MAX_PASSWORD_LEN + 1];
    uint32_t balance;
    uint32_t is_admin;
};

struct packet {
    uint32_t data_len;
    uint8_t action;
    uint8_t buf[0];
};

struct User *user_list; 
FILE *data; 
int data_fd; 

FILE *shop; 
int shop_fd; 

FILE *orders; 
int orders_fd; 
