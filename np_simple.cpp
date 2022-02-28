
// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <cstring>
#include <string>
#include <iostream>

#include "npshell2.h"

std::string convertToString(char* a, int size) 
{ 
    int i; 
    std::string s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
} 

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}, ter[2] = {'%', ' '}; 
    npshell2 np;
    int stdoutCopy = dup(STDOUT_FILENO), stderrCopy = dup(STDERR_FILENO);
    int port = atoi(argv[1]);

    setenv("PATH", "bin:.", 1);
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    while(true)
    {
        //Accept new client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  (socklen_t*)&addrlen))<0) 
        { 
            perror("Accept error"); 
            exit(EXIT_FAILURE); 
        }

        switch (fork())
        {
            case 0:     /* child */
                close(server_fd);
                while(true)
                {
                    std::string input = "";
                    write(new_socket, ter, 2);
                    do
                    {
                        valread = read( new_socket , buffer, 1024); 
                        input = input + convertToString(buffer, valread);
                    }
                    while( buffer[valread-1] != '\n');

                    if(input == "exit")
                        exit(0);
                    dup2(new_socket, STDOUT_FILENO);
                    dup2(new_socket, STDERR_FILENO);
                    np.getInput(input);
                    dup2(stdoutCopy, STDOUT_FILENO);
                    dup2(stderrCopy, STDERR_FILENO);   
                
                }
                break;
            default:    /* parent */
                close(new_socket);
                break;
            case -1:    
                std::cerr << "Fork slave socket error" << std::endl;
        }
    }

    return 0; 
} 

