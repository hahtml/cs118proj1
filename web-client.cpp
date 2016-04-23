#include "HttpHeader.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int main(int argc, char* argv[])
{
  int port = 80; //default HTTP port number

  if (argc != 2) {
    fprintf(stderr, "usage: web-client [URL]\n");
    exit(1);
  }
  string url(argv[1]);
  
  HttpRequest request(url);
  string host = request.getField("host");
  string hostWithoutPort = host;

  size_t pos = host.find(":");
  if (pos != string::npos) {
    pos++;
    port = atoi(host.substr(pos, host.length()-pos).c_str());
    hostWithoutPort = host.substr(0, pos-1);
  }
  
  struct hostent* hptr;
  hptr = gethostbyname2(hostWithoutPort.c_str(), AF_INET);
  if (!hptr) {
    fprintf(stderr, "unknown host\n");
    exit(1);
  }
  
  struct sockaddr_in servaddr;  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  memcpy(&(servaddr.sin_addr.s_addr), hptr->h_addr, hptr->h_length);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if ((connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) {
    perror("connect");
    exit(1);
  }
  
  char ipstr[INET_ADDRSTRLEN];
  inet_ntop(servaddr.sin_family, &servaddr.sin_addr, ipstr, sizeof(ipstr));
  printf("connect to host <%s> with IP <%s>\n", host.c_str(), ipstr);

  wire_t outputWire = request.encodeMsg(""); //no entity required for GET
  if (send(sockfd, (char*)&outputWire[0], outputWire.size(), 0) < 0) {
    perror("send");
    exit(1);
  }  
  
  char inputbuf[MAXSIZE];
  ssize_t len;
  if((len = recv(sockfd, inputbuf, sizeof(inputbuf), 0)) < 0) {
    perror("recv");
    exit(1);
  }
  
  wire_t inputWire;  
  insertIntoWire(&inputWire, inputbuf, len);
  HttpResponse response;  
  string inputEntity = response.consume(inputWire);

  if (response.getStatus() != "200")
    return 0;

  const char* path = request.getPath().c_str();
  char* filename = strrchr(const_cast<char*>(path), '/') + 1;
  FILE* fp = fopen(filename, "w+");
  fwrite(inputEntity.c_str(), sizeof(char), inputEntity.size(), fp);
  fclose(fp);
  
  close(sockfd);
  return 0;
}
