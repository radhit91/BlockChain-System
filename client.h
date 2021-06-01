#include <chrono>
#include <ctime>
#include <cstdlib>

class Client{
	public:
	int request_val = -1;
	int request_pos = 0;
	int r1_port_no;
	int r2_port_no;
	int r3_port_no;
	int cli_port_no;
	bool is_client = true;
	int replica_name;
	
	Client(int replica_name, int r1_port, int r2_port, int r3_port, int cli_port){
		this->replica_name = replica_name;
		this->r1_port_no = r1_port;
		this->r2_port_no = r2_port;
		this->r3_port_no = r3_port;
		this->cli_port_no = cli_port;
	}

	int client_connection(int replica_name, Client client){

		int sockfd, portno, n;
    	struct sockaddr_in serv_addr;

    	struct hostent *server;

    	char buffer[256];
    	switch(replica_name){
			case 1: {
				portno = client.r1_port_no;
				break;
			}
			case 2: {
				portno = client.r2_port_no;
				break;
			}
			case 3: {
				portno = client.r3_port_no;
				break;
			}
			case 4: {
				portno = client.cli_port_no;
				break;
			}
			default: 
				break;		

		}

    	// create socket and get file descriptor
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	    server = gethostbyname("127.0.0.1");		

	    if (server == NULL) {
	        fprintf(stderr,"ERROR, no such host\n");
	        exit(0);
	    }
	
	    bzero((char *) &serv_addr, sizeof(serv_addr));
	    serv_addr.sin_family = AF_INET;
	    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	    serv_addr.sin_port = htons(portno);


	    // connect to server with server address which is set above (serv_addr)

	    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
	        perror("ERROR while connecting");
	        exit(1);
	    }

	    // inside this while loop, implement communicating with read/write or send/recv function
	    while (1) {
	        //printf("What do you want to say? ");
	        bzero(buffer,256);
	        //scanf("%s", buffer);
			std::string message;
			srand(client.request_pos);
			client.request_val = rand() % 10 + 1;
			//client.request_pos++;

			message = "4x" + std::to_string(client.replica_name) + "x-1" + "x" + std::to_string(client.request_val) + "x" + std::to_string(client.request_pos);
			strcpy(buffer, message.c_str());
			
 			
				
	
	        n = write(sockfd,buffer,strlen(buffer));
	
	        if (n < 0){
	            perror("ERROR while writing to socket");
	            exit(1);
	        }

	        bzero(buffer,256);
	        /*n = read(sockfd, buffer, 255);
	
	        if (n < 0){
	            perror("ERROR while reading from socket");
	            exit(1);
	        }*/
	        printf("server replied: %s \n", buffer);

	        // escape this loop, if the server sends message "quit"

	        if (!bcmp(buffer, "quit", 4))
	            break;
		
			break;
	    }
		close(sockfd);
	    return 0;		
	
	}

	void error(const char *msg)
	{
    	perror(msg);
    	exit(1);
	}

};
