/*
    Raw UDP sockets
*/
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

/*
    96 bit (12 bytes) pseudo header needed for udp header checksum calculation
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};

/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}

void sendMagicPacket(char *destinationIPAddress, int knock_port) {
  //Create a raw socket of type IPPROTO
  int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

  if(s == -1)
  {
      //socket creation failed, may be because of non-root privileges
      perror("Failed to create raw socket");
      exit(1);
  }

  //Datagram to represent the packet
  char datagram[4096] , source_ip[32] , *data , *pseudogram;

  //zero out the packet buffer
  memset (datagram, 0, 4096);

  //IP header
  struct iphdr *iph = (struct iphdr *) datagram;

  //UDP header
  struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip));

  struct sockaddr_in sin;
  struct pseudo_header psh;

  //Data part
  data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
  strcpy(data , "Arbitrary data");

  //some address resolution
  strcpy(source_ip , "152.14.93.168");

  sin.sin_family = AF_INET;
  sin.sin_port = htons(7891);
  sin.sin_addr.s_addr = inet_addr (destinationIPAddress);

  //Fill in the IP Header
  iph->ihl = 5;
  iph->version = 4;
  iph->tos = 0;
  iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + strlen(data);
  iph->id = htonl (54321); //Id of this packet
  iph->frag_off = 0;
  iph->ttl = 255;
  iph->protocol = IPPROTO_UDP;
  iph->check = 0;      //Set to 0 before calculating checksum
  iph->saddr = inet_addr ( source_ip );    //Spoof the source ip address
  iph->daddr = sin.sin_addr.s_addr;

  //Ip checksum
  iph->check = csum ((unsigned short *) datagram, iph->tot_len);

  //UDP header
  udph->source = htons (6666);
  udph->dest = htons (knock_port);
  udph->len = htons(8 + strlen(data)); //tcp header size
  udph->check = 0; //leave checksum 0 now, filled later by pseudo header

  //Now the UDP checksum using the pseudo header
  psh.source_address = inet_addr( source_ip );
  psh.dest_address = sin.sin_addr.s_addr;
  psh.placeholder = 0;
  psh.protocol = IPPROTO_UDP;
  psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );

  int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
  pseudogram = malloc(psize);

  memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
  memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udphdr) + strlen(data));

  udph->check = csum( (unsigned short*) pseudogram , psize);

  //loop if you want to flood :)
  //while (1)
  {
      //Send the packet
      if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
      {
          perror("sendto failed");
      }
      //Data send successfully
      else
      {
//          printf ("Packet Send. Length : %d \n" , iph->tot_len);
          printf ("Packet sent to port: %d\n", ntohs(udph->dest));
      }
  }
}

void main (int argc, char* argv[])
{

  if (argc != 3) {
    printf("You should only provide two arguments.\n");
    exit(1);
  }

//  printf("%s\n", argv[1]); // read configuration-file
  int knock_sequence[5] = {0, 0, 0, 0, 0};

  FILE *file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  file = fopen(argv[1], "r");

  if (file == NULL) {
    exit(1);
  }

  for (int i = 0; i < 5; i++) {
    while((read = getline(&line, &len, file)) != -1) {
//      printf("%s", line);
        knock_sequence[i] = atoi(line);
//        printf("%d\n", knock_sequence[i]); // print sequences in configuration-file
//        sleep(1);
        sendMagicPacket(argv[2], knock_sequence[i]);
        sleep(1);

    }
  }

  fclose(file);

  if(line) {
    free(line);
  }

}

//Complete
