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
     
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , sizeof(client_message) , 0)) > 0 )
    {
        //end of string marker
        client_message[read_size] = '\0';
        
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
        
        //clear the message buffer
        memset(client_message, 0, 2000);
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
