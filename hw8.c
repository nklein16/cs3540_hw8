#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <setjmp.h>

void err_sys(char* message);

int sockfd, newsockfd;
int portno;
int clilength;
int n;
int debug = 1;
jmp_buf jbuf;

char buf[256];

struct sockaddr_in serv_addr, cli_addr;

int main()
{

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		err_sys("Failed to create socket...");

	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = 80;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if((setjmp(jbuf)) != 0)
	{	
		printf("You are in setjmp\n");
		portno = 23264;
		serv_addr.sin_port = htons(portno);
		printf("portno is %d\n", portno);
	}
	
	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		longjmp(jbuf, portno++);
	
	listen(sockfd, 5);
	printf("listening\n");

	clilength = sizeof(cli_addr);
	pid_t pid;
	while(1)
	{
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilength);
		if(newsockfd < 0)
			err_sys("Error on accept");

		if((pid = fork()) < 0)
			err_sys("Failed to fork()");
		else if(pid == 0)
		{
			printf("Child, processing connection...\n");
			bzero(buf, 256);
			n = read(newsockfd, buf, 255);
			if(n < 0)
				err_sys("Error on read");
			printf("Here is what we got: %s\n", buf);
			n = write(newsockfd, "I got your message", 18);
			if(n < 0)
				err_sys("Error on write");
			close(newsockfd);
			exit(0);
		}
		else
		{
			printf("Parent, waiting for more connections...\n");
			close(newsockfd);
		}
	}
	close(sockfd);
	return 0;
}

void err_sys(char* message)
{
	printf("%s\n", message);
	exit(0);
}
