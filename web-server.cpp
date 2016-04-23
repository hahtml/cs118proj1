#include "HttpHeader.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>
#include <stdio.h>
#include <errno.h>
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

#define STATUS_404 "Not Found"
#define STATUS_200 "OK"

void* conn_handler(void* arg);

int main(int argc, char* argv[])
 {
  //default settings:
  string host = "127.0.0.1", dir = ".";
  int port = 4000;
  
  //error message for wrong number of arguments
  if (argc > 4) {
    fprintf(stderr, "usage: web-server [hostname] [port] [file-dir]\n");
    exit(1);
  }

  //assign arguments to host, port and dir
  switch(argc) {
    case 4:
      dir = argv[3];
    case 3:
      port = atoi(argv[2]);
    case 2:
      if (strcmp(argv[1], "localhost") == 0) break;
      host = argv[1];
  }

  //declaraion for socketfd and address pointer
  int listenfd, *connfd;
  struct sockaddr_in servaddr, cliaddr;
  socklen_t cliaddr_len = sizeof(cliaddr);
  char ipstr[INET_ADDRSTRLEN];
  pthread_t thread_id;

  //create listening socket using TCP
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("failed to create socket");
    exit(1);
  }
  
  //allow others to reuse the address
  int yes = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  //declare and initialize the address struct
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(host.c_str());
  
  //bind address to socket
  if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind");
    exit(1);
  }

  //set socket to listen status
  if (listen(listenfd, 2) < 0) {
    perror("listen");
    exit(1);
  }  

  printf("Server running on IP <%s> at PORT <%d> from directory <%s>\n", host.c_str(), port, dir.c_str());
  cout << "waiting for incoming connection" << endl;
  
  while (1) {
    //dynamically allocate memory for connfd to avoid race condition
    connfd = (int*)malloc(sizeof(int));
    if ((*connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddr_len)) < 0) {
      perror("accept");
      continue;
    }

    printf("new connection is received from IP <%s> at PORT <%d>\n",
	   inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ipstr, sizeof(ipstr)),
	   ntohs(cliaddr.sin_port));

    pthread_create(&thread_id, NULL, &conn_handler, (void*)connfd);
    pthread_detach(thread_id);
  }
}

void* conn_handler(void* arg) 
{
  int connfd = *(int*)arg;  
  char inputbuf[MAXSIZE];
  
  ssize_t len;
  if((len = recv(connfd, inputbuf, sizeof(inputbuf), 0)) < 0) {
    perror("recv");
    exit(1);
  } 

  wire_t inputWire;
  insertIntoWire(&inputWire, inputbuf, len);  
  HttpRequest request;
  string inputEntity = request.consume(inputWire);

  const char* path = request.getPath().c_str();
  //remove the leading '/' from the path
  path++;

  HttpResponse response;  
  response.setVersion(request.getVersion());  

  char* filebuf = (char*)malloc(sizeof(char));
  *filebuf = '\0';
  if (strlen(path) == 0) { //path is empty
    fprintf(stderr, "path is empty\n");
    response.setStatus("404");
    response.setStatusMsg(STATUS_404);    
  }
  else {
    FILE* fp = fopen(path, "r"); 
    if(!fp) {
      perror("fopen");
      response.setStatus("404");
      response.setStatusMsg(STATUS_404);
    }
    else {
      //put the cursor at the end
      fseek(fp, 0, SEEK_END);
      //determine the length of the file
      long fileLen = ftell(fp);
      //put the cursor back to the start point
      fseek(fp, 0, SEEK_SET);
      filebuf = (char*)realloc(filebuf, sizeof(char) * (fileLen + 1));
      //pass the content to filebuf
      size_t num_bytes = fread(filebuf, sizeof(char), fileLen, fp);
      filebuf[num_bytes] = '\0';
	
      fclose(fp);
      response.setStatus("200");
      response.setStatusMsg(STATUS_200);

      char* dot = strrchr (const_cast<char*>(path), '.');
      if (dot) {
	dot++; //remove the leading '.'
	string ext(dot);
	response.setField("Content-Type", ext);
      }
    }
  }
  string outputEntity(filebuf);
  wire_t outputWire = response.encodeMsg(outputEntity);
  free(filebuf);

  if(send(connfd, (char*)&outputWire[0], outputWire.size(), 0) < 0) {
    perror("send");
    exit(1);
  }

  close(connfd);
  free(arg);
  printf("connection on thread ID <%lu> is closed\n", pthread_self());
  return 0;
}
