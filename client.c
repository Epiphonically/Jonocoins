#include "client.h"
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



int safely_send(void *buf) {
	struct packet *the_packet = (struct packet *) buf;
	size_t need = sizeof(struct packet) + the_packet->data_len;
	the_packet->data_len = htonl(the_packet->data_len);
	size_t sent = 0;
	int incr;
	while (sent < need) {
		incr = send(client_sock, buf + sent, need - sent, 0);
		if (incr <= 0) {
			printf("send failed\n");
			fflush(stdout);
			return -1;
		}
		sent += incr;
	}
	return sent;
}

int safely_recv(void *buf) { 
    
    if (recv(client_sock, buf, (sizeof(struct packet)), 0) != (sizeof(struct packet))) {
		printf("no header catastrophic\n");
		fflush(stdout);
		return -1;
	}
    
    struct packet *the_packet = (struct packet *) buf;
	the_packet->data_len = ntohl(the_packet->data_len);
    size_t need = the_packet->data_len + sizeof(struct packet);
    size_t got = sizeof(struct packet);

    //printf("sizeof: %d, data_len: %d\n", sizeof(struct packet), the_packet->data_len);
    int incr;
    while (got < need) {
        incr = recv(client_sock, buf + got, need - got, 0);
        if (incr <= 0) {
			printf("recv failed\n");
			fflush(stdout);
            return -1;
        }
        got += incr;
    }
    return got;
}

int wait_for_ack() {
	/* timeval struct for no cooldown */
    struct timeval zero_time;
	zero_time.tv_sec = 0;
	zero_time.tv_usec = 0;

	while (select(client_sock + 1, &select_me, NULL, NULL, &zero_time) <= 0) {
		FD_ZERO(&select_me);
		FD_SET(client_sock, &select_me);
	}

	if (safely_recv(recv_buf) < 0) {
		return -1;
	} else {
		struct packet *packet = (struct packet *) recv_buf;
		switch (packet->action) {
			case NET_ACK:
				printf("%s", (char *) packet->buf);
				return 1;
			break;

			case NET_NACK:
				printf("%s", (char *) packet->buf);
				return -1;
			break;

			default:
				printf("Catastrophic failure!\n");
				return -2;
			break;
		}
	}
	return 1;
}

void load_command_arg(char *line, char *command, char *command_arg) {
    int sweep, idx;

    fgets(line, MAX_LINE, stdin);
    sweep = 0;
    idx = 0;
    for (; sweep < MAX_LINE; sweep++) {
        
        if (line[sweep] != ' ' && line[sweep] != '\n' && idx != MAX_COMMAND - 1) {
            command[idx++] = line[sweep];
        } else {
            sweep++;
            command[idx++] = '\0'; 
            break;
        }
    }
    idx = 0;
    for (; sweep < MAX_LINE; sweep++) {
        if (line[sweep] != ' ' && line[sweep] != '\n' && idx != MAX_COMMAND_ARG - 1) {
            command_arg[idx++] = line[sweep];
        } else {
            command_arg[idx++] = '\0'; 
            break;
        }
    }

}

