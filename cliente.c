#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include "basic.h"
#include "socket_helper.h"

#define MAXLINE 4096

/* copiado do livro */
ssize_t Readline (int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n=1; n<maxlen; n++) {
        again:
        if ( (rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break; /* newline is stored, like fgets() */
        } else if (rc == 0) {
            if (n == 1)
                return (0); /* EOF, no data read */
            else
                break; /* EOF, some data was read */
        } else {
            if (errno == EINTR)
                goto again;
            return (-1); /* error, errno set by read() */
        }
    }

    *ptr = 0; /* null terminate like fgets() */
    return(n);
}

void doit(int sockfd);

int main(int argc, char **argv) {
   int    port, sockfd, opcao, retval;
   char * ip;
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;
   fd_set rset;
   struct timeval tv;

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress, Port>");
      perror(error);
      exit(1);
   }

   ip = argv[1];
   port = atoi(argv[2]);

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);

   servaddr = ClientSockaddrIn(AF_INET, ip, port);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   printf("\n\n Seja bem vindo ao jogo de forca do Nelsão!\n \n \n Escolha uma opção: \n 1) Iniciar partida simples \n 2) Ser carrasco ao iniciar partida \n 3) Multiplayer \n \n");
   scanf("%d", &opcao);

   if (opcao == 1) {
      printf("\n A partida de jogo da forca começou! \n \n");
      printf("Você possui 6 vidas.\n");
      doit(sockfd);
   }
   else {

   }
/*     else if opcao == 2 {

   }
   else {

   }*/

   exit(0);
}

void doit(int sockfd) {
    fd_set fdset;
    int maxfds;
    char recvline[MAXLINE];
    char sendline[MAXLINE];
    int lines = 0, ch = 0;

    FD_ZERO(&fdset);
//    while(1) {
        FD_SET(STDIN_FILENO, &fdset);
        FD_SET(sockfd, &fdset);
        maxfds = sockfd+1; /* stdin sempre vai ser menor do que sockfd, entao pega direto o valor de sockfd como limitante superior */
        select(maxfds, &fdset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &fdset)) { /* atividade no socket */
           if (Readline(sockfd, recvline, MAXLINE) <= 0)
               perror("readline");
           printf("O tamanho da palavra é: %s \n", recvline);
       }
       if (FD_ISSET(STDIN_FILENO, &fdset)) {
           // if (fgets(sendline, MAXLINE, stdin) != NULL) /* tem algo disponivel para enviar */
           //     write(sockfd, sendline, strlen(sendline));
           // else return; /* leu tudo que enviou */
          return;
       }
//   }
}
