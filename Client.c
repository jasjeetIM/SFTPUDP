/*
 * File: P1/Client.c
 * Desc: Implements the functions declared for the client. 
 * Author: Jasjeet Dhaliwal
 * Date: 1/27/2016
 *
 */

#include "Client.h"


int initClient (Client* client, client_args* args) {
	
	/*Create file descriptor for client socket */ 
	if ((client->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			error("Error opening client socket."); 
			return 0; 
	}

	/* Terminate program if server ip is invalid */ 
	if (args->ip == NULL) {
		error("Error: server IP address is not valid."); 
		return 0; 
	}
	
	/* Populate client sockaddr_in struct with IP and port of server */ 
	client->serv_addr.sin_family = AF_INET; 
	client->port = atoi(args->port); 
	client->serv_addr.sin_port = htons(client->port);
        client->serv_addr.sin_addr.s_addr = inet_addr(args->ip);
	client->servlen = sizeof(client->serv_addr); 
	strcpy(client->fin,"FINCS"); 

//	int flags = fcntl(client->sockfd, F_GETFL, 0);
  //	fcntl(client->sockfd, F_SETFL, flags | O_NONBLOCK);

	
	return 1; 
}
	

void error (char * str ) {
	printf ("%s\n", str); 
	exit(EXIT_FAILURE); 
}

uint8_t crc_client(char * message, int len) {
	uint8_t remainder = 0; 
	int i, bit; 
	for (i = 0; i<len; ++i) {
		remainder ^= message[i]; 
		for (bit = 7; bit >= 0; --bit) {
			// Check if the first bit is 1				
			if (remainder & 0x80) {
				remainder^=POLYNOMIAL; 
			}
			
			remainder = remainder << 1; 
		}
 
	}
	
	return remainder; 
}

int sendFile(Client * client, client_args* args) {
	
	/* Initialize file stream */ 
	FILE * inFile;
	unsigned char crc;
	size_t r_temp, w_temp;  
	int j = 0, n = 0, seq = 0; 
	  
	if (!(inFile = fopen(args->in_file, "r"))) error("Error: failed to open input inFile"); 


	/* Send Output File name. Filename must be <=8 bytes */
	crc = (unsigned char) crc_client(args->out_file, strlen(args->out_file)); 
		
	bzero(client->w_buffer, BUFFER_SZ); 
	strcpy(client->w_buffer, args->out_file); 
	w_temp = strlen(client->w_buffer); 
	client->w_buffer[w_temp]= crc;   
	
	for (; ; ) { 	
		
		if (seq%2 == 0) {
			client->w_buffer[w_temp+1] = '0'; 
			client->w_buffer[w_temp+2] = 0; 			
		}

		else {
			client->w_buffer[w_temp+1] = '1'; 
			client->w_buffer[w_temp+2] = 0; 
		}

		if (sendto(client->sockfd,client->w_buffer, strlen(client->w_buffer), 0,(struct sockaddr *)&(client->serv_addr), sizeof(client->serv_addr)) <0) error("Error: failure to send File name to server."); 
		
		/* Wait for an ACK */ 
		usleep(100000); 
		
		//Timeout, so send again
		if(recvfrom(client->sockfd, client->r_buffer, BUFFER_SZ, 0,(struct sockaddr *)&(client->serv_addr), &(client->servlen)) <= 0) continue; 
		
		//Check if Ack has the correct sequence number
		if (strstr(client->r_buffer, "ACK")) {
			r_temp = strlen(client->r_buffer); 
			if (client->r_buffer[r_temp-1] == ((seq%2) + '0'))break; 
		}
	  }

	

			/* Test Duplicate Packet 1 and Recive ACK */ 

			printf("Client: Sending duplicate packet 1 with sequence = %c\n", (char)(seq%2 + '0')); 
 
		if (sendto(client->sockfd,client->w_buffer, strlen(client->w_buffer), 0,(struct sockaddr *)&(client->serv_addr), sizeof(client->serv_addr)) <0) error("Error: failure to send File name to server."); 


		while(1) {
			/* Wait for an ACK */ 
			bzero(client->r_buffer, BUFFER_SZ); 
			usleep(100000); 	
			n = recvfrom(client->sockfd, client->r_buffer, BUFFER_SZ, 0, (struct sockaddr*)&(client->serv_addr), &(client->servlen)); 
			if (n<=0) continue;
			else break;  
		}


		
			/* Test Duplicate Packet 2 and Recive ACK*/   
/*
			printf("Client: Sending duplicate packet 2 with sequence = %c\n", (char)(seq%2 + '0')); 
 
		if (sendto(client->sockfd,client->w_buffer, strlen(client->w_buffer), 0,(struct sockaddr *)&(client->serv_addr), sizeof(client->serv_addr)) <0) error("Error: failure to send File name to server."); 


		while(1) {
			/* Wait for an ACK * 
			bzero(client->r_buffer, BUFFER_SZ); 
			usleep(100000); 	
			n = recvfrom(client->sockfd, client->r_buffer, BUFFER_SZ, 0, (struct sockaddr*)&(client->serv_addr), &(client->servlen)); 
			if (n<=0) continue;
			else break;  
		}

		/* Sending Incorrect Checksum */
		++seq; 		
		if (seq%2 == 0) {
			client->w_buffer[w_temp+1] = '0'; 
			client->w_buffer[w_temp] = '0'; 			
		}

		else {
			client->w_buffer[w_temp+1] = '1'; 
			client->w_buffer[w_temp] ='0' ; 
		}
		printf("Client: Sending incorrect checksum packet\n"); 
		if (sendto(client->sockfd,client->w_buffer, strlen(client->w_buffer), 0,(struct sockaddr *)&(client->serv_addr), sizeof(client->serv_addr)) <0) error("Error: failure to send File name to server."); 
		

		++seq; //To get the sequence count back in order




	/* Main loop : send file data in 10 byte frames, proceed with the next frame after receiving ACK for the sent frame. */
	while(!feof(inFile)) {
		++seq; 
		bzero(client->w_buffer, BUFFER_SZ);
		
		/* Read 8 bytes from file */ 
	     	j = fread(client->w_buffer,1, BUFFER_SZ-2 , inFile); 
		
		/* CRC Calc */
		crc = (unsigned char) crc_client(client->w_buffer, j);  	
		client->w_buffer[j]= crc; 
		 
	 
		if (seq%2 == 0) {
			client->w_buffer[j+1] = '0';  			
		}

		else {
			client->w_buffer[j+1] = '1'; 
		}

		if (sendto(client->sockfd, client->w_buffer, j+2 , 0, (struct sockaddr*)&(client->serv_addr), client->servlen) < 0) error("Error: Send file data failed.");   
		while(1) {
			/* Wait for an ACK */ 
			bzero(client->r_buffer, BUFFER_SZ); 
			usleep(10000); 	
			n = recvfrom(client->sockfd, client->r_buffer, BUFFER_SZ, 0, (struct sockaddr*)&(client->serv_addr), &(client->servlen)); 
			
			/*No ACK from server*/
			if (n <=0)  {
					/* Send file data in a new segment */ 
					if (sendto(client->sockfd, client->w_buffer, j+2, 0, (struct sockaddr*) &(client->serv_addr), client->servlen) < 0) error("Error: Send file data failed.");   	
				continue; 
			}					
			
			/* Recieved ACK, check to see if it contains the correct sequence number */
			if (strstr(client->r_buffer, "ACK")) {
				if (client->r_buffer[n-1] == (seq%2 + '0')) break;
				else {	  
					/* Send file data in a new segment */ 
					if (sendto(client->sockfd, client->w_buffer, j+2, 0, (struct sockaddr*) &(client->serv_addr), client->servlen) < 0) error("Error: Send file data failed."); 
 				continue; 
				}
			}	

		}
	}	
	client->fin[3] = 0; 
	crc = (unsigned char) crc_client(client->fin, 3); 
	client->fin[3] = crc; 
	int i = 0; 
	++seq; 
	for(;i<5;++i) {
		if (seq%2 == 0) {
			client->fin[4] = '0';  		
		}
		else {
			client->fin[4] = '1'; 
		}
		
		if (sendto(client->sockfd, client->fin, 5, 0, (struct sockaddr*) &(client->serv_addr), client->servlen) < 0) error("Error: Send file data failed."); 
		usleep(10000); 
		
		n = recvfrom(client->sockfd, client->r_buffer, BUFFER_SZ, 0, (struct sockaddr*)&(client->serv_addr), &(client->servlen)); 
	
		if (n<=0)  continue; 
		else {
			if (strstr(client->r_buffer, "ACK")) {
					 
					if (client->r_buffer[n-1] == ((seq%2) + '0')) break; 
			}
		}
	}

		
	fclose(inFile);  
	return 1; 
}



int closeClient (Client * client) {
	/*Close file descriptors */ 
	close(client->sockfd); 
	return 1; 
} 
