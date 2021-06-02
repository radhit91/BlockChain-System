#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sclient.h"
#include <vector>
#include <unordered_map>

#define ATTR_CNT 5
#define CNT 10

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

void parse_message(int cli_fd, char* c_arr, string delimiter, Client* client){
	size_t pos = 0, cnt = 0;
	string s(c_arr);
	pos = s.find(delimiter);
	string token[ATTR_CNT]; 
	token[cnt] = s.substr(0, pos);
	//if(stoi(token[cnt]) == 5)
		//printf("Verfication Request Received. Sending result to node: %d\n", );
	//printf("token: %s\n", token[cnt].c_str());
	cnt++;
	//cout << token << endl;
	//s.erase(0, pos + delimiter.length());
	s = s.substr(pos + delimiter.length());

	while ((pos = s.find(delimiter)) != string::npos) {
    	token[cnt] = s.substr(0, pos);
		//printf("token: %s\n", token[cnt].c_str());
    	//cout << token << endl;
    	//s.erase(0, pos + delimiter.length());
		cnt++;
		s = s.substr(pos + delimiter.length());
	}
	token[cnt] = s;
	if(stoi(token[0]) == 5)
		printf("Verfication Request Received. Sending result to node: %d\n", stoi(token[cnt]));
	else
		printf("Client Response(val = %d, pos = %d) received from replica %d\n", stoi(token[3]), stoi(token[4]), stoi(token[1]));
	//printf("token: %s\n", token[cnt].c_str());
}

int main(int argc, char** argv){
	
	Client client(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	//cout << replica.timer << endl;
	string delimiter = "x";
	//vector<int> secret_key;
	unordered_map<int, int> hash_map;

	/*if(atoi(argv[6]) == 1){
		replica.Replica::client_connection(atoi(argv[6]));
	}
	else if(atoi(argv[6]) == 0){
		//replica.Replica::client_connection(atoi(argv[6]));*/
	
	int pos = 0;
	int secret_key = 0;
	
	uint16_t port_number = client.cli_port_no;

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

	int cnt = CNT;

    while (1) {
	
		pos++;
		secret_key = 7*pos + 3*cnt + 5;

		if(cnt){
			client.secret_key = secret_key;
			client.request_pos = pos;
			client.max_ballot = pos + ATTR_CNT;
			srand(cnt+ATTR_CNT);
			int replica_no = rand() % 3 + 1;
			client.Client::client_connection(replica_no, client);
			hash_map[replica_no] = secret_key;
			//printf("Done: %d\n", cnt);
		}

		cnt--;

        int client_fd = accept(server_fd, (struct sockaddr *) &client_sockaddr, &client_socklen);

        pid = fork();

        if (pid == 0) {
            close(server_fd);
			client.request_pos = pos;
			//printf("Position: %d\n", client.request_pos);

            if (client_fd == -1) {
                exit(0);
            }

            //printf("Connection with `%d` has been established and delegated to the process %d.\nWaiting for a query...\n", client_fd, getpid());
			//printf("before1---\n");
            last_operation = clock();
			//printf("before2---\n");

            while (1) {
				//printf("before");
				
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

                //printf("Process %d: ", getpid());
                //printf("Received `%s`. Operation Succeeded\n", buffer);

                free(response);
                response = process_operation(buffer);
				bzero(buffer, buffer_len * sizeof(char));

				//std::string s = "scott>=tiger>=mushroom";
				//std::string delimiter = ">=";

				//printf("Position: %d\n", client.request_pos);
				parse_message(client_fd, response, delimiter, &client);
				//printf("after");
				//printf("Position: %d\n", client.request_pos);

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
}
