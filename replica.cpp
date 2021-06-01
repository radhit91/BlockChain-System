#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "replica.h"

#define ATTR_CNT 5

using namespace std;

void bzero(void *a, size_t n) {
	    memset(a, 0, n);
}
	
void bcopy(const void *src, void *dest, size_t n) {
    memmove(dest, src, n);
}

char* process_operation(char *input) {
    size_t n = strlen(input) * sizeof(char);
    char *output = (char*)malloc(n);
    memcpy(output, input, n);
    return output;
}

struct sockaddr_in* init_sockaddr_in(uint16_t port_number) {
   	struct sockaddr_in *socket_address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
   	memset(socket_address, 0, sizeof(*socket_address));
   	socket_address -> sin_family = AF_INET;
   	socket_address -> sin_addr.s_addr = htonl(INADDR_ANY);
   	socket_address -> sin_port = htons(port_number);
   	return socket_address;
}

void parse_message(int cli_fd, char* c_arr, string delimiter, Replica* replica){
	size_t pos = 0, cnt = 0;
	string s(c_arr);
	pos = s.find(delimiter);
	string token[ATTR_CNT]; 
	token[cnt] = s.substr(0, pos);
	printf("token: %s\n", token[cnt].c_str());
	cnt++;
	//cout << token << endl;
	//s.erase(0, pos + delimiter.length());
	s = s.substr(pos + delimiter.length());

	while ((pos = s.find(delimiter)) != string::npos) {
    	token[cnt] = s.substr(0, pos);
		printf("token: %s\n", token[cnt].c_str());
    	//cout << token << endl;
    	//s.erase(0, pos + delimiter.length());
		cnt++;
		s = s.substr(pos + delimiter.length());
	}
	printf("inside2 %s %s\n", s.c_str(), s.substr(0, pos).c_str());
	token[cnt] = s.substr(0, pos);
	printf("inside3\n");
	printf("token: %s\n", token[cnt].c_str());
	printf("inside\n");

	if(stoi(token[0]) == 0){
		if(replica->max_ballot < stoi(token[2])){
			replica->max_ballot = stoi(token[2]);
			if(replica->Replica::client_connection(stoi(token[1]), 1, replica)){
				printf("Unable to send promise message. Exiting...\n");
				exit(0);
			} 		
		}

	}
	else if(stoi(token[0]) == 1){
		if(stoi(token[3]) != -1 && stoi(token[4]) != -1){
			replica->last_prop_val = stoi(token[3]);
			replica->last_log_pos = stoi(token[4]);
			if(replica->Replica::client_connection(stoi(token[1]), 2, replica)){
				printf("Unable to send propose message. Exiting...\n");
				exit(0);
			} 
		}
		else{
			printf("-----------------------\n");
			printf("%d %d %d\n", replica->request_val, replica->request_pos, replica->max_ballot);
			replica->last_prop_val = replica->request_val;
			replica->last_log_pos = replica->request_pos;
			if(replica->Replica::client_connection(stoi(token[1]), 2, replica)){
				printf("Unable to send propose message. Exiting...\n");
				exit(0);
			} 
		}
	
	}
	else if(stoi(token[0]) == 2){
		replica->last_prop_val = stoi(token[3]);
		replica->last_log_pos = stoi(token[4]);
		if(replica->Replica::client_connection(stoi(token[1]), 3, replica)){
				printf("Unable to send accept message. Exiting...\n");
				exit(0);
		}
		replica->last_prop_val = -1;
		replica->last_log_pos = -1;
	}
	else if(stoi(token[0]) == 3){
		if(replica->Replica::client_connection(4, 4, replica)){
				printf("Unable to send client response message. Exiting...\n");
				exit(0);
		}
		if(replica->last_prop_val == stoi(token[3]) && replica->last_log_pos == stoi(token[4])){
			replica->last_prop_val = -1;
			replica->last_log_pos = -1;
		}
		else{
			replica->last_prop_val = replica->request_val;
			replica->last_log_pos = replica->request_pos;
		}

	}
	else if(stoi(token[0]) == 4){
		replica->max_ballot = replica->max_ballot + 1;
		replica->request_val = stoi(token[3]);
		replica->request_pos = stoi(token[4]);
		printf("%d %d\n", replica->request_val, replica->request_pos);
		//replica->last_prop_val = replica->request_val;
		//replica->last_log_pos = replica->request_pos;
		replica->is_leader = true;
		int server1, server2;
		if(replica->replica_name == 1){
			server1 = 2;
			server2 = 3;
		}
		else if(replica->replica_name == 2){
			server1 = 3;
			server2 = 1;
		}
		else if(replica->replica_name == 3){
			server1 = 1;
			server2 = 2;
		}
		else{
			printf("Invalid Replica Name. Exiting...\n");
			exit(0);
		}	
			
		if(replica->Replica::client_connection(server1, 0, replica)){
			if(replica->Replica::client_connection(server2, 0, replica)){
				printf("Unable to send prepare message to other replicas. Exiting...\n");
				exit(0);
			}
		}
		printf("%d %d\n", replica->request_val, replica->request_pos);
	}
	else{
		printf("Invalid Message Type. Exiting...\n");
		exit(0);
	}	
	
	
	/*printf("token: %s, delimiter: %s, c_arr: %s, string: %s\n", token[cnt].c_str(), delimiter.c_str(), c_arr, s.c_str());
	char* c = new char[token.length() + 1];
	strcpy(c, token.c_str());
	replica->Replica::client_connection(port); */
}

