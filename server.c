#include "server.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int get_bal(char *username) {
    
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(username, user_pass.username) == 0) {
            return user_pass.balance;

        }

    }
    /* User not found */
    return -1;
}

int add_bal(char *username, int ammount, int idenpotent) {
    /* Read superblock */
    
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(username, user_pass.username) == 0) {
            fseek(data, sizeof(num_accounts) + (i * sizeof(struct user_pass_tuple)), SEEK_SET);
            printf("Seeking to: %d for, %s\n", 4 + (i * sizeof(struct user_pass_tuple)), username);
            if (idenpotent) {
                if (ammount < 0) {
                    user_pass.balance = 0;
                } else {
                    user_pass.balance = ammount;
                }
            } else {
                if (ammount < 0 && (-1 * ammount) > user_pass.balance) {
                    user_pass.balance = 0;
                } else {
                    user_pass.balance += ammount;
                }
                 
            }
           
            fwrite(&user_pass, sizeof(struct user_pass_tuple), 1, data);

            return 1;
        } 
    }
    
    return 0;
}

struct User* Find_User_Struct(int fd) {
    struct User *loop_user = user_list->next;

    for (; loop_user != user_list; loop_user = loop_user->next) {
        if (fd == loop_user->sockfd) {
        
            return loop_user;
        }
    }
    return NULL;
}   

int load_user_data(struct User *him) {
     /* Read superblock */
    
    char buf[4];
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(him->name, user_pass.username) == 0) {
            him->is_admin = user_pass.is_admin;
            return 1;
        } 
    }
    
    return 0;
}

int load_user_pass_tuple(char *username, struct user_pass_tuple *load_me) {
    /* Read superblock */
    
    char buf[4];
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(username, user_pass.username) == 0) {
            memcpy(load_me, &user_pass, sizeof(struct user_pass_tuple));
            return 1;
        } 
    }
    
    return 0;
}

int Find_User(char *username, char *password) {
    /* Read superblock */
    
    char buf[4];
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(username, user_pass.username) == 0) {
            strcpy(password, user_pass.password);
            return 1;
        } 
    }
    
    return 0;
}

int Create_User(char *username, char *password) {
    char buf[4];
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);
    
    fflush(stdout);
    fread(&num_accounts, sizeof(num_accounts), 1, data);
   
    num_accounts++;
    fseek(data, 0, SEEK_SET);
    fwrite(&num_accounts, sizeof(num_accounts), 1, data);
 
    fseek(data, sizeof(num_accounts) + ((num_accounts - 1) * sizeof(struct user_pass_tuple)), SEEK_SET);
    uint32_t i;

    struct user_pass_tuple user_pass;
    memset(&user_pass, 0, sizeof(struct user_pass_tuple));
    strcpy(user_pass.username, username);
    strcpy(user_pass.password, password);
   
    printf("User created! username: %s, password: %s\n", user_pass.username, user_pass.password);
    fwrite(&user_pass, sizeof(user_pass), 1, data);

    return 1;
}

int safely_send(int fd, void *buf) {
    struct packet *packet = (struct packet *) buf;
	size_t need = sizeof(struct packet) + packet->data_len;
    packet->data_len = htonl(packet->data_len);
	size_t sent = 0;
	int incr;
	while (sent < need) {
		incr = send(fd, buf + sent, need - sent, 0);
		if (incr <= 0) {
			return -1;
		}
		sent += incr;
	}
	return sent;
}

int safely_recv(int fd, void *buf) { 
    
    if (recv(fd, buf, (sizeof(struct packet)), 0) != (sizeof(struct packet))) {
        return -1;
    }
    
    struct packet *the_packet = (struct packet *) buf;
    the_packet->data_len = (ntohl(the_packet->data_len));
    size_t need = the_packet->data_len + sizeof(struct packet);
    size_t got = sizeof(struct packet);

    printf("Got packet with data_len: %d\n", sizeof(struct packet), the_packet->data_len);
    int incr;
    while (got < need) {
        incr = recv(fd, buf + got, need - got, 0);
        if (incr <= 0) {
            return -1;
        }
        got += incr;
    }
    return got;
}

