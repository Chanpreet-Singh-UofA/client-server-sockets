//
//  client.cpp
//  assign3
//
//  Created by Chanpreet Singh on 2021-11-22.
//
#include "client.hpp"
#include "transleep.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#define HOST_NAME_MAX 255
#include <stdio.h>
#include <time.h>
#include <fstream>
using namespace std;
ofstream output_file;
#include <ctime>
int transNumber =0;


int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000], client_host[1000];
    memset(message, 0, sizeof(message));
    memset(server_reply, 0, sizeof(server_reply));
    memset(client_host, 0, sizeof(client_host));
    int port=*(int *)argv[1];
    
    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);
    int pid = getpid();
    std::string hName = hostname;
    
    std::string fileName = (" "+hName+"."+std::to_string(pid));
    output_file.open(fileName);
    ;
    output_file<<"In         "<<fileName<<" Output\n";
    output_file<<"           "<<"Using port "<<argv[1]<<"\n";
    output_file<<"           "<<"Using Server Address "<<argv[2]<<"\n";
    output_file<<"           "<<"Host "<<fileName<<"\n";
    output_file.close();
    struct timeval current_time;
    
    
    //--------------------------------------------------------------------------------------------------------------
    //binarytides.com/server-client-example-c-sockets-linux
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(argv[2]);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
    
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected\n");
    
    //--------------------------------------------------------------------------------------------------------------
    
    
    bool condition = true;
    //keep communicating with server
    while(condition)
    {
        memset(message, 0, sizeof(message));
        memset(server_reply, 0, sizeof(server_reply));
        scanf("%s" , message);
        
        if(message[0] == 'S'){
            
            output_file.open(fileName, ios_base::app);
            output_file<<message<<"         Sleep ";
            char* substr = message + 1;
            memmove(message, substr, strlen(substr) + 1);
            int number = std::stoi(message);
            output_file<<number<<" units"<<"\n";
            output_file.close();
            Sleep(number);
            
        }else if (message[0] == 'T'){
            
            //send T value to server
            gettimeofday(&current_time, NULL);
            output_file.open(fileName, ios_base::app);
            output_file<< message<<"         "<<current_time.tv_sec<<"."<<current_time.tv_usec<< ": Send ("<< message<<")\n";
            output_file.close();
            
            transNumber= transNumber+1;
            char* substr = message + 1;
            memmove(message, substr, strlen(substr) + 1);
            //we add the client name to message before sending it to server
            string temp(message);
            string readyString = temp + fileName;
            strcpy(message, readyString.c_str());
            
            if( send(sock , message , strlen(message) , 0) < 0)
            {
                puts("Send Message failed");
                return 1;
            }
            
            //Receive a reply from the server and then send the next T
            if( recv(sock , server_reply , 2000,0) < 0)
            {
                puts("recv failed");
                break;
            }
            
            gettimeofday(&current_time, NULL);
            output_file.open(fileName, ios_base::app);
            output_file<<"           "<<current_time.tv_sec<<"."<<current_time.tv_usec<< ": Recv ("<<server_reply<<")\n";
            output_file.close();
        }else{
            //If we get any other inpur other then T or S then we close the client.
            condition=false;
        }
    }
    puts("Client ends");
    close(sock);
    output_file.open(fileName, ios_base::app);
    output_file<<"           "<<"Sent "<<transNumber<<" transactions"<<")\n";
    output_file.close();
    return 0;
}
