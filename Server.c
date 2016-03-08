/* 
 * File: P2/Server.c
 * Desc: Implements the functions related to the Server structure. 
 * Author: Jasjeet Dhaliwal
 * Date: 3/7/2016
 *
 */

#include "Server.h"


int initServer(Server * server, serv_args* args) {
	
	/* Create a file descriptor for the server socket */
	if ((server->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		sError ("Error in opening socket for the server. "); 
		return 0; 
	}


	/* Populate the sockaddr_in struct with IP and port from the serv_args struct */
	bzero((char*)&(server->serv_addr), sizeof(server->serv_addr)); 
	server->port = atoi(args->port); 	
	server->serv_addr.sin_family = AF_INET; 
	server->serv_addr.sin_addr.s_addr = INADDR_ANY; 
	server->serv_addr.sin_port = htons(server->port); 
	strcpy(server->ack, "ACKS"); 
	bzero((char*)&(server->cliaddr),sizeof(server->cliaddr)); 
	server->clilen = sizeof(server->cliaddr);

 
	/* Bind the server to the socket */
	if (    bind(server->sockfd, (struct sockaddr *)&(server->serv_addr), sizeof(server->serv_addr)) < 0) {
		sError ("Error: server failed to bind."); 
		return 0; 
	}

	listen(server->sockfd, 5); 


	bzero(server->r_buffer, BUFFER_SIZE); 
	
	/* Helper variables */ 
	int n = 0,  r_temp = 0;  
	char seq = '0';  
		
	/* Wait for File name from client */  
	while(1) {
		r_temp = recvfrom(server->sockfd, server->r_buffer, BUFFER_SIZE, 0,(struct sockaddr *)&(server->cliaddr), &(server->clilen)); 


		if (r_temp <= 0) continue; 
		else {
			seq = (char )server->r_buffer[r_temp-1];
			/* Perform CRC */
			if (crc_server(server->r_buffer, r_temp-1)) continue; 
			else  {
				server->ack[3] = seq;
				server->ack[4] = 0;
				server->r_buffer[r_temp-2] = 0;
				break;
			}; 
		}
	}
	
	/* Send ACK for the File name frame */ 
	if (sendto(server->sockfd, server->ack, strlen(server->ack), 0,(struct sockaddr *)&(server->cliaddr), server->clilen ) < 0) sError ("Error: failed to send ACK to client.");

	/* Open file with the received name */ 
	FILE * file; 
	file = fopen(server->r_buffer, "w"); 

	/* Main loop: receives data frames of size 10 bytes, responds with an ACK, and writes data to file */ 
	while (1) {
	
		bzero(server->r_buffer, BUFFER_SIZE); 
		
		/* Get data frame */ 
		while(1) {
			n = recvfrom(server->sockfd, server->r_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&(server->cliaddr), &(server->clilen)); 	 		if (n <= 0) continue; 
			else {  
				seq = (char)server->r_buffer[n -1];  
							
				/* Perform CRC */
				if (crc_server(server->r_buffer, n-1)) { printf("Failed\n"); continue; } 
				else  {
					server->ack[3] = seq;
					server->ack[4] = 0;
					break;     
				}; 
			}
		}
		
		if (strstr(server->r_buffer, "FIN")) {
			if ( sendto(server->sockfd, server->ack, strlen(server->ack), 0, (struct sockaddr *)&(server->cliaddr), server->clilen) < 0) sError("Error: failed to send ACK to client."); 
			else break; 
		}  
	
		/* Write to file logic: 
 		   Case 1: n == 10 --> write to file in 2 iterations of 5 bytes each. 
		   Case 2: n < 10 -->  
					Case 2.1: n > 5 --> write to file in 2 iterations
					Case 2.2: n <=5 --> write to file in 1 iteration
		*/ 
		/* Removing crc and seq */ 
				
		n = n -2; 
		if (n == 8) {
			fwrite(server->r_buffer,1, 5, file);	
			fwrite(&(server->r_buffer[5]), 1, 3, file); 
 		}

 
		else {
			if (n > 5) {
				 fwrite(server->r_buffer, 1, 5, file);
				 fwrite (&(server->r_buffer[5]), 1, n-5, file); 
			}

			else fwrite(server->r_buffer, 1, n, file);  
		}
		
		/* Send ACK to Client */ 
		if ( sendto(server->sockfd, server->ack, strlen(server->ack), 0, (struct sockaddr *)&(server->cliaddr), server->clilen) < 0) sError("Error: failed to send ACK to client."); 
	}
	fclose(file); 
	return 1; 
}


uint8_t crc_server (char * message, int len) {
        uint8_t remainder = 0;
        int i; 
	int bit; 
        for (i = 0; i< len; ++i) {
                remainder ^= message[i];
                for (bit = 7; bit >= 0; --bit) {
                        // Check if the first bit is 1                          
                       if (remainder & 0x80) {
				remainder^=POLY;
                       } 
                      
			remainder = remainder << 1;
                } 
	}  
	return remainder; 
}



int closeServer(Server * server) {
	/*Close file descriptors */ 
	close(server->sockfd); 
	return 1; 

}


void sError ( char * str) {
	printf ("%s\n", str); 
	exit(EXIT_FAILURE); 
}