int remove_user_with_sockfd(int fd) {
    struct User *loop_user = Find_User_Struct(fd);
    if (!loop_user) {
        return -1;
    } else {    
        if (loop_user->name) {
            free(loop_user->name);
        }
        if (loop_user->password) {
            free(loop_user->password);
        }
        loop_user->prev->next = loop_user->next;
        loop_user->next->prev = loop_user->prev;
        free(loop_user);
        printf("User: %d disconnected!\n", fd);
        fflush(stdout);
        return 1;
    }
  
}

int send_ack(char *str, int fd) {
    struct packet *send_me = malloc(sizeof(struct packet) + strlen(str) + 1);
    send_me->data_len = strlen(str) + 1;
    send_me->action = NET_ACK;
    strcpy(send_me->buf, str);
    safely_send(fd, send_me);
    free(send_me);
}

int send_nack(char *err, int fd) {
    struct packet *send_me = malloc(sizeof(struct packet) + strlen(err) + 1);
    send_me->data_len = strlen(err) + 1;
    send_me->action = NET_NACK;
    strcpy(send_me->buf, err);
    safely_send(fd, send_me);
    free(send_me);
}

void sort_pass_tuple_list(struct user_pass_tuple *list, uint32_t len) {
    int i;
    int j;

    struct user_pass_tuple temp;
    int min_idx;
    for (i = 0; i < len; i++) {
        min_idx = i;
        for (j = i; j < len; j++) {
            if (list[j].balance < list[min_idx].balance) {
                min_idx = j;
            }
        }
        memcpy(&temp, list + i, sizeof(struct user_pass_tuple));
        memcpy(list + i, list + min_idx, (sizeof(struct user_pass_tuple)));
        memcpy(list + min_idx, &temp, (sizeof(struct user_pass_tuple)));
    }
    
}

int remove_order(struct User* buyer, char *item_name) {
    uint32_t num_orders;
    fseek(orders, 0, SEEK_SET);
    fread(&num_orders, sizeof(num_orders), 1, orders);

    uint32_t i;
    uint8_t curr_name_len, curr_item_len;

    char *temp_name;
    char *temp_item_name;
    void *temp_buf;
    uint32_t curr_ammount;

    int pos1, pos2, file_size;
    fseek(orders, 4, SEEK_SET);
    for (i = 0; i < num_orders; i++) {
        pos1 = ftell(orders);
        fread(&curr_name_len, sizeof(curr_name_len), 1, orders);
        temp_name = malloc(curr_name_len + 1);
        fread(temp_name, curr_name_len, 1, orders);
        temp_name[curr_name_len] = '\0';

        fread(&curr_item_len, sizeof(curr_item_len), 1, orders);
        temp_item_name = malloc(curr_item_len + 1);
        fread(temp_item_name, curr_item_len, 1, orders);
        temp_item_name[curr_item_len] = '\0';

        pos2 = ftell(orders);
        fread(&curr_ammount, sizeof(curr_ammount), 1, orders);

        
        if (strcmp(temp_name, buyer->name) == 0 && strcmp(temp_item_name, item_name) == 0) {
            fseek(orders, 0, SEEK_END);
            file_size = ftell(orders);
            curr_ammount--;

            if (curr_ammount == 0) {
                fseek(orders, pos2 + sizeof(curr_ammount), SEEK_SET);
                temp_buf = malloc(file_size - pos2 - sizeof(curr_ammount));
                fread(temp_buf, file_size - pos2 - sizeof(curr_ammount), 1, orders);
                fseek(orders, pos1, SEEK_SET);
                fwrite(temp_buf, file_size - pos2 - sizeof(curr_ammount), 1, orders);
                free(temp_buf);

                num_orders--;
                fseek(orders, 0, SEEK_SET);
                fwrite(&num_orders, sizeof(num_orders), 1, orders);
                
                free(temp_buf);
            } else {
                fseek(orders, pos2, SEEK_SET);
                fwrite(&curr_ammount, sizeof(curr_ammount), 1, orders);
            }

            free(temp_name);
            free(temp_item_name);
            return 1;
        }

        free(temp_name);
        free(temp_item_name);
    }
    return 0;
}

