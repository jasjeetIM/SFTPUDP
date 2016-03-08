/*
 * File: P2/Server.h
 * Desc: This file delcares the Server structure and its related functions.It uses CRC to check for errors.  
 * Author: Jasjeet Dhaliwal
 * Date: 3/7/2016
 *
 */


#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Args.h"
#include <fcntl.h>

#define BUFFER_SIZE 10
#define INITBUFFER_SIZE 40
#define POLY 0xD8

typedef struct Server {
	
	int sockfd, port;  
	unsigned char r_buffer[BUFFER_SIZE];  
	char ack [4];	
	struct sockaddr_in serv_addr, cliaddr;  
	socklen_t clilen; 
} Server; 

int initServer(Server * server, serv_args* args); 
int startListening(Server * server); 
uint8_t crc_server(char * message, int len); 
int closeServer(Server * server); 
void sError (char * str); 

extern Server * server; 

#endif



