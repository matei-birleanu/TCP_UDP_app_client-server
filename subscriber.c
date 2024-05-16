
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "helpers.h"

void run_client(int sockfd, char *id) {
	char buffer[1025];
	memset(buffer, 0, 1025);
	struct msg mes;
	// trimit mesaj serverului cu id ul meu
	strcpy(mes.content,id);
	int ret = send(sockfd, &mes, sizeof(mes), 0);
	DIE(ret < 0, "send");
	struct pollfd pfds[2];
	int num = 0;
	// creez multimea de fd
	pfds[num].fd = STDIN_FILENO;
	pfds[num].events = POLLIN;
	num++;
	pfds[num].fd = sockfd;
	pfds[num].events = POLLIN;
	num++;
	while(1){
		ret = poll(pfds, num,-1);
		//primesc mesaj de la tastatura
		if(pfds[0].revents & POLLIN) {
			fgets(buffer,sizeof(buffer),stdin);
			char comm[1024], topic[51];
			int nr = sscanf(buffer, "%s %s", comm, topic);
			// initalizez tipul mesajul 
			// si celelalte campuri pentru a fi
			// trimis catre server
			if(nr == 2 && !strcmp(comm,"subscribe")){
				strcpy(mes.type,"1");
				strcpy(mes.topic,topic);
				strcpy(mes.content, id);
				ret = send(sockfd, &mes, sizeof(mes),0);
				DIE(ret < 0, "not sent to server");
				printf("Subscribed to topic %s\n",topic);
				continue;	
			}
			if(nr == 2 && !strcmp(comm, "unsubscribe")){
				strcpy(mes.type,"2");
				strcpy(mes.topic,topic);
				strcpy(mes.content, id);
				ret = send(sockfd, &mes, sizeof(mes),0);
				DIE(ret < 0, "message not sent to server");
				printf("Unsubscribed from topic %s\n",topic);
				continue;	
			}
			if(nr == 1 && !strcmp(comm, "exit")){
				strcpy(mes.type, "0");
				ret = send(sockfd, &mes, sizeof(mes), 0);
				DIE(ret < 0, "message not sent to server");
				break;
			}
			printf("Unknown command.\n");
			continue;
		}
		if(pfds[1].revents & POLLIN) { // primesc mesaj de la server
			ret = recv(pfds[1].fd,&mes,sizeof(msg),0);
			DIE(ret < 0, "message from server not received");
			if(!strcmp(mes.content, "exit"))
				break;
			// daca nu este exit mesajul afisez continutul	
			printf("%s:%d - %s - %s - %s\n", mes.ip_udp, mes.port, 
			mes.topic, mes.type, mes.content);	
		}
	}
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	if (argc != 4) {
		printf("\n Usage: %s <ip> <port>\n", argv[0]);
		return 1;
	}
	char *clientid = argv[1];
	// Parsam port-ul ca un numar
	uint16_t port;
	int rc = sscanf(argv[3], "%hu", &port);
	DIE(rc != 1, "Given port is invalid");

	// Obtinem un socket TCP pentru conectarea la server
	const int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	// Completăm in serv_addr adresa serverului, familia de adrese si portul
	// pentru conectare
	struct sockaddr_in serv_addr;
	socklen_t socket_len = sizeof(struct sockaddr_in);

	memset(&serv_addr, 0, socket_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	rc = inet_pton(AF_INET, argv[2], &serv_addr.sin_addr.s_addr);
	DIE(rc <= 0, "inet_pton");

	// Ne conectăm la server
	rc = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(rc < 0, "connect");
	int flag = 1;
	// dezactivez aglortimul lui Neagle
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	run_client(sockfd, clientid);

	// Inchidem conexiunea si socketul creat
	close(sockfd);

	return 0;
	}
