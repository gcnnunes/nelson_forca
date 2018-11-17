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

void doit(int connfd, struct sockaddr_in clientaddr, char *word);
char   *word = NULL;

int main (int argc, char **argv) {
   int    listenfd,
          connfd,
          port;
   struct sockaddr_in servaddr;
   char   error[MAXDATASIZE + 1];
   char wordsize[100];
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

   printf("OLA\n");


   for ( ; ; ) {
      pid_t pid;

      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);

      connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddr_len);

      word = NULL;
      if ((read = getline(&word, &len, fp)) == -1) {
        fp = fopen("dicionario.txt","r"); // loop pro começo do arquivo
        if ((read = getline(&word, &len, fp)) == -1) // se falhar mesmo tentando abrir o arquivo de novo
            exit(EXIT_FAILURE);
      }

      printf("OLA 2\n");

      sprintf(wordsize, "%lu", strlen(word)-1);

      write(connfd, &wordsize, strlen(wordsize));

      printf("OLA 3\n");

      if((pid = fork()) == 0) {
         Close(listenfd);
         fclose(fp);
         printf("OLA 4\n");

         doit(connfd, clientaddr, word);
         Close(connfd);

         exit(0);
      }

      free(word);
      Close(connfd);
   }

   fclose(fp);
   return(0);
}

int find_char(char *str, char c)
{
   int i,f,lenf=0;
   lenf=strlen(str);
   for(i=0;i<lenf;i++)
   {
      if(str[i]==c)
      {
         printf("character position:%d\n",i+1);
         f=1;
      }
   }
   if(f==0)
   {
      printf("\ncharacter not found");
      return(1);
   }

   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr, char *word) {
   char recvline[MAXDATASIZE + 1];
   int n;
   int positions[20];
   socklen_t remoteaddr_len = sizeof(clientaddr);
   printf("OLA 5\n");

//   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
   while (1) {      
      printf("OLA 6\n");      
      n = read(connfd, recvline, MAXDATASIZE);
      recvline[n] = 0;
      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

      printf("OLA 7\n");
   
      printf("<%s-%d>: %s\n", inet_ntoa(clientaddr.sin_addr),(int) ntohs(clientaddr.sin_port), recvline);
   
      positions[0] = find_char(word, recvline[0]); //procura na palavra a letra recebida do cliente e retorna as posições encontradas

      if(strcmp(recvline, EXIT_COMMAND) == 0) {
         break;
      }
   
      write(connfd, recvline, strlen(recvline));  
   }
}