
#include "server.hpp"

#include "transleep.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>

#include <fstream>
using namespace std;
ofstream output_file;
#define TRUE 1
#define FALSE 0

// A huge part of this assignment is from the online source that I would like to cite.
//https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[10] ,
        max_clients = 10 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    // for server timeout after 30 sec of inactivity
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    
    char client_message[2000];
    memset(client_message, 0, sizeof(client_message));
    int port=*(int *)argv[1];
        
    int transNumber =0;
    string fileName = "Server Output";
    output_file.open(fileName);
    struct timeval current_time;
    
    time_t startTimer, endTimer;
    time(&startTimer);
    time(&endTimer);
    double seconds=0;
    
    output_file<<"       Server Output\n";
    output_file<<"Using port "<<argv[1]<<"\n";
    output_file.close();
        
    //set of socket descriptors
    fd_set readfds;
        
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }
        
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
        sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
        
    //bind the socket to localhost port 8888
    if (::bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //printf("Listener on port %d \n", PORT);
        
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
        
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
        
    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);
    
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
            
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];
                
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
                
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }
    
        //the timeout is set to 30 sec
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);
        if (activity <= 0) {
            break;
        }else if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }
            
        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
                
            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , client_message, 2000)) == 0)
                {
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                    
                //Process the message recieved
                else
                {
                    transNumber = transNumber+1;
                    if(transNumber ==1){
                        time(&startTimer);
                    }
                    
                    gettimeofday(&current_time, NULL);
                    output_file.open(fileName, ios_base::app);
                    output_file<<current_time.tv_sec<<"."<<current_time.tv_usec<<": # "<<transNumber<<" T"<<client_message<<"\n";
                    output_file.close();
                
                    int number = std::stoi(client_message);
                    
                    //we give trans the number that we got from client to process it
                    Trans(number);
                    
                    //we clear the client message array, add D as a done message and send it back to client.
                    memset(client_message, 0, sizeof(client_message));
                    client_message[0] = 'D';
                    std::string transNumStr = std::to_string(transNumber);
                    for (int i=1; i<=transNumStr.length();i++){
                        client_message[i] = transNumStr[i-1];
                    }
                    send(sd , client_message , strlen(client_message) , 0 );
                    memset(client_message, 0, sizeof(client_message));
                    
                    gettimeofday(&current_time, NULL);
                    output_file.open(fileName, ios_base::app);
                    output_file<<current_time.tv_sec<<"."<<current_time.tv_usec<<": # "<<transNumber<<" (Done)"<<"\n";
                    output_file.close();
                    time(&endTimer);
                }
            }
        }
    }
    
    output_file.open(fileName, ios_base::app);
    output_file<<"SUMMARY"<<"\n";
    output_file<<"   "<<transNumber<<" transactions"<<"\n";
    seconds = difftime(endTimer,startTimer);
    output_file<<"   "<<transNumber/seconds<<" transactions/sec"<<"\n";
    output_file.close();
    
    return 0;
}