int check_item_subtract_funds(struct User* buyer, char *item_name) {
    uint32_t num_items;
    fseek(shop, 0, SEEK_SET);
    fread(&num_items, sizeof(num_items), 1, shop);

    uint32_t i;
    int curr_price;
    uint8_t curr_len;
    int temp_bal;
    char *curr_item_name;
    for (i = 0; i < num_items; i++) {
        fread(&curr_len, sizeof(curr_len), 1, shop);
        curr_item_name = malloc(curr_len - 3);
        fread(curr_item_name, curr_len - sizeof(curr_price), 1, shop);
        curr_item_name[curr_len - 4] = '\0';
        fread(&curr_price, sizeof(curr_price), 1, shop);

        if (strcmp(item_name, curr_item_name) == 0) {
            free(curr_item_name);
            temp_bal = get_bal(buyer->name);
            if (temp_bal < 0) {
                /* Username not found */
                return -2;
            } 

            if (temp_bal >= curr_price) {
                
                add_bal(buyer->name, -1 * curr_price, 0);
                /* return vale of 1 indicates success */
                return 1;
            } else {
                /* return value of -1 indicates not enough funds */
                return -1;
            }
        }
        free(curr_item_name);
    }
    
    /* return value of zero indicates item not found */
    return 0;
}

int add_order(struct User* buyer, char *item_name) {
    

    int ret_val = check_item_subtract_funds(buyer, item_name);

    if (ret_val != 1) {
        return ret_val;
    }
    

    uint32_t num_orders;
    fseek(orders, 0, SEEK_SET);
    fread(&num_orders, sizeof(num_orders), 1, orders);

    uint32_t i;
    uint8_t curr_name_len, curr_item_len;

    char *temp_name;
    char *temp_item_name;
    uint32_t curr_ammount;
    int pos;
    fseek(orders, 4, SEEK_SET);
    
    for (i = 0; i < num_orders; i++) {
        fread(&curr_name_len, sizeof(curr_name_len), 1, orders);
        temp_name = malloc(curr_name_len + 1);
        fread(temp_name, curr_name_len, 1, orders);
        temp_name[curr_name_len] = '\0';

        fread(&curr_item_len, sizeof(curr_item_len), 1, orders);
        temp_item_name = malloc(curr_item_len + 1);
        fread(temp_item_name, curr_item_len, 1, orders);
        temp_item_name[curr_item_len] = '\0';

        pos = ftell(orders);
        fread(&curr_ammount, sizeof(curr_ammount), 1, orders);
        if (strcmp(temp_name, buyer->name) == 0 && strcmp(temp_item_name, item_name) == 0) {
            
            curr_ammount++;

            fseek(orders, pos, SEEK_SET);
            fwrite(&curr_ammount, sizeof(curr_ammount), 1, orders);

            free(temp_name);
            free(temp_item_name);
            return 1;
        }

        free(temp_name);
        free(temp_item_name);
    }
    
    curr_name_len = strlen(buyer->name);
    curr_item_len = strlen(item_name);

    fwrite(&curr_name_len, sizeof(curr_name_len), 1, orders);
    fwrite(buyer->name, strlen(buyer->name), 1, orders);

    fwrite(&curr_item_len, sizeof(curr_item_len), 1, orders);
    fwrite(item_name, strlen(item_name), 1, orders);

    curr_ammount = 1;
    fwrite(&curr_ammount, sizeof(curr_ammount), 1, orders);

    num_orders++;
    fseek(orders, 0, SEEK_SET);
    fwrite(&num_orders, sizeof(num_orders), 1, orders);

    return 1;
}