int Manage_Command(char *command, char *command_arg) {
	struct packet *packet;
	char *lower_command = malloc(strlen(command) + 1);
	strcpy(lower_command, command);
	int i;
	for (i = 0; i < strlen(lower_command); i++) {
		lower_command[i] = tolower(lower_command[i]);
	}
	switch (state) {
		case STATE_COMMANDS:
			if (strcmp(lower_command, "clear") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_CLEAR;

				safely_send(packet);

				free(packet);

				wait_for_ack();
			} else if (strcmp(lower_command, "help") == 0) {
				printf("---------------------------\nJonathan Dollar Commands:\nhelp: list all commands\ncreate <username>: create new account\nlogin <username>: log into account\nbalance: see account balance\nleaderboard: see leaderboard\nlogout: logout of account\nshop: view shop\nbuy <item name>: buy item\norders: view current orders\n---------------------------\n");
				fflush(stdout);
			} else if (strcmp(lower_command, "login") == 0) {

				printf("trying to login\n");
				fflush(stdout);
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_LOGIN;
				strcpy(packet->buf, command_arg);
				safely_send(packet);

				free(packet);
				if (wait_for_ack() > 0) {
					
					state = STATE_ENTER_PASSWORD;
				}
			} else if (strcmp(lower_command, "create") == 0) {
				printf("Trying to create new account\n");
				fflush(stdout);
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_CREATE;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);

				if (wait_for_ack() > 0) {
					
					state = STATE_CREATE_PASSWORD;
				}
			} else if (strcmp(lower_command, "logout") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_LOGOUT;
				
				safely_send(packet);
				free(packet);

				wait_for_ack();
			} else if (strcmp(lower_command, "balance") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_GET_BAL;
			
				safely_send(packet);
				free(packet);

				wait_for_ack();
			} else if (strcmp(lower_command, "leaderboard") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_LEADERBOARD;
			
				safely_send(packet);
				free(packet);

				wait_for_ack();
			} else if (strcmp(lower_command, "add") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_ADD_BAL;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);

				if (wait_for_ack() > 0) {
					state = STATE_ENTER_AMMOUNT;
				}
			} else if (strcmp(lower_command, "sub") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_SUB_BAL;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);

				if (wait_for_ack() > 0) {
					state = STATE_ENTER_AMMOUNT;
				}
			} else if (strcmp(lower_command, "set") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_SET_BAL;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);

				if (wait_for_ack() > 0) {
					state = STATE_ENTER_AMMOUNT;
				}
			} else if (strcmp(lower_command, "shop") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_GET_SHOP;
				safely_send(packet);
				free(packet);
				wait_for_ack();
			} else if (strcmp(lower_command, "shopadd") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_ADD_TO_SHOP;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);
				if (wait_for_ack() > 0) {
					state = STATE_ENTER_AMMOUNT;
				}
			} else if (strcmp(lower_command, "shopremove") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_REMOVE_FROM_SHOP;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);
				wait_for_ack();
			} else if (strcmp(lower_command, "buy") == 0) {
				packet = malloc(sizeof(struct packet) + strlen(command_arg) + 1);
				packet->data_len = strlen(command_arg) + 1;
				packet->action = NET_BUY;
				strcpy(packet->buf, command_arg);
				safely_send(packet);
				free(packet);
				if (wait_for_ack() > 0) {
					state = STATE_YES_NO;
				}
			} else if (strcmp(lower_command, "orders") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_GET_ORDERS;
				safely_send(packet);
				free(packet);
				wait_for_ack();
			}
		break;
		
		case STATE_CREATE_PASSWORD:
			
			packet = malloc(sizeof(struct packet) + strlen(command) + 1);
			packet->data_len = strlen(command) + 1;
			packet->action = NET_CREATE_PASSWORD;
			strcpy(packet->buf, command);
			safely_send(packet);

			free(packet);

			wait_for_ack();
			state = STATE_COMMANDS;
			
		break;

		case STATE_ENTER_PASSWORD:
			packet = malloc(sizeof(struct packet) + strlen(command) + 1);
			packet->data_len = strlen(command) + 1;
			packet->action = NET_ENTER_PASSWORD;
			strcpy(packet->buf, command);
			safely_send(packet);

			free(packet);

			wait_for_ack();
			state = STATE_COMMANDS;
		break;

		case STATE_ENTER_AMMOUNT:
			packet = malloc(sizeof(struct packet) + sizeof(uint32_t));
			packet->data_len = sizeof(uint32_t);
			packet->action = NET_ENTER_AMMOUNT;
			*((int *) packet->buf) = atoi(command);
			
			safely_send(packet);

			free(packet);
			wait_for_ack();
			state = STATE_COMMANDS;
		break;	

		case STATE_YES_NO:
			if (strcmp(lower_command, "yes") == 0 || strcmp(lower_command, "y") == 0) {
				packet = malloc(sizeof(struct packet));
				packet->data_len = 0;
				packet->action = NET_BUY;
				safely_send(packet);
				free(packet);
				wait_for_ack();
				
			} else {
				printf("Purchase canceled.\n");
			}
			state = STATE_COMMANDS;
		break;
	}
	free(lower_command);
	return 1;
}

void handle_io() {

    char command[MAX_COMMAND];
    char command_arg[MAX_COMMAND_ARG];
    char line[MAX_COMMAND + MAX_COMMAND_ARG + 1];


	char send_buf[256];
	

	while (1) {
		command[0] = '\0';
		command_arg[0] = '\0';
		line[0] = '\0';
		

		load_command_arg(line, command, command_arg);


		// printf("Command: %s, Arg: %s\n", command, command_arg);
		// fflush(stdout);

		
		if (Manage_Command(command, command_arg) < 0) {
			return;
		}






	}
}

int main(/* int argc, char *argv[] */) {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	SetConsoleOutputCP(CP_UTF8);
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, IP, &server_addr.sin_addr);

	/* Connect to server */
	printf("Trying to connect\n");
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == 0) {
        printf("Connected to server!\n");
        fflush(stdout);
    } else {
        printf("Could not establish connection to server.\n");
        fflush(stdout);
        exit(1);
    }
	state = STATE_COMMANDS;

	
    
    


	
	/* timeval struct for no cooldown */
    struct timeval zero_time;
	zero_time.tv_sec = 0;
	zero_time.tv_usec = 0;

	printf("type \"help\" to see list of available commands\n");
	fflush(stdout);
	/* Hande io */
	handle_io();

    
    return 0;
}