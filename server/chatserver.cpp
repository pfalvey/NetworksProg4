/*
    C++ socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include<fstream>   //reading files
#include<sstream>   //parsing strings for passwords
#include<map>       
#include<string>
#include<iostream>
 
//the thread function
void *connection_handler(void *);
bool passwordsExist = false; //is true if a passwords.txt file already exists
std::map<std::string, std::string> passes; //map with username as key, password as value
 
int main(int argc , char *argv[])
{   
    /*   read in the passwords file   */
    std::fstream ifs;
    std::string filename = "passwords.txt";
    ifs.open(filename);
    if(ifs) {    //passwords.txt exists
        passwordsExist = true;
        std::string temp;
        while (!ifs.eof()){
            getline(ifs, temp);
            std::stringstream ss;
            ss << temp;
            std::string user, pass;
            ss >> user >> pass;
            if (user.length() > 0 && pass.length() > 0)
                passes.insert(std::pair<std::string, std::string>(user, pass));
        }
    }
    /*for (auto it=passes.begin(); it!=passes.end(); ++it){
        std::cout<<it->first <<" "<<it->second<<std::endl;
    }*/


    int socket_desc , client_sock , c, server_port;
    struct sockaddr_in server , client;
    if (argc == 2){
        server_port = atoi(argv[1]);

    } 
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( server_port );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[BUFSIZ];
     
    //Send some messages to the client
    char message[BUFSIZ] = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    sprintf(message,"Enter P for private conversation\nEnter B for message broadcasting\nEnter E for Exit\n");
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , sizeof(client_message) , 0)) > 0 )
    {
        //end of string marker
        client_message[read_size] = '\0';

        //Enter message mode
        if (strcmp(client_message, "P\n") == 0) {
            privateMessage(sock);
        } else if (strcmp(client_message, "B\n") == 0) {
            //std::cout << "chose: B\n";
        } else if (strcmp(client_message, "E\n") == 0) {
            //std::cout << "chose: E\n";
            break;
        }
        
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
        
        //clear the message buffer
        memset(client_message, 0, 2000);

        //Send menu
        char menu[BUFSIZ];
        sprintf(menu,"Enter P for private conversation\nEnter B for message broadcasting\nEnter E for Exit\n");
        write(sock, menu, strlen(menu));
        memset(menu, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
}


