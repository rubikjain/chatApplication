// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <thread>
#include <list>

using namespace std;

#define PORT 8080
#define MAX_EVENTS 10
#define MAX_FRIENDS 10
#define MAX_MESSAGE_SIZE 1024

typedef struct packet_login_logout
{
    int operation;
}login_logout_packet;

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

enum state
{
    AVAILABLE,
    AWAY
};

enum operation
{
    LOGIN=0,
    LOGOUT,
    SEND,
    BROADCAST,
    ONLINE,
    NEW_CLIENT
};

std::list <int> client_list;

void receiverThread(int sock);
void process_data(void *buffer, int size);

int main(int argc, char const *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    int client_state = AWAY;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        printf("\nTrying to connect \n");
        sleep(1);	
    }

    login_logout_packet login;
    login.operation = 0;
    if(send(sock , &login , sizeof(login) , 0 ) > 0)
    {
        thread t1(receiverThread, sock);
	client_state = AVAILABLE;
	while(client_state != AWAY)
	{
	    int input;
	    cout<<"Press any key:\n";
	    cout<<"1. Logout\n";
	    cout<<"2. Send\n";
	    cout<<"3. Broadcast\n";
	    cin>>input;
	    switch(input)
	    {
	         case LOGOUT:
	         {
                     login_logout_packet logout;
                     logout.operation = 1;
#if 0
                     cout<<"Sizeof Logout:"<<sizeof(logout);
                     void *ptr = NULL;
                     ptr = &logout;
                     cout<<"Logout"<<logout.operation;
                     for(int i=0;i<sizeof(logout);i+=1)
                     {
                         printf("%c", (ptr+i));
                     }
#endif
                     if(send(sock , &logout , sizeof(logout) , 0 ) < 0)
                     {
                         cout<<"Failed to send the logout packet\n Still logging off";
                     }
                     else
                     {
                         cout<<"Logging off\n";
                     }
                     client_state = AWAY;
	             break;
	         }
		 case SEND:
                 {
                     send_packet sp;
                     sp.operation = SEND;
                     cout<<"Below friends are online. Whom do you want to send?\n";
                     cin>>sp.destination_id;
                     cout<<"Enter your message:\n";
                     cin>>sp.buffer;
                     sp.buffer_size = strlen(sp.buffer);
                     if((send(sock , &sp , sizeof(sp) , 0 )) < 0)
                     {
                         cout<<"Sending to socket failed\n";
                     }
                     break;
                 }
                 case BROADCAST:
                 {
                     if(client_list.size() > MAX_FRIENDS)
                     {
                         cout<<"Friend limit exceeded\n";
                         break;
                     }
                     broadcast_packet bp;
                     bp.total_friends = client_list.size();
                     bp.operation = BROADCAST;
                     cout<<"Enter your message:\n";
                     cin>>bp.buffer;
                     bp.buffer_size = strlen(bp.buffer);
                     int i = 0;
                     for(std::list <int>::iterator it = client_list.begin(); it != client_list.end(); ++it)
                     {
                         bp.destination_id[i++] = *it;
                     }
                     if((send(sock , &bp , sizeof(bp) , 0 )) < 0)
                     {
                         cout<<"Sending to socket failed\n";
                     }
                     break;
                 }
                 default:
		 {
		      cout<<"Invalid input\n";
		      break;
		 }
	    }
	    cin.clear();
	}
        t1.join();
        close(sock);
    }
    else
    {
	cout<<"Send failed\n";
    }
    return 0;
}

void receiverThread(int sock)
{
    cout<<"Handling\n";
}
#if 0
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd, n;
    void *buffer = NULL;
    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (n = 0; n < nfds; n++)
        {
            cout<<"Nfds are:"<<nfds<<endl;
            int readFd = events[n].data.fd;
            int length = read(readFd, buffer, 1024);
            process_data(&buffer,length);
        }
    }
}

void process_data(void *buffer, int size)
{
    int operation;
    memcpy(&operation, (char *)buffer, sizeof(operation));
    cout<<"Operation is :"<<operation<<endl;
    switch(operation)
    {
        case NEW_CLIENT:
        {
            int id;
            memcpy(&id, (char *)(&buffer+sizeof(operation)), sizeof(id));
            client_list.push_back(id);
            break;
        }
        default:
        {
            cout<<"Wrong\n";
            break;
        }
    }            
}
#endif
