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
#define EXIT_COMMAND "exit\n"

#define MAXLINE 4096
#define MAX(a,b) (((a)>(b))?(a):(b))


int vidas = 6;
int tamanho = 0;
char letra;

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
  Readline(sockfd, recvline, MAXLINE); //recebe o tamanho da palavra do servidor
  sscanf(recvline, "%d", &tamanho);

  printf("\n\nSeja bem vindo ao jogo de forca do Nelsão!\n \n \nEscolha uma opção: \n1) Iniciar partida simples \n2) Ser carrasco ao iniciar partida \n3) Multiplayer \n \n");
  scanf("%d", &opcao);

  if (opcao == 1) {
    printf("\nA partida de jogo da forca começou! \n \n");
    doit(sockfd);
  }
  else {
    printf("Desculpe, esta opção não está implementada.\n");
    write(sockfd, EXIT_COMMAND, strlen(EXIT_COMMAND));
  }
   exit(0);
}

void doit(int sockfd) {
    fd_set fdset;
    int maxfds, i, j, letrasRestantes = tamanho;
    char recvline[MAXLINE], rdline[MAXLINE], temp[MAXLINE];
    char sendline[MAXLINE], vetorPalavra[tamanho];
    int lines = 0, ch = 0, position = 0;
    bool esperandoResposta = false;


    FD_ZERO(&fdset);

    for (int i = 0; i < tamanho; i++)
      vetorPalavra[i] = '_';


    while(vidas > 0) {
      printf("\n============================================\n\n");
      printf("O tamanho da palavra é: %d \n", tamanho);
      printf("Você possui %d vidas.\n", vidas);


      for (int i = 0; i < tamanho; i++) {
        printf("%c ", vetorPalavra[i]);
      }
      if (letrasRestantes == 0) {
        printf("\n\nParabéns! Você venceu o jogo!\n");
        write(sockfd, EXIT_COMMAND, strlen(EXIT_COMMAND));
        return;
      }
      printf("\n\nDigite uma letra:\n");

      while(true) {
        FD_SET(STDIN_FILENO, &fdset);
        FD_SET(sockfd, &fdset);
        maxfds = MAX(STDIN_FILENO, sockfd) + 1;
        select(maxfds, &fdset, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &fdset) && !esperandoResposta) {
          Readline(STDIN_FILENO, rdline, MAXLINE);
          write(sockfd, rdline, strlen(rdline));
          if (strcmp(rdline, EXIT_COMMAND) == 0)
            return;
          letra = rdline[0];
          esperandoResposta = true;
        }
        if (FD_ISSET(sockfd, &fdset) && esperandoResposta) {
          Readline(sockfd, recvline, MAXLINE);
          i = 0;
          while (recvline[i] != '\n') {
            if (recvline[i] == '#') {
              vidas = -1;
              printf("Você perdeu o jogo!\n");
              write(sockfd, EXIT_COMMAND, strlen(EXIT_COMMAND));
            }
            else if (recvline[i] == '!') {
              vidas = -1;
              printf("Parabéns! Você venceu o jogo!\n");
              write(sockfd, EXIT_COMMAND, strlen(EXIT_COMMAND));

            }
            else if (recvline[i] == ';') {
              i++;
              continue;
            }
            else if (recvline[i] == '*') {
              vidas--;
              printf("Letra inválida!\n");
            }
            else if (recvline[i] == '0') { // letra chutada não existe na palavra
              vidas--;
            }
            else {
              j = 0;
              memset(temp, 0, MAXLINE);
              temp[j] = recvline[i];
              while ((recvline[i+1] != ';') && (recvline[i+1] != '\n')) {      //lógica para posições com mais de um digito           
                i++;
                j++;
                temp[j] = recvline[i];
              }
              sscanf(temp, "%d", &position);
              vetorPalavra[position-1] = letra;
              letrasRestantes--;
            }
            i++;
          }
          esperandoResposta = false;
          break;
        }
        // fim do while true
      }
    }
    if (vidas != -1) {
      printf("Você perdeu o jogo!\n");
      write(sockfd, EXIT_COMMAND, strlen(EXIT_COMMAND));
    }
}
