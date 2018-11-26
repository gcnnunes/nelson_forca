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
#define MAX(a,b) (((a)>(b))?(a):(b))


int vidas = 6;
int tamanho;

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
  char recvline[MAXLINE];

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
  Readline(sockfd, recvline, 2); //recebe o tamanho da palavra do servidor
  tamanho = recvline[0] - '0';

  printf("\n\nSeja bem vindo ao jogo de forca do Nelsão!\n \n \nEscolha uma opção: \n1) Iniciar partida simples \n2) Ser carrasco ao iniciar partida \n3) Multiplayer \n \n");
  scanf("%d", &opcao);

  if (opcao == 1) {
    printf("\nA partida de jogo da forca começou! \n \n");
    printf("Você possui %d vidas.\n", vidas);
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
    char recvline[MAXLINE], rdline[MAXLINE];
    char sendline[MAXLINE], letra;
    int lines = 0, ch = 0;


    FD_ZERO(&fdset);
    while(vidas > 0) {
//      maxfds = sockfd + 1;
      printf("O tamanho da palavra é: %d \n", tamanho);
      printf("_");
      for (int i = 1; i < tamanho; i++) {
        printf(" _");
      }
      printf("\n\nDigite uma letra: ");
      while(true) {
        FD_SET(STDIN_FILENO, &fdset);
        FD_SET(sockfd, &fdset);
        maxfds = MAX(STDIN_FILENO, sockfd) + 1;
        select(maxfds, &fdset, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &fdset)) {
          Readline(STDIN_FILENO, rdline, MAXLINE);
          printf("\nString lida com sucesso! E ela é %s\n", rdline);
          letra = rdline[0];
          printf("Enviando %c...\n", letra);
          write(sockfd, &letra, 1);
        }
        if (FD_ISSET(sockfd, &fdset)) {
          printf("\n\nRecebi algo! Vulgo: ");
          Readline(sockfd, recvline, MAXLINE);
          printf("%s", recvline);
          break;
        }
      }
    }
}
