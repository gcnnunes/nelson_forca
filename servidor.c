#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>

#include "basic.h"
#include "socket_helper.h"

#define LISTENQ 10              
#define MAXDATASIZE 4096         
#define EXIT_COMMAND "exit\n"

void doit(int connfd, struct sockaddr_in clientaddr, char *line);

int main (int argc, char **argv) {
   int    listenfd,              
          connfd,               
          port;                            
   struct sockaddr_in servaddr;  
   char   error[MAXDATASIZE + 1];    
   char   *line = NULL;
   char wordsize;
   size_t len = MAXDATASIZE;
   ssize_t read;

 

   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   FILE *fp = fopen("dicionario.txt","r");
   if (fp == NULL)
      exit(EXIT_FAILURE);

   port = atoi(argv[1]);

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);


   servaddr = ServerSockaddrIn(AF_INET, INADDR_ANY, port);


   Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   Listen(listenfd, LISTENQ);


   for ( ; ; ) {
      pid_t pid;

      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);

      connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddr_len);

      if ((read = getline(&line, &len, fp)) == -1) 
         exit(EXIT_FAILURE);          

      wordsize = strlen(line)+'0';
      write(connfd, &wordsize, sizeof(wordsize));

      if((pid = fork()) == 0) {
         Close(listenfd);
         fclose(fp);

         doit(connfd, clientaddr, line);

         Close(connfd);

         exit(0);
      }

      Close(connfd);
   }

   fclose(fp);   
   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr, char *line) {
   char recvline[MAXDATASIZE + 1];
   int n;                  
   socklen_t remoteaddr_len = sizeof(clientaddr);
   
   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
      recvline[n] = 0; 

      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

      printf("<%s-%d>: %s\n", inet_ntoa(clientaddr.sin_addr),(int) ntohs(clientaddr.sin_port), recvline);

      if(strcmp(recvline, EXIT_COMMAND) == 0) {
         break;
      }

      write(connfd, recvline, strlen(recvline));

   }
}
