#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <csignal>
#include <queue>
#include <sstream>

using namespace std;

int portNum, adr[1024], f[2], fds[2], t;
char bfr[20];
int len=0;

struct readyQueue
{
	int sckt;
	string fileSum;
	string clientId;
	int pNum;
	readyQueue(string ClientId,string FileSum,int Sckt,int PNum)
	{
		sckt=Sckt;
		fileSum=FileSum;
		clientId=ClientId;
		pNum=PNum;
	}
	readyQueue(){}
};
readyQueue readyQ[1024];
queue<readyQueue> rdyQu;

struct connActive
{
	int sckt,flag;
	connActive(int Sckt,int Flag)
	{ 
		sckt=Sckt;
		flag=Flag;
	}
	connActive(){}
};

connActive active[1024];
queue<connActive> conAct;

void sigHandle(int sgnl)
{
	close(f[1]);
	while(!rdyQu.empty())
	{
		readyQueue r = rdyQu.front();
		string name = r.fileSum;
		string cli = r.clientId;
		int p=r.pNum;
		int s=r.sckt;
		string portNum=to_string(p);
		string st=to_string(s);
		string sng = cli + ", " + name + ", " + st + ", " + portNum;
		write(f[0],sng.c_str(),50);
		rdyQu.pop();	
	}
}

void *adminThread(void *)
{
	close(fds[1]);
	while(1)
	{
		read(fds[0],bfr,20);
		if(*bfr=='t'||*bfr=='T')
		{
			readyQueue rus = rdyQu.front();
			while(!rdyQu.empty())
			{
				close(rus.sckt);
				rdyQu.pop();
			}
			exit(0);
			break;
		}
		if(*bfr=='x'||*bfr=='X')
		{
			while(!rdyQu.empty())
			{
				int sum=0;
				ifstream f;
				readyQueue rm = rdyQu.front();
				if(rm.fileSum!="nullfile")
				{
					f.open(rm.fileSum);
					if(!f)
					cout << "File could not be opened..!!";
					int number;
					f >> number;
					while(number>0)
					{
						int n;	
						f>>n;
						sum+=n;
						number--;
					}
					string str = to_string(sum);
					send(rm.sckt,str.c_str(),20,0);
					rdyQu.pop();
				}
				usleep(t*1000);
			}
		}
	}
}

int sockfd, newsockfd,portno;
struct sockaddr_in serv_addr, cli_addr;
socklen_t clilen;
char buffer[1024];

void *ccThread(void *)
{ 
	while(1)
	{
		int cltSocket;
		clilen = sizeof(cli_addr);
		cltSocket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (cltSocket < 0)
		{
			perror("ERROR on Accept");
			exit(1);
		}
		adr[len]=cltSocket;
		len++;
	}
}

void *cThread(void *)
{
	while(1)
	{
		char bfr1[50];
		int j;
		for(int i=0;i<len;i++)
		{	   
			j=read(adr[i],bfr1,50);
			if(j<0)
			{
				perror("Receiving failed");
				exit(1);
			}
			string fileSum=bfr1;
			bzero(bfr1,50);
			string arr[2];
			int j = 0;
			stringstream sts(fileSum);
			while (sts.good() && j < 2)
			{
				sts >> arr[j];
				++j;
			}
			int client;
			client=ntohs(cli_addr.sin_port);
			if(arr[1]!="nullfile")
			{
				readyQ[i]=readyQueue(arr[0],arr[1],adr[i],client);
				active[i]=connActive(adr[i],1);	
				rdyQu.push(readyQ[i]);
				conAct.push(active[i]);
			}	
			else
			{
				active[i]=connActive(adr[i],0);
				conAct.push(active[i]);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	char* admCommand;
	char val;
	admCommand=&val;
	pipe(fds);
	pipe(f);
	int pid=fork();
	int x;
	
	if(pid==0)
	{
		x = getpid();
		cout << "Computer, " << pid << ", " << x << endl;
		signal(SIGRTMIN,sigHandle);
		portNum=atoi(argv[1]);
		t=atoi(argv[2]);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		if (sockfd < 0)
		{
			perror("ERROR opening Socket");
			exit(1);
		}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = portNum;
		serv_addr.sin_port = htons(portno);
		serv_addr.sin_family = AF_INET;		
		serv_addr.sin_addr.s_addr = INADDR_ANY;
	
		if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		{
			perror("ERROR on Binding");
			exit(1);
		}
		listen(sockfd,5);
		cout << "computer process server socket ready" << endl;
		pthread_t threads[4];
		pthread_create(&threads[1],NULL,ccThread,NULL);
		pthread_create(&threads[2],NULL,cThread,NULL);
		pthread_create(&threads[3],NULL,adminThread,NULL);
		pthread_join(threads[3],NULL);
	}
	else
	{
		cout << "Admin, " << pid << ", " << getpid() << endl;
		sleep(1);
		while(1)
		{
			cout << "Admin Command:";
			cin >> admCommand;
			if(*admCommand=='x'||*admCommand=='X')
			{
				close(fds[0]);
				write(fds[1],admCommand,5);
			}
			else if(*admCommand=='t'||*admCommand=='T')
			{
				close(fds[0]);
				write(fds[1],admCommand,5);
				exit(0);
			}
			else if(*admCommand=='q'||*admCommand=='Q')
			{
				cout << x;	
				kill(x,SIGRTMIN);
				close(f[1]);
				char rs[50];	
				read(f[0],rs,50);
				cout << rs;
				break;
			}
		}
	}
return 0;
}
