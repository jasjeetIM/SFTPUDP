/* 
 * File: P1/client.h
 * Desc: The function prototypes and variables used by the client.
 * Author: Jasjeet Dhaliwal
 * Date: 1/27/2016
 *
 */


#ifndef CLIENT_H
#define CLIENT_H


#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Args.h"
#include <fcntl.h>

#define BUFFER_SZ 10
#define POLYNOMIAL 0xD8

typedef struct Client {
	int sockfd, port;
	struct sockaddr_in serv_addr; 
	socklen_t servlen;
	char fin[5];  
	 unsigned char r_buffer [BUFFER_SZ], w_buffer[BUFFER_SZ];
} Client; 

int initClient(Client * client, client_args* args); 
int sendFile(Client * client, client_args* args); 
uint8_t crc_client(char * message, int len); 
int closeClient(Client * client); 
void error (char * str); 


extern Client * client; 

#endif
