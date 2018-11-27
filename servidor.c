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

      sprintf(wordsize, "%lu", strlen(word)-1);

      write(connfd, &wordsize, strlen(wordsize));

      printf("Palavra escolhida através do dicionário. \n\n");

      if((pid = fork()) == 0) {
         Close(listenfd);
         fclose(fp);

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

void find_char(char * word, char c, char * returnString)
{
   int i, f=0, lenf=0;
   char temp[MAXDATASIZE];
   lenf = strlen(word);

   sprintf(returnString, "");

   for(i=0;i<lenf;i++) {
      if(word[i] == c) {
          sprintf(temp, "%d;", i+1);
          strcat(returnString, temp);
      }
   }
   if (strcmp(returnString, "") == 0)
     strcat(returnString, "0");
   strcat(returnString, "\n");
   printf("%s", returnString);
   return;
}

void doit(int connfd, struct sockaddr_in clientaddr, char *word) {
   char recvline[MAXDATASIZE + 1];
   char sendline[MAXDATASIZE + 1];
   int n;
   int position;
   char pos[2];
   socklen_t remoteaddr_len = sizeof(clientaddr);

//   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
   while (1) {
      n = read(connfd, recvline, MAXDATASIZE);
      printf("n = %d\n", n);
      recvline[n] = 0;
      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

//      printf("<%s-%d>: %s\n", inet_ntoa(clientaddr.sin_addr),(int) ntohs(clientaddr.sin_port), recvline);

      if(strcmp(recvline, EXIT_COMMAND) == 0) {
         printf("Recebeu exit.\n");
         break;
      }

      if (n == 2) { // recebeu um caractere
        find_char(word, recvline[0], sendline); //procura na palavra a letra recebida do cliente e retorna as posições encontradas
        write(connfd, sendline, strlen(sendline));
      }
      else { // recebeu uma tentativa de palavra (ou um \n sozinho invalido)
        if(strcmp(recvline, word) == 0)
          sprintf(sendline, "!\n"); // sinaliza que ganhou o jogo
        else
          sprintf(sendline, "#\n"); // sinaliza que perdeu o jogo
        write(connfd, sendline, strlen(sendline));
      }
   }
}
