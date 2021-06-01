#include <chrono>
#include <ctime>
#define NANO_SECS 1

class Replica{
	
	public:

	//int timer = 0;
	//char* ip_addr;
	int r1_port_no;
	int r2_port_no;
	int r3_port_no;
	int cli_port_no;
	bool is_leader = false;
	int replica_name;
	int max_ballot = 0;
	int last_prop_val = -1;
	int last_log_pos = -1;
	int request_val = -1;
	int request_pos = -1;
	int msg_type = -1;
	bool ignore_msg = false;
	bool promise_msg_recvd = false;
	//int prop_val = 0;
	//int prop_pos = 0;

	Replica(int replica_name, int r1_port, int r2_port, int r3_port, int cli_port){
		this->replica_name = replica_name;
		this->r1_port_no = r1_port;
		this->r2_port_no = r2_port;
		this->r3_port_no = r3_port;
		this->cli_port_no = cli_port;
	}

	bool start_timer(int count){
		bool timer_success = false;
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();
		auto start_duration = start.time_since_epoch();
		auto start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(start_duration).count();
		auto end_ms = start_ms;
			
		while(end_ms < count*1000 + start_ms){
			end = std::chrono::system_clock::now();
			auto end_duration = end.time_since_epoch();
			end_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_duration).count(); 
		}
		std::cout << "in" << std::endl;
		timer_success = true;
		return timer_success;
	}

	bool send_message(int msg_type, int ballot, int value, int position){
		//Send different types of messages to either client or other replicas
		/* msg_type:
		0 - prepare
		1 - promise
		2 - propose
		3 - accept
		4 - client response
		*/
		//return "true" if message was sent successfully; else return "false"
		return true;
	}

	bool receive_message(){
		//Receive different types of messages from either client or other replicas and parse them according to their message types
		/* msg_type:
		0 - prepare
		1 - promise
		2 - propose
		3 - accept
		4 - client request
		*/

		//return "true" if message was received & relevant data variables/structures updated successfully; else return "false"
		return true;
	}

	int client_connection(int replica_name, int msg_type, Replica* replica){

		int sockfd, portno, n;
    	struct sockaddr_in serv_addr;

    	struct hostent *server;

    	char buffer[256];
    	switch(replica_name){
			case 1: {
				portno = replica->r1_port_no;
				break;
			}
			case 2: {
				portno = replica->r2_port_no;
				break;
			}
			case 3: {
				portno = replica->r3_port_no;
				break;
			}
			case 4: {
				portno = replica->cli_port_no;
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
			switch(msg_type){
				case 0: {
					message = std::to_string(msg_type) + "x" + std::to_string(replica->replica_name) + "x" + std::to_string(replica->max_ballot) + "x-1x-1";
					strcpy(buffer, message.c_str());
					break;
 				}
				case 1: {
					message = std::to_string(msg_type) + "x" + std::to_string(replica->replica_name) + "x" + std::to_string(replica->max_ballot) + "x" + std::to_string(replica->last_prop_val) + "x" + std::to_string(replica->last_log_pos);
					strcpy(buffer, message.c_str());
					break;
 				}
				case 2: {
					message = std::to_string(msg_type) + "x" + std::to_string(replica->replica_name) + "x" + std::to_string(replica->max_ballot) + "x" + std::to_string(replica->last_prop_val) + "x" + std::to_string(replica->last_log_pos);
					strcpy(buffer, message.c_str());
					break;
 				}
				case 3: {
					message = std::to_string(msg_type) + "x" + std::to_string(replica->replica_name) + "x" + std::to_string(replica->max_ballot) + "x" + std::to_string(replica->last_prop_val) + "x" + std::to_string(replica->last_log_pos);
					strcpy(buffer, message.c_str());
					break;
 				}
				case 4: {
					message = std::to_string(msg_type) + "x" + std::to_string(replica->replica_name) + "x-1x" + std::to_string(replica->last_prop_val) + "x" + std::to_string(replica->last_log_pos);
					strcpy(buffer, message.c_str());
					break;
 				}
				default:
					break;


			}
	
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

	int server_connection(const uint16_t port_no){
	

	}
	
	void error(const char *msg)
	{
    	perror(msg);
    	exit(1);
	}	


};