int remove_item_from_shop(char *item_name) {
    
    uint32_t num_items;
    fseek(shop, 0, SEEK_SET);
    fread(&num_items, sizeof(num_items), 1, shop);
    printf("%d\n", num_items);
    fflush(stdout);
    uint32_t i, j, size, sweep = sizeof(num_items);
    uint8_t curr_len;

    char *temp_str;
    for (i = 0; i < num_items; i++) {
        fread(&curr_len, sizeof(curr_len), 1, shop);
        temp_str = malloc(curr_len - 3);
        printf("LEN: %d\n", curr_len);
        fflush(stdout);
        fread(temp_str, curr_len - 4, 1, shop);
        temp_str[curr_len - 4] = '\0';
        
        fseek(shop, 4, SEEK_CUR);
        if (strcmp(temp_str, item_name) == 0) {
            fseek(shop, 0, SEEK_END);
            size = ftell(shop);
            printf("Size: %d\n", size);
            printf("Sweep: %d\n", sweep);
            printf("Curr Len: %d\n", curr_len);
            free(temp_str);
            temp_str = malloc(size - sweep - curr_len - sizeof(curr_len));
            
            fseek(shop, sweep + sizeof(curr_len) + curr_len, SEEK_SET);
            fread(temp_str, size - sweep - curr_len - sizeof(curr_len), 1, shop);

            fseek(shop, sweep, SEEK_SET);
            
            fwrite(temp_str, size - sweep - curr_len - sizeof(curr_len), 1, shop);
           
            free(temp_str);
            
            num_items--;
            fseek(shop, 0, SEEK_SET);
            fwrite(&num_items, sizeof(num_items), 1, shop);


            return 1;
        }
        free(temp_str);
        sweep += curr_len + sizeof(curr_len);
    }

    
   
    return 0;
}



void add_item_to_shop(char *item_name, int item_price) {
    
    uint32_t num_items;
    fseek(shop, 0, SEEK_SET);
    fread(&num_items, sizeof(num_items), 1, shop);

    printf("Trying to add item with num items: %d and name: %s and price: %d\n", num_items, item_name, item_price);
    uint32_t i;
    uint8_t curr_len;
    fseek(shop, 4, SEEK_SET);
    for (i = 0; i < num_items; i++) {
        fread(&curr_len, sizeof(curr_len), 1, shop);
        fseek(shop, curr_len, SEEK_CUR);
    }
    printf("File Pos: %d\n", ftell(shop));
    curr_len = strlen(item_name) + sizeof(int);
    fwrite(&curr_len, sizeof(curr_len), 1, shop);
    fwrite(item_name, strlen(item_name), 1, shop);
    fwrite(&item_price, sizeof(item_price), 1, shop);


    num_items++;
    fseek(shop, 0, SEEK_SET);
    fwrite(&num_items, sizeof(num_items), 1, shop);
   
}

int load_shop(char *load_me) {
    
    uint32_t num_items;
    fseek(shop, 0, SEEK_SET);
    fread(&num_items, sizeof(num_items), 1, shop);
    int sweep = 0;
    char *header = "Shop Items:\n";
    sprintf(load_me, header);
    sweep += strlen(header);
   
    uint32_t i;
    uint8_t curr_len;
    char *curr_name;
    int curr_price;

    int prev_len;

    printf("NUM ITEMS: %d\n", num_items);
    for (i = 0; i < num_items; i++) {
        printf("Count read: %d\n", fread(&curr_len, sizeof(curr_len), 1, shop));
        printf("Len: %d\n", curr_len);
        curr_name = malloc(curr_len - 4 + 1);
        
        printf("name count: %d\n", fread(curr_name, curr_len - 4, 1, shop));
        
        curr_name[curr_len - 4] = '\0';

       

        printf("NAME: %s\n", curr_name);

        printf("PRICE COUNT:%d\n", fread(&curr_price, sizeof(curr_price), 1, shop));
        printf("PRICE: %d\n", curr_price);
        int prev_len = strlen(load_me);
        sprintf(load_me + sweep, "| %s, J$ %d\n", curr_name, curr_price);
        sweep += strlen(load_me) - prev_len;
    }
    return 1;
}