int main(int argc, char** argv){
	
	Replica replica(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	//cout << replica.timer << endl;
	string delimiter = "x";

	/*if(atoi(argv[6]) == 1){
		replica.Replica::client_connection(atoi(argv[6]));
	}
	else if(atoi(argv[6]) == 0){
		//replica.Replica::client_connection(atoi(argv[6]));*/
	

	
	uint16_t port_number;
	switch(replica.replica_name){
		case 1: {
			port_number = replica.r1_port_no;
			break;
		}
		case 2: {
			port_number = replica.r2_port_no;
			break;
		}
		case 3: {
			port_number = replica.r3_port_no;
			break;
		}
		default: 
			break;		

	}

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in *server_sockaddr = init_sockaddr_in(port_number);
    struct sockaddr_in *client_sockaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    socklen_t server_socklen = sizeof(*server_sockaddr);
    socklen_t client_socklen = sizeof(*client_sockaddr);


    if (bind(server_fd, (const struct sockaddr *) server_sockaddr, server_socklen) < 0)
    {
        printf("Error! Bind has failed\n");
        exit(0);
    }
    if (listen(server_fd, 3) < 0)
    {
        printf("Error! Can't listen\n");
        exit(0);
    }


    const size_t buffer_len = 256;
    char *buffer = (char *)malloc(buffer_len * sizeof(char));
    char *response = NULL;
    time_t last_operation;
    __pid_t pid = -1;

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@%d %d\n", replica.request_val, replica.request_pos);
    while (1) {
		printf("#############################%d %d\n", replica.request_val, replica.request_pos);
        int client_fd = accept(server_fd, (struct sockaddr *) &client_sockaddr, &client_socklen);

        pid = fork();

        if (pid == 0) {
            close(server_fd);

            if (client_fd == -1) {
                exit(0);
            }

            printf("Connection with `%d` has been established and delegated to the process %d.\nWaiting for a query...\n", client_fd, getpid());
			printf("before1---\n");
            last_operation = clock();
			printf("before2---\n");

            while (1) {
				printf("before");
				printf("%d %d\n", replica.request_val, replica.request_pos);
				//replica.Replica::receive_message(client_fd, buffer, buffer_len);
                read(client_fd, buffer, buffer_len);

                if (buffer == "close") {
                    printf("Process %d: ", getpid());
                    close(client_fd);
                    printf("Closing session with `%d`. Bye!\n", client_fd);
                    break;
                }

                if (strlen(buffer) == 0) {
                    clock_t d = clock() - last_operation;
                    double dif = 1.0 * d / CLOCKS_PER_SEC;

                    if (dif > 5.0) {
                        printf("Process %d: ", getpid());
                        close(client_fd);
                        printf("Connection timed out after %.3lf seconds. ", dif);
                        printf("Closing session with `%d`. Bye!\n", client_fd);
                        break;
                    }

                    continue;
                }

                printf("Process %d: ", getpid());
                printf("Received `%s`. Processing... ", buffer);

                free(response);
                response = process_operation(buffer);
				bzero(buffer, buffer_len * sizeof(char));

				//std::string s = "scott>=tiger>=mushroom";
				//std::string delimiter = ">=";

				
				parse_message(client_fd, response, delimiter, &replica);
				printf("%d %d\n", replica.request_val, replica.request_pos);
				printf("after");

                //send(client_fd, response, strlen(response), 0);
                //printf("Responded with `%s`. Waiting for a new query...\n", response);

                last_operation = clock();
				break;
            }
            exit(0);
        }
        else {
            close(client_fd);
        }
		//break;
    }
	




	
	/*while(replica.Replica::receive_message()){ //buffer not empty 
		
		//Start when a client request is received; set as "true" for testing
		//function: receive_from_client()
		//last_prop_val & log_pos value in the replica object updated by this function
		//if(receive_message()){ //from client
			//Send prepare messages to the other two replicas
			//function: send_message(parameters)
			//
		if(replica.msg_type == 4){ //from client
			replica.Replica::send_message(0, replica.max_ballot, -1, -1); //to replica1
			replica.Replica::send_message(0, replica.max_ballot, -1, -1); //to replica2
		}
		else if(replica.msg_type == 0){ //prepare message from another replica
			if(replica.ignore_msg == false){
				replica.Replica::send_message(1, replica.max_ballot, replica.last_prop_val, replica.last_log_pos); //to M
			}
			else{
				replica.ignore_msg = false;
			}
			
		}
		else if(replica.msg_type == 1){ //promise message from another replica
			if(replica.promise_msg_recvd == false){
				replica.promise_msg_recvd = true;
				replica.Replica::send_message(2, replica.max_ballot, replica.last_prop_val, replica.last_log_pos); //to fastest repllica		
			}
			else{
				replica.promise_msg_recvd = false;
			}
			
		}
		else if(replica.msg_type == 2){ //propose message from M
			replica.Replica::send_message(3, replica.max_ballot, replica.last_prop_val, replica.last_log_pos); //to M

		}
		else if(replica.msg_type == 3){ //accept message from another replica
			replica.Replica::send_message(4, replica.max_ballot, replica.last_prop_val, replica.last_log_pos); //to client							
		}
			
		/*cout << "inside" << replica.timer << endl;
		//replica.start_timer(replica.timer);
		cout << replica.timer << endl;*/
		//exit(0);
		
	//}*/

} 
