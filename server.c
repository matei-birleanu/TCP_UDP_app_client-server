#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include "helpers.h"

#define MAX_CONNECTIONS 32

msg parse_udp(char *buffer, struct udp_message *upd_mes,struct sockaddr_in *udpserv){
	// prelucrez mesajul udp primit din buffer ul primit
	strncpy(upd_mes->topic,buffer,50);
	upd_mes->data_type = *((uint8_t *)(buffer + 50));
    memcpy(upd_mes->body, buffer + 51, 1500);
	char help[126];
	msg message;
	memset(message.content, 0, 1501);
	inet_ntop(AF_INET, &(udpserv->sin_addr), help, INET_ADDRSTRLEN);
	strcpy(message.ip_udp,help);
	// construiesc mesajul pe care il voi trimite clientilor
	message.port = ntohs(udpserv->sin_port);
	strncpy(message.topic, upd_mes->topic, 51);
	u_int32_t val;
	if (upd_mes->data_type == 0) { // INT
		uint8_t sgn = 0;
		// octetul de semn
		sgn = *((uint8_t *)upd_mes->body);
		val = ntohl(*((uint32_t *)(upd_mes->body + 1)));
		if (sgn != 0)    
			val = val * (-1);
		strcpy(message.type, "INT");
		sprintf(message.content, "%d", val);
		return message;
	}

	if (upd_mes->data_type == 1) { // SHORT REAL
		val = ntohs(*((uint16_t *)upd_mes->body));
		float float_val;
		float_val = (float) val / 100;
		strcpy(message.type, "SHORT_REAL");
		sprintf(message.content, "%.2f", float_val);
		return message;
	}

	if (upd_mes->data_type == 2) { // FLOAT
		uint8_t sgn = 0;
		// octetul de semn
		sgn = *((uint8_t *)upd_mes->body);
		val = ntohl(*((uint32_t *)(upd_mes->body + 1)));
		uint8_t pow = 0;
		// modului puterii negative a lui 10
		pow = *((uint8_t *)(upd_mes->body + 5));
		float float_val;
		float_val = (float)val;
		if (sgn != 0)
			float_val = float_val * (-1);
		while (pow) {
			float_val /= 10;
			pow--;
		}
    	strcpy(message.type, "FLOAT");
    	sprintf(message.content, "%.4f", float_val);
    	return message;
	}

	if (upd_mes->data_type == 3) { // STRING
		strcpy(message.type, "STRING");
		strcpy(message.content, upd_mes->body);
		return message;
	}
	return message;
}
char * get_topic(client * client, char * topic){
	for(int i = 0 ; i < client->nrt; i++)
		if(!strcmp(client->topics[i],topic))
			return client->topics[i];
	return NULL;		
}
void add_topic_to_sub(struct client *client, char *topic){
	int nr = client->nrt;
	strcpy(client->topics[nr],topic);
	client->nrt++;
}
void remove_topic_from_sub(struct client *client, char *topic){
	for(int i = 0 ; i < client->nrt; i++){
		if(!strcmp(client->topics[i],topic)){
			for(int j = i; j < client->nrt - 1; j++)
				strcpy(client->topics[j],client->topics[j + 1]);
		}
		client->nrt--;
		return;
	}
}
client * find_client_with_id(client * clients, char *id, int connected,  int nr){
	for(int j = 0 ; j < nr; j++){
		if(!strcmp(clients[j].id,id) && clients[j].connected == connected){
			return &clients[j];
		}
	}
	return NULL;
}
void run_server(int udp, int tcp, struct sockaddr_in *udpserv){
	char buffer[1025];
	client *clients = NULL;
	memset(buffer, 0, 1025);
	struct pollfd pfds[MAX_CONNECTIONS];
	int num = 0;
	int num_subs = 0;
	int rc;
	rc = listen(tcp, MAX_CONNECTIONS);
	// creez multimea de fd
	pfds[num].fd = STDIN_FILENO;
	pfds[num].events = POLLIN;
	num++;
	pfds[num].fd = udp;
	pfds[num].events = POLLIN;
	num++;
	pfds[num].fd = tcp;
	pfds[num].events = POLLIN;
	num++;
	while(1){
		rc = poll(pfds, num, -1);
		DIE(rc < 0,"poll");
		for(int i = 0 ; i < num; i++){
			if(pfds[i].revents & POLLIN) {
				if(pfds[i].fd == udp){//primesc mesaj de la clientii udp
					memset(buffer, 0, 1025);
					socklen_t udp_size = sizeof(*udpserv);
					rc = recvfrom(udp,buffer,1551,0,(struct sockaddr *)udpserv,&udp_size);
					DIE(rc < 0, "message not received from udp");
					msg message;
					struct udp_message upd_mes;
					memset(upd_mes.topic, 0 ,51);
					memset(upd_mes.body, 0, 1501);
					// creez mesajul pe care l trimit clientilor tcp
					message = parse_udp(buffer,&upd_mes, udpserv);
					for(int j = 0 ; j < num_subs; j++){
						char * topic = get_topic(&clients[j],upd_mes.topic);
						// daca subscriberul este abonat la topic va primi mesajul
						if(topic && clients[j].connected == 1){
							int ret = send(clients[j].fd,&message,sizeof(message),0);
							DIE(ret < 0, "topic message not send to subscriber");
						}
					}
					continue;	
				}
				if(pfds[i].fd == tcp){// primesc mesaj de conectare pe socket ul tcp
					struct sockaddr_in cli_addr;
					msg message;
					socklen_t client_size = sizeof(cli_addr);
					int newsock = accept(tcp, (struct sockaddr *)&cli_addr, &client_size);
					// adaug noul socket in vectorul de fd
					pfds[num].fd = newsock;
					pfds[num].events = POLLIN;
					num++;
					// primesc mesajul cu id ul clientului si tratezi cazurile
					int ret = recv(newsock,&message,sizeof(message),0);
					DIE(ret < 0, "client ID not received");
					if(clients == NULL){
						clients = malloc(sizeof(client));
						clients[num_subs].fd = newsock;
						clients->connected = 1;
						strcpy(clients[num_subs].id,message.content);
						num_subs++;
					}
					else {
						// am deja un client cu id
						client *aux = NULL;
						aux = find_client_with_id(clients,message.content,1,num_subs);
						if(aux != NULL){
							printf("Client %s already connected.\n",message.content);
							msg mes;
							strcpy(mes.content, "exit");
							rc = send(newsock,&mes,sizeof(message),0);
							DIE(rc < 0, "message not sent to server");
							close(newsock);
							// elimin socketul adaugat 
							pfds[num - 1].events = 0;
							num--;
							break;
						} else {
							// verific daca a mai fost cautat intrucat 
							// nu a fost sters din lista de clienti la o 
							// posibila deconectare mai veche
							client *aux = NULL;
							aux = find_client_with_id(clients,message.content,0,num_subs);
							if(aux){
								aux->connected = 1;
								aux->fd = newsock;
							} else {
							clients = realloc(clients, (num_subs + 1) * sizeof(client));
							strcpy(clients[num_subs].id,message.content);
							clients[num_subs].fd = newsock;
							clients[num_subs].nrt = 0;
							num_subs++;
							}
						}
						printf("New client %s connected from %s:%d.\n",
							message.content, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
						break;	

					}
					printf("New client %s connected from %s:%d.\n",
							message.content, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					continue;
				}
				if(pfds[i].fd == STDIN_FILENO){ //primesc mesaj de la tastatura
					fgets(buffer,sizeof(buffer),stdin);
					// in caz de exit trimit mesajul tuturor clientilor
					if(!strncmp(buffer, "exit",4)){
						msg message;
						strcpy(message.content, "exit");
						for(int j = 3; j < num; j++){
							rc = send(pfds[j].fd,&message,sizeof(message),0);
							DIE(rc < 0 ,"exit message not send");
							close(pfds[j].fd);
						}
						free(clients);
						return;	
					}
					continue;
				}
				// primesc mesaj pe un socket deja cunoscut
				// de la un client tcp
				msg message;
				rc = recv(pfds[i].fd, &message,sizeof(message),0);
				DIE(rc < 0, "message not received from client");
				client *client = NULL;
				int j = 0;
				while(j < num_subs){
					if (pfds[i].fd == clients[j].fd) {
						client = &clients[j];
						break;
					}
					j++;
				}
				// tratez toate cazurile de comenzi ale clientului
				switch(atoi(message.type)){
					case 1:
						add_topic_to_sub(client, message.topic);
						break;
					case 2:
						remove_topic_from_sub(client, message.topic);
						break;
					case 0:
						client->connected = 0;
						printf("Client %s disconnected.\n", client->id);
						close(pfds[i].fd);
						
						for (int j = i; j < num - 1; j++) {
       						pfds[j] = pfds[j + 1];
						}
    					pfds[num].events = 0;
    					num--;
						break;	
					default:
						printf("Unknown request coming from TCP client %s.\n",client->id);
						break;	
				}
			}
		}
	}
	
}
int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	if (argc != 2) {
		printf("Usage: ./server <port>\n");
		return 0;
	}
	// Parsam port-ul ca un numar
	uint16_t port;
	int rc = sscanf(argv[1], "%hu", &port);
	DIE(rc != 1, "Given port is invalid");
	if (argc != 2) {
        printf("\n Usage: %s <port>\n", argv[0]);
        return 1;
    }
	//CREEZ SOCKET PENTRU UDP
	// Obtinem un socket TCP pentru receptionarea conexiunilor
	const int listenudpfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(listenudpfd < 0, "socket");

	// Completăm in serv_addr adresa serverului, familia de adrese si portul
	// pentru conectare
	struct sockaddr_in serv_addr;
	socklen_t socket_len = sizeof(struct sockaddr_in);

	const int enable = 1;
	// dezactivez aglortimul lui Neagle
	if (setsockopt(listenudpfd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int)) < 0)
		perror("setsockopt(TCP_NODELAY) failed");

	memset(&serv_addr, 0, socket_len);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	rc = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
	DIE(rc <= 0, "inet_pton");

	// Asociem adresa serverului cu socketul creat folosind bind
	rc = bind(listenudpfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
	DIE(rc < 0, "bind");
	// creez socket pentru tcp
	int listentcpfd = socket(AF_INET, SOCK_STREAM, 0);
	// dezactivez aglortimul lui Neagle
	if (setsockopt(listentcpfd,IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int)) < 0)
		perror("setsockopt(TCP_NODELAY) failed");
	rc = bind(listentcpfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));

	run_server(listenudpfd,listentcpfd,&serv_addr);

	// Inchidem fd
	close(listenudpfd);
	close(listentcpfd);
	return 0;
}