int load_leaderboard(char *load_me) {
    /* Read superblock */
    
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   
    struct user_pass_tuple *user_bal_list = malloc(num_accounts * sizeof(struct user_pass_tuple));

    int i;
   
    for (i = 0; i < num_accounts; i++) {
        
        fread((user_bal_list + i), sizeof(struct user_pass_tuple), 1, data);
        printf("read: %s\n", user_bal_list[i].username);
    }
    int sweep = 0;
    sort_pass_tuple_list(user_bal_list, num_accounts);
    
    sprintf(load_me, "Jonathan Dollar Leaderboards:\n");
    sweep += strlen("Jonathan Dollar Leaderboards:\n");

    int rank = 1;

    struct user_pass_tuple *curr_user;
    
    char pad[MAX_PASSWORD_LENGTH];
    for (i = num_accounts - 1; i >= 0; i--) {
        curr_user = user_bal_list + i;

        
        switch (rank) {
            case 1:

                sprintf(pad, "| * * * [%d: %s], J$ %d\n", rank, curr_user->username, curr_user->balance);
                strcpy(load_me + sweep, pad);
                sweep += strlen(pad);
            break;

            case 2:
                sprintf(pad, "|  * *  [%d: %s], J$ %d\n", rank, curr_user->username, curr_user->balance);
                strcpy(load_me + sweep, pad);
                sweep += strlen(pad);
            break;

            case 3:
                sprintf(pad, "|   *   [%d: %s], J$ %d\n", rank, curr_user->username, curr_user->balance);
                strcpy(load_me + sweep, pad);
                sweep += strlen(pad);
            break;

            default:
                sprintf(pad, "|   -   [%d: %s], J$ %d\n", rank, curr_user->username, curr_user->balance);
                strcpy(load_me + sweep, pad);
                sweep += strlen(pad);
            break;
        }
        rank++; 
    }
    return 1;
}   

int is_allowed(struct User *him, int action) {
    if (him->is_admin) {
        return 1;
    } else if (action == NET_CLEAR || action == NET_ADD_BAL || action == NET_SUB_BAL || action == NET_SET_BAL || action == NET_ADD_TO_SHOP || action == NET_REMOVE_FROM_SHOP) {
        return 0;
    }

    return 1;
}

int username_taken(char *name) {
    /* Read superblock */
    
    char buf[4];
    uint32_t num_accounts;
    fseek(data, 0, SEEK_SET);


    fread(&num_accounts, sizeof(num_accounts), 1, data);
   


    uint32_t i;
  
   
    struct user_pass_tuple user_pass;
    for (i = 0; i < num_accounts; i++) {
   
        
        fread(&user_pass, sizeof(struct user_pass_tuple), 1, data);
        if (strcmp(name, user_pass.username) == 0) {
           
            return 1;
        } 
    }
    
    return 0;
}

void load_orders(struct User* him, char *load_me) {
    uint32_t num_orders;
    fseek(orders, 0, SEEK_SET);
    fread(&num_orders, sizeof(num_orders), 1, orders);

    uint32_t i;
    uint8_t curr_name_len, curr_item_len;

    char *temp_name;
    char *temp_item_name;
    uint32_t curr_ammount;
    int pos;
    fseek(orders, 4, SEEK_SET);
    
    int sweep = strlen("Your Orders:\n");
    strcpy(load_me, "Your Orders:\n");
    
    for (i = 0; i < num_orders; i++) {
        fread(&curr_name_len, sizeof(curr_name_len), 1, orders);
        temp_name = malloc(curr_name_len + 1);
        fread(temp_name, curr_name_len, 1, orders);
        temp_name[curr_name_len] = '\0';

        fread(&curr_item_len, sizeof(curr_item_len), 1, orders);
        temp_item_name = malloc(curr_item_len + 1);
        fread(temp_item_name, curr_item_len, 1, orders);
        temp_item_name[curr_item_len] = '\0';

        fread(&curr_ammount, sizeof(curr_ammount), 1, orders);
        if (strcmp(him->name, temp_name) == 0) {
            sprintf(load_me + sweep, "| %s, x%d\n", temp_item_name, curr_ammount);
            sweep += (strlen(load_me) - sweep);
        }

        free(temp_name);
        free(temp_item_name);
    }
}

