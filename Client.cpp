#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <bits/stdc++.h>
#include <ctype.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char* argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	portno=atoi(argv[3]);
    char buff[1024];
	int cltId=atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(0);
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_port = htons(portno);
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[2]);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
		perror("ERROR connecting");
		exit(0);
	}

    bzero(buff,1024);
    while(1)
	{
		string st, st1;
		string client=to_string(cltId);
		cout << endl << "Input file name: ";
		getline(cin,st);
		st1 = client + " " + st;
		if(st=="nullfile")
		{
			exit(1);
		}
		else
		{
			char fileSum[20];
			write(sockfd,st1.c_str(),50);
			char resp[20];
			bzero(&resp,20);
			int r = recv(sockfd, &resp, 20, 0);
			if(r==0)
			{
				close(sockfd);
				exit(0);
			}
			cout << cltId << ", " << st << ", " << resp;
		}
	}
    close(sockfd);
    return 0;
}