/*
    C++ socket client, handles multiple clients using threads
    Compile
*/
 
#include<arpa/inet.h> //inet_addr
#include<pthread.h> //for threading , link with lpthread
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

int commandMenu(int);
void privateMessage(int);
void broadcastMessage(int);
void clientExit(int);
void printMessage(char *);

//the thread function
void *handle_messages(void*);
void check_password(void *);
char *username;
int quit = 0;  //set this to 1 when exiting to wrap up threads

int main(int argc , char *argv[])
{
    int SERVER_PORT;
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host;

    char buf[BUFSIZ];
    int s;
    int len;
    std::string response;
    if (argc==4) 
    {
        host = argv[1];
        SERVER_PORT = atoi(argv[2]);
        username = argv[3];
    }
    else 
    {
        fprintf(stderr, "usage: ./chatclient [SERVER NAME] [PORT] [USERNAME]\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) 
    {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(SERVER_PORT);
    /* active open */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("simplex-talk: socket"); exit(1);
    }
    
    printf("Welcome to the TCP client!\n");

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
      perror("simplex-talk: connect");
      close(s); exit(1);
    }

    pthread_t handler_thread;
    //thread for receiving incoming messages
    
    puts("Connected to server\n");
    //connection to server established, handle username/password
    check_password(&s);
         
    if( pthread_create( &handler_thread , NULL , handle_messages , (void*) &s) < 0)
    {
        perror("could not create thread");
        return 1;
    }

    bzero(buf, sizeof(buf));

    while(!quit)
    {
        commandMenu(s);
    }
    //Now join the thread , so that we dont terminate before the thread
    pthread_join( handler_thread , NULL);
    //puts("Handler assigned");
     
    return 0;
}

void check_password(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
     
    //Send username to server
    char buf[BUFSIZ];
    strcpy(buf, username);
    if (write(sock, buf, strlen(buf)) == -1){
        perror("Error sending username to server\n");
        exit(1);
    } 
    memset(buf, 0, sizeof(buf));
    if (read_size = recv(sock , buf, sizeof(buf) , 0) == -1){
        perror("Error receiving message from server\n");
        exit(1);

    }
    std::cout<<buf;
    memset(buf, 0, sizeof(buf));
    /* send password */
    fgets(buf, sizeof(buf), stdin);
    if (write(sock, buf, strlen(buf)) == -1){
        perror("Error sending password to server\n");
        exit(1);
    } 
    memset(buf, 0, sizeof(buf));
    if (read_size = recv(sock , buf, sizeof(buf) , 0) == -1){
        perror("Error receiving message from server\n");
        exit(1);

    }
    std::cout<<buf;
    std::string response = buf;
    memset(buf, 0, sizeof(buf));
    std::stringstream repo;
    repo << response;
    std::string check;
    repo >> check;
    while (check.compare("Welcome") != 0){ //wrong password
        fgets(buf, sizeof(buf), stdin);
        if (write(sock, buf, strlen(buf)) == -1){
            perror("Error sending username to server\n");
            exit(1);
        } 
        memset(buf, 0, sizeof(buf));
        if (read_size = recv(sock , buf, sizeof(buf) , 0) == -1){
            perror("Error receiving message from server\n");
            exit(1);
        }
        std::cout<<buf;
        std::stringstream repoTemp;
        check = "";
        repoTemp >> check;
    }
}

    
void *handle_messages(void *socket_desc) {
    //Receive a message from server
    char buf[BUFSIZ];
    int sock = *(int*)socket_desc;
    int r;

    while(1)
    {
        if(quit)
        {
  	    break;
        }
  
	bzero(buf, sizeof(buf));

	if(r = recv(sock, buf, sizeof(buf), 0) <= 0)
	{
	  perror("Error receiving message from server\n");
	  exit(1);
	}

      if (strcmp(buf, "CONF") != 0){  
        std::cout << std::endl << std::endl << buf <<std::endl;
        std::cout << "\nEnter P for private conversation\nEnter B for message broadcasting\nEnter E for Exit\n\n  >> ";
        fflush(stdout);
      }
      //for now we just print the message, later we may have to parse it
      memset(buf, 0, sizeof(buf));
  }
  
  
} 

int commandMenu(int sock) {
    // Display Options and Read Input
    std::cout << "Enter P for private conversation\nEnter B for message broadcasting\nEnter E for Exit\n\n";
    std::string command;

    // Enter Operation Function
    while (true) {
        if (command.compare("P") == 0) {
            privateMessage(sock);
        } else if (command.compare("B") == 0) {
            broadcastMessage(sock);
        } else if (command.compare("E") == 0) {
            clientExit(sock);
            return 1;
        } else {
            std::cout << "\nPlease enter one of the options\n";
        }
        std::cout << "Enter P for private conversation\nEnter B for message broadcasting\nEnter E for Exit\n\n  >> ";
        std::cin >> command;
    }

}


void privateMessage(int sock) {
    // Send operation to server
    char operation[BUFSIZ];
    sprintf(operation, "CP");
    write(sock, operation, strlen(operation));

    char username[BUFSIZ];
    memset(username, 0, sizeof(username));
    std::cout << "Enter Username >> ";
    fgets(username, sizeof(username), stdin);

    // Send username
    char username_msg[BUFSIZ];
    memset(username_msg, 0, sizeof(username_msg));
    username_msg[0] = 'C';
    for (int c = 0 ; c < strlen(username); c++) {
        if (username[c] == '\n') {
            username_msg[c+1] = '\0';
            break;
        } else {
            username_msg[c+1] = username[c];
        }
    }
    write(sock, username_msg, strlen(username_msg));

    // Send message to server
    char message[BUFSIZ];
    memset(message, 0, sizeof(message));
    std::cout << "Enter Private Message >> ";
    fgets(message, sizeof(message), stdin);

    char send_msg[BUFSIZ];
    send_msg[0] = 'C';
    for (int c = 0 ; c < strlen(message); c++) {
        if (message[c] == '\n') {
            send_msg[c+1] = '\0';
            break;
        } else {
            send_msg[c+1] = message[c];
        }
    }
    write(sock, send_msg, strlen(send_msg));
    std::cout << "Message Sent.\n";
}

void broadcastMessage(int sock) {
    // Send operation to server
    char buf[BUFSIZ];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "CB");
    write(sock, buf, strlen(buf));

    // Ask user for message
    std::cout << "Enter Broadcast Message >> ";
    std::string msg;
    getline(std::cin.ignore(), msg);
    // Send message
    memset(buf, 0, sizeof(buf));
    strcpy(buf, msg.c_str());
    write(sock, buf, strlen(buf));

}

void clientExit(int sock) {
    // send operation to server
    char buf[BUFSIZ];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "CE");
    quit = 1;
}

void printMessage(char * msg) {
    msg++;
    std::string message = msg;
    std::cout << message;
}