void handle_packet(struct packet *packet, int fd) {
    struct User *him = Find_User_Struct(fd);
    struct user_pass_tuple load_me;
    char *err;
    int ret_num;
    if (is_allowed(him, packet->action)) {
        switch (packet->action) {
            case NET_CLEAR:
               
                uint32_t o = 0;
                fseek(data, 0, SEEK_SET);
                fwrite(&o, sizeof(o), 1, data);
                printf("Clearing File\n"); 
                send_ack("File cleared.\n", fd);
            break;

            case NET_CREATE:
                printf("Len: %d\n", strlen(packet->buf));
                packet->buf[MAX_USERNAME_LENGTH] = '\0';
                if (username_taken((char *) packet->buf)) {
                    send_nack("Username taken.\n", fd);
                } else if (!him->logged_in && !him->name && strlen(packet->buf) > 0) {
                    him->name = malloc(strlen(packet->buf) + 1);
                    strcpy(him->name, packet->buf);
                    send_ack("Create Password:\n", fd);
                
                } else {
                    send_nack("Invalid name or already logged in.\n", fd);
                }
            
            break;

            case NET_LOGIN:
                packet->buf[MAX_USERNAME_LENGTH] = '\0';
                char temp_password[MAX_PASSWORD_LENGTH];
                if (!him->logged_in && !him->name && strlen(packet->buf) > 0 && Find_User(packet->buf, temp_password)) {
                    him->name = malloc(strlen(packet->buf) + 1);
                    strcpy(him->name, packet->buf);
                    him->password = malloc(strlen(temp_password) + 1);
                    strcpy(him->password, temp_password);
                
                    send_ack("Enter Password:\n", fd);
                
                } else {
                
                    send_nack("Nonexistent name or already logged in.\n", fd);
                }
                
            
                
            break;

            case NET_CREATE_PASSWORD:
                packet->buf[MAX_PASSWORD_LENGTH] = '\0';
                if (strlen(packet->buf) > 0) {
                    char *str = malloc(strlen(him->name) + strlen(packet->buf) + strlen("Welcome: , Your password is: \n"));
                    sprintf(str, "Welcome: %s, Your password is: %s\n", (char *) him->name, (char *) packet->buf);
                    Create_User(him->name, packet->buf);
                    him->logged_in = 1;
                    
                    send_ack(str, fd);
                    free(str);
                } else {
                    if (him->name) {
                        free(him->name);
                        him->name = NULL;
                    }
                    if (him->password) {
                        free(him->password);
                        him->password = NULL;
                    }
                    send_nack("Bad password.\n", fd);
                }
            break;

            case NET_ENTER_PASSWORD:
                packet->buf[MAX_PASSWORD_LENGTH] = '\0';
                if (!him->logged_in && strcmp(him->password, packet->buf) == 0) {
                    char *str = malloc(strlen(him->name) + strlen("Logged in as: \n"));
                    sprintf(str, "Logged in as: %s\n", (char *)him->name);
                    send_ack(str, fd);
                    free(str);   

                    him->logged_in = 1;
                    load_user_data(him);
                } else {
                    if (him->name) {
                        free(him->name);
                        him->name = NULL;
                    }
                    if (him->password) {
                        free(him->password);
                        him->password = NULL;
                    }
                    send_nack("Wrong password.\n", fd);
                }
            break;

            case NET_LOGOUT:
                if (him->logged_in) {
                    him->logged_in = 0;
                    if (him->name) {
                        free(him->name);
                        him->name = NULL;
                    }
                    if (him->password) {
                        free(him->password);
                        him->password = NULL;
                    }
                    send_ack("Logged out.\n", fd);
                } else {
                    send_nack("You were not logged in.\n", fd);
                }
            break;

            case NET_GET_BAL:
                if (him->logged_in) {
                    char str[MAX_PACKET_SIZE];
                    
                    load_user_pass_tuple(him->name, &load_me);

                    sprintf(str, "Your balance is: %d Jonathan dollars.\n", load_me.balance);

                    send_ack(str, fd);
                } else {
                    send_nack("You have to be logged in.\n", fd);
                }
            break;  

            case NET_LEADERBOARD:
                if (him->logged_in) {
                    char str[MAX_PACKET_SIZE];
                    load_leaderboard(str);
                    send_ack(str, fd);
                } else {
                    send_nack("Log in or make an account to view leaderboards.\n", fd);
                }
            break;

            case NET_ADD_BAL:
                if (load_user_pass_tuple(packet->buf, &load_me)) {
                    him->net_flags[NET_ADD_BAL].set = 1;
                    him->net_flags[NET_ADD_BAL].buf = malloc(strlen((char *) packet->buf) + 1);
                    strcpy(him->net_flags[NET_ADD_BAL].buf, (char *) packet->buf);
                    send_ack("Enter ammount to add.\n", fd);
                } else {
                    send_nack("User not found.\n", fd);
                }
   
            break;

            case NET_SUB_BAL:
                if (load_user_pass_tuple(packet->buf, &load_me)) {
                    him->net_flags[NET_SUB_BAL].set = 1;
                    him->net_flags[NET_SUB_BAL].buf = malloc(strlen((char *) packet->buf) + 1);
                    strcpy(him->net_flags[NET_SUB_BAL].buf, (char *) packet->buf);
                    send_ack("Enter ammount to subtract.\n", fd);
                } else {
                    send_nack("User not found.\n", fd);
                }
            break;

            case NET_SET_BAL:
                if (load_user_pass_tuple(packet->buf, &load_me)) {
                    him->net_flags[NET_SET_BAL].set = 1;
                    him->net_flags[NET_SET_BAL].buf = malloc(strlen((char *) packet->buf) + 1);
                    strcpy(him->net_flags[NET_SET_BAL].buf, (char *) packet->buf);
                    send_ack("Enter ammount.\n", fd);
                } else {
                    send_nack("User not found.\n", fd);
                }
            break;

            case NET_ENTER_AMMOUNT:
                if (him->net_flags[NET_ADD_BAL].set) {
                    /* whom? */
                    add_bal(((char *) him->net_flags[NET_ADD_BAL].buf), ((int *) packet->buf)[0], 0);
                    him->net_flags[NET_ADD_BAL].set = 0;
                    free(him->net_flags[NET_ADD_BAL].buf);
                    send_ack("Funds given.\n", fd);
                } else if (him->net_flags[NET_SUB_BAL].set) {
                    add_bal(((char *) him->net_flags[NET_SUB_BAL].buf), -1*((int *) packet->buf)[0], 0);
                    him->net_flags[NET_SUB_BAL].set = 0;
                    free(him->net_flags[NET_SUB_BAL].buf);
                    send_ack("Funds taken.\n", fd);
                } else if (him->net_flags[NET_SET_BAL].set) {
                    add_bal(((char *) him->net_flags[NET_SET_BAL].buf), ((int *) packet->buf)[0], 1);
                    him->net_flags[NET_SET_BAL].set = 0;
                    free(him->net_flags[NET_SET_BAL].buf);
                    send_ack("Funds modified.\n", fd);
                } else if (him->net_flags[NET_ADD_TO_SHOP].set) {
                    add_item_to_shop(((char *) him->net_flags[NET_ADD_TO_SHOP].buf), ((int *) packet->buf)[0]);
                    him->net_flags[NET_ADD_TO_SHOP].set = 0;
                    free(him->net_flags[NET_ADD_TO_SHOP].buf);
                    send_ack("Item Added.\n", fd);
                } else {
                    send_nack("No flag set.\n", fd);
                }
            break; 

            case NET_GET_SHOP:
                
                char str[MAX_PACKET_SIZE];
                load_shop(str);
                send_ack(str, fd);
            break;

            case NET_ADD_TO_SHOP:
                him->net_flags[NET_ADD_TO_SHOP].set = 1;
                him->net_flags[NET_ADD_TO_SHOP].buf = malloc(strlen((char *) packet->buf) + 1);
                strcpy(him->net_flags[NET_ADD_TO_SHOP].buf, (char *) packet->buf);
                send_ack("Enter the price of this item.\n", fd);
            break;

            case NET_REMOVE_FROM_SHOP:
                if (remove_item_from_shop(packet->buf)) {
                    send_ack("Item removed.\n", fd);
                } else {
                    send_nack("Item does not exist.\n", fd);
                }
            break;

            case NET_BUY:
                if (him->logged_in) {
                    if (!him->net_flags[NET_BUY].set) {  
                        him->net_flags[NET_BUY].set = 1;
                        him->net_flags[NET_BUY].buf = malloc(strlen((char *) packet->buf) + 1);
                        strcpy(him->net_flags[NET_BUY].buf, (char *) packet->buf);
                        send_ack("Are you sure you want to buy this item? (yes/no).\n", fd);
                    } else {
                        him->net_flags[NET_BUY].set = 0;
                        ret_num = add_order(him, him->net_flags[NET_BUY].buf);
                        free(him->net_flags[NET_BUY].buf);
                        if (ret_num == 1) {
                           send_ack("Item successfully purchased.\n", fd);
                        } else if (ret_num == 0) {
                            send_nack("Item not found.\n", fd);
                        } else if (ret_num == -1) {
                            send_nack("Insufficient funds.\n", fd);
                        }
                        
                    }
                } else {
                    send_nack("You need to be logged in.\n", fd);
                }
            break;
            
            case NET_GET_ORDERS:
                if (him->logged_in) {
                    char order_str[10000];
                    load_orders(him, order_str);
                    send_ack(order_str, fd);
                } else {
                    send_nack("You need to be logged in.\n", fd);
                }
            break;
        }
    } else {
        send_nack("You need to be an admin to use this command.\n", fd);
    }
    
}

