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

void privateMessage(int, std::string);
void broadcastMessage(int);
void clientExit(std::string);
 
//the thread function
void *connection_handler(void *);
bool passwordsExist = false; //is true if a passwords.txt file already exists
std::map<std::string, std::string> passes; //map with username as key, password as value
std::map<std::string, int> clients; //map with username and sockets

void privateMessage(int sock);
void broadcastMessage(int sock);
void clientExit(int sock);
 
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
    else
    {
      std::cout << "Usage:" << std::endl << "\t./chatserver <port number>" << std::endl;
      exit(1);
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
    char buf[BUFSIZ];
    if (recv(sock, buf, sizeof(buf), 0) == -1){
        perror("error retrieving username from client\n");
    } 
    std::string tempUsername = buf;
    //tempUsername.pop_back();
    bool existingUser = false;
    for (auto it=passes.begin(); it!=passes.end(); ++it){
        if (tempUsername.compare(it->first) == 0){
            existingUser = true;
            std::string returnMessage = "Welcome back! Enter password >> ";
            write(sock, returnMessage.c_str(), strlen(returnMessage.c_str()));
            memset(buf, 0, BUFSIZ);
            /*retrieve password from user*/
            bool gotPass = false;
            while (!gotPass){
                if (recv(sock, buf, sizeof(buf), 0) == -1){
                    perror("error retrieving username from client\n");
                }
                std::string tempPass = buf;
                tempPass.pop_back();
                if (tempPass.compare(it->second) == 0){
                    gotPass = true;
                    continue;
                } 
                else {
                    returnMessage = "Wrong password. Please re-enter >> ";
                    write(sock, returnMessage.c_str(), strlen(returnMessage.c_str()));
                    memset(buf, 0, BUFSIZ);
                }
            }
        }
    }
    if (!existingUser){
        std::string returnMessage = "New user? Create password >> ";
        write(sock, returnMessage.c_str(), strlen(returnMessage.c_str()));
        memset(buf, 0, BUFSIZ);
        if (recv(sock, buf, sizeof(buf), 0) == -1){
            perror("error retrieving username from client\n");
        }
        std::string tempPass = buf;

        /* write new password to file */
        if (passwordsExist){
            std::ofstream ofs("passwords.txt", std::ofstream::app);
            ofs << tempUsername << " " << tempPass;
            ofs.close();
        }
        else { //create new file
            std::fstream outfile;
            outfile.open("passwords.txt", std::fstream::in | std::fstream::out | std::fstream::app);
            outfile << tempUsername << " " << tempPass;
            outfile.close();
        }
        tempPass.pop_back();
        passes.insert(std::pair<std::string, std::string>(tempUsername, tempPass));
        
    }
    //Send some messages to the client
    std::string welcomeMessage = "Welcome " + tempUsername + "!\n";
    write(sock , welcomeMessage.c_str() , strlen(welcomeMessage.c_str()));
    clients.insert(std::pair<std::string, int>(tempUsername, sock));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , sizeof(client_message) , 0)) > 0 )
    {
        std::string mes = client_message;
        std::cout<<"*"<<mes<<"*\n";
        if (mes.compare("CP") == 0)
            privateMessage(sock, tempUsername);
        else if (mes.compare("CB") == 0)
            broadcastMessage(sock);
        else if (mes.compare("E") == 0)
            clientExit(tempUsername);

    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
        clients.erase(tempUsername);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
} 

void privateMessage(int sock, std::string senderName) {
    // Create list of users as a string
    std::string users_str = "Online Users:\n";
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        users_str += " -> " + it->first + "\n";
    } 

    // Convert string to char* & send to client
    char users_buf[BUFSIZ];
    strcpy(users_buf, users_str.c_str());
    write(sock, users_buf, strlen(users_buf));

    // Receive username
    char buffer[BUFSIZ];
    if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
        perror("Error Receiving Message From Client\n");
        exit(1);
    }

    // Check if user exists
    std::string username = buffer;
    int priv_sock = 0;
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (username.compare(it->first) == 0) { priv_sock = it->second; }
    }
    if (priv_sock == 0) {
        std::string userErr = "User does not exist\n";
        write(sock, userErr.c_str(), strlen(userErr.c_str()));
        return;
    }
    
    // Receive message, add formatting, send to user
    memset(buffer, 0, BUFSIZ);
    if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
        perror("Error receiving message from client\n");
        exit(1);
    }
    std::string bufferTemp = buffer;
    std::string sendMsg = "#### New Message from " + senderName + ": " + bufferTemp + " ####";
    write(priv_sock, sendMsg.c_str(), strlen(sendMsg.c_str()));

}

void broadcastMessage(int sock) {
    // Send acknowledgement to server
    std::string conf = "CONF";
    write(sock, conf.c_str(), strlen(conf.c_str()));
    char buffer[BUFSIZ];
    // Receive Message, add formatting
    if (recv(sock , buffer , sizeof(buffer) , 0) <= 0){
        perror("Error receiving message from client\n");
        exit(1);
    }
    // Send message to all users
    std::string bufferTemp = buffer;
    std::string message = "#### New Message: " + bufferTemp + " ####";
    for (auto it = clients.begin(); it!= clients.end(); ++it){
        write(it->second, message.c_str(), strlen(message.c_str()));   
    }
}

void clientExit(std::string username) {
    std::cout << "Client disconnected";
    clients.erase(username);
}
