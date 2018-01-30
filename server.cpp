/*
 * server.cpp
 *
 *  Created on: Jan 18, 2018
 *      Author: Rubik Jain
 */
// Server side C/C++ program to demonstrate Socket programming


/* ----------- ------------------------- ---------------- ------------- ------
   |Operation| |Number of destination n| |Destination[n]| |Buffer Size| |Data|
   ----------- ------------------------- ---------------- ------------- ------ 
   0         7 8                      15 16         16+8n
*/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>
#include <list>
#include <unistd.h>

using namespace std;

#define PORT 8080
#define MAX_EVENTS 10
#define MAX_FRIENDS 10
#define MAX_MESSAGE_SIZE 1024
unsigned int id = 0;

typedef struct packet_login
{
    int operation;
}login_packet;

typedef struct client_connection
{
    int operation;
    int id;
}new_client;

typedef struct packet_format
{
    int operation;
    int destination_id;
    int buffer_size;
    char buffer[MAX_MESSAGE_SIZE];
}send_packet;

typedef struct broadcast_format
{
    int operation;
    int total_friends;
    int destination_id[MAX_FRIENDS];
    int buffer_size;
    char buffer[MAX_MESSAGE_SIZE];
}broadcast_packet;

typedef struct client_information
{
    int client_fd;
    int client_id;
}client_info;

std::list <client_info> client_list;

enum operation
{
    LOGIN,
    LOGOUT,
    SEND,
    BROADCAST,
    ONLINE,
    NEW_CLIENT
};

enum state
{
    AVAILABLE,
    AWAY
};

int process_data(void *buffer, int size, int client_fd);
bool delete_the_client(int client_fd);
bool check_fd_is_present(int client_fd);
int get_fd_from_id(int client_id);
void broadcast(int id);

int main()
{
    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    void *buffer = NULL;
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd, n = 0, cFd = -1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

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

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        cFd = events[n].data.fd;
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_fd)
            {
                printf("\nJain\n");
                conn_sock = accept(server_fd, (struct sockaddr *) &address, &addrlen);
                if (conn_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                //setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                //ev.events = EPOLLIN;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            else
            {
                     cout<<"Received fsgas\n";
                if(events[n].events & EPOLLIN & EPOLLET)
  //              {
                     valread = recv(events[n].data.fd, buffer, sizeof(buffer),0);
                     if(valread > 0)
                    {
                        process_data(&buffer, valread, events[n].data.fd);
                        //do_use_fd(events[n].data.fd);
                        printf("\nRead size %d\n",valread);
    //                }
                }
                else
                {
                     cout<<"Received\n";
                }
            }
        }
    }
    return 0;
}

int process_data(void *buffer, int size, int client_fd)
{
    int operation;
    cout<<"Entering process data\n";
    memcpy(&operation, (char *)buffer, sizeof(operation));
    cout<<"Operation is :"<<operation<<endl;
    switch(operation)
    {
        case LOGIN:
        {
            if(!check_fd_is_present(client_fd))
            {
                client_info c;
                c.client_fd = client_fd;
                c.client_id = id;
                client_list.push_back(c);
                cout<<"Client "<<id<<" logged in\n";
		broadcast(id);
                id++;
            }
            else
            {
                cout<<client_fd<<" is already available\n";
            }
            break;
        }
        case LOGOUT:
        {
            cout<<"Deleting\n";
            if(delete_the_client(client_fd))
            {
                cout<<"Client Logged off!!\n";
            }
            else
            {
                cout<<client_fd<<" is not available\n";
            }
            break;
        }
        case SEND:
        {
#if 0
            int number_of_destination;
            int position = sizeof(operation);
            memcpy(&number_of_destination, (char *)&buffer+position, sizeof(number_of_destination));

            int buffer_size;
            memcpy(&buffer_size,(char *) &buffer+position+(sizeof(int)*number_of_destination), sizeof(int));
            char data[MAX_MESSAGE_SIZE];
            memcpy(data, (char *) &buffer+position+(sizeof(int)*number_of_destination)+sizeof(buffer_size),buffer_size);

            position += sizeof(number_of_destination);
            for(int i = 0; i < number_of_destination; i++)
            {
                int client_id;
                memcpy(&client_id, (char *)&buffer+position, sizeof(client_id));
                position += sizeof(client_id);
                int client_fd = get_fd_from_id(client_id);
                if(client_fd >= 0)
                {
                    send(client_fd , data , strlen(data) , 0 ); //To check if fd can be zero
                }
                else
                {
                    cout<<"Receiver not available\n";
                }
            }
#endif
            send_packet sp;
            memcpy(&sp, (send_packet *)&buffer, sizeof(send_packet));
            int client_fd = get_fd_from_id(sp.destination_id);
            if(client_fd >= 0)
            {
                cout<<"Client message:"<<sp.buffer<<endl;
                send(client_fd , sp.buffer , sp.buffer_size , 0 ); //To check if fd can be zero
            }
            else
            {
                cout<<"Receiver not available\n";
            }
            break;
        }
        case BROADCAST:
        {
            broadcast_packet bp;
            memcpy(&bp, (send_packet *)&buffer, sizeof(broadcast_packet));
            
            break;
        }
        default:
        {
            printf("\nInvalid message received\n");
            break;
        }
    }
}

bool check_fd_is_present(int client_fd)
{
    for (std::list<client_info>::iterator it = client_list.begin(); it != client_list.end(); ++it)
    {
cout<<"Client_fd:"<<it->client_fd<<" client_id"<<it->client_id<<"\n";
#if 0
        if(it->client_fd == client_fd)
        {
            return true;
        }
#endif
    }
    return false;
}

bool delete_the_client(int client_fd)
{
    for (std::list<client_info>::iterator it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(it->client_fd == client_fd)
        {
            client_list.erase(it);
            return true;
        }
    }
    return false;
}

int get_fd_from_id(int client_id)
{
    for (std::list<client_info>::iterator it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(it->client_id == client_id)
        {
            return it->client_fd;
        }
    }
    return -1;
}

void broadcast(int id)
{
    for (std::list<client_info>::iterator it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(it->client_id != id)
        {
            new_client client;
            client.operation = NEW_CLIENT;
            client.id = id;   
            if(send (it->client_fd, &client, sizeof(client), 0) < 0)
            {
                cout<<"New client id sending failed\n";
            }
        }
    }
}