int main(/* int argc, char *argv[] */) {
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
     
   
    struct addrinfo *serv_addr;
    struct addrinfo hints; 

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, PORT, &hints, &serv_addr);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    SOCKET serv_sock = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol);

    if (bind(serv_sock, serv_addr->ai_addr, (int)serv_addr->ai_addrlen) != 0) {
        printf("Bind Error\n");
        fflush(stdout);
        exit(-1);
    }
    freeaddrinfo(serv_addr);

    if (listen(serv_sock, MAX_SOCK_QUEUE) != 0) {
        printf("Listen Error\n");
        fflush(stdout);
        exit(-1);
    }
    /* the user_list is a linked list to all currently connected users */
    user_list = malloc(sizeof(struct User));
    user_list->next = user_list;
    user_list->prev = user_list;

    fd_set select_me;

    struct User *loop_user;
    int nfds = 0;

    /* timeval struct for no cooldown */
    struct timeval zero_time;
	zero_time.tv_sec = 0;
	zero_time.tv_usec = 0;

    uint8_t rcv_buf[MAX_PACKET_SIZE];
    ssize_t num_recvd;

    char name_buf[MAX_USERNAME_LENGTH];
    char pass_buf[MAX_PASSWORD_LENGTH];
    

    /* Load in data from database first here? */
    data = fopen("jontan.data", "r+b");
    if (!data) {
        printf("couldnt open data file\n");
        return -1;
    }
    data_fd = fileno(data);

    /* Open shop file */
    shop = fopen("shop.data", "r+b");
    if (!shop) {
        printf("shop not found\n");
        return -1;
    }
    shop_fd = fileno(shop);
 
    /* Open shop file */
    orders = fopen("orders.data", "r+b");
    if (!orders) {
        printf("orders not found\n");
        return -1;
    }
    orders_fd = fileno(orders);

    struct packet *the_packet;

    while (1) {

        /* populate the fdset and find the max sockfd */
        FD_ZERO(&select_me);

        FD_SET(serv_sock, &select_me);
        nfds = serv_sock;
        loop_user = user_list->next;
        while (loop_user != user_list) {
            
            if (loop_user->sockfd > nfds) {
                nfds = loop_user->sockfd;
            }

            FD_SET(loop_user->sockfd, &select_me);
            loop_user = loop_user->next;
        }
     
        
        /* select on the sockets */
        if (select(nfds + 1, &select_me, NULL, NULL, &zero_time) > 0) {
           
            for (int the_fd = 0; the_fd < nfds + 1; the_fd++) {
				if (FD_ISSET(the_fd, &select_me)) { /* You got mail */
                    
                    if (the_fd == serv_sock) {
                        
                        struct sockaddr_in new_addr;
                        uint32_t socklen = sizeof(new_addr);
                        SOCKET newfd = accept(serv_sock, (struct sockaddr *) &new_addr, &socklen);

                        struct User *new_user = malloc(sizeof(struct User));
                        memset(new_user, 0, sizeof(struct User));
                        new_user->logged_in = 0;
                        new_user->sockfd = newfd;
                        
                        struct User *temp = user_list->next;
                        user_list->next = new_user;
                        new_user->next = temp;
                        new_user->prev = user_list;
                        temp->prev = new_user;

                        printf("New User Joined with fd: %d\n", newfd);
                        fflush(stdout);
                    } else {
                        num_recvd = safely_recv(the_fd, rcv_buf);
                        if (num_recvd == -1) {
                            
                            remove_user_with_sockfd(the_fd);
                           
                        } else {

                            handle_packet((struct packet *) rcv_buf, the_fd);
                     
                        }
                    }
                } 
            }
        }
    }

    return 0;
}