#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void *handleRequestPtr(char *vargp); //whenever thread is created, each thread will execute this function
void handleRequest(int fd, char *hostname, char * port);
void readRequestHdrs(rio_t *rp, char *buf, char *hdr, char *host);
void parse_uri(char *uri, char *host, char *portnum, char *rest);
char *parse_version(char *version);


// Logic
// proxy server listens to the client request
// accept request (connfd = Accept(listenfd...))
// read GET request, parse request into method, uri, version, and then also parse uri into  hostname, portnumber, rest of path
// read all request headers and save into buffer
// make connection to the server (clientfd = open_clientfd(hostname ,portnumber))
// write to clientfd : newly concatenated GET request + request headers saved in buffer
// read server response
// write to connfd



int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;
    char handleRequestArg[sizeof(int) + 2*MAXLINE]; // a buffer for argument that will be passed to handleRequestPtr, and inside handleRequestPtr, it will be parsed before passing onto handleRequest function

    if (argc != 2) {
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen); // fill in client's socket address in clientaddr, size of socket address in clientlen. return connected descriptor that can be used to communicate with the client via UNIX IO routines
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        int *connfdp = (int *) (handleRequestArg);
        *connfdp = connfd; // put connfd inside the handleRequestArg buffer
        sprintf(handleRequestArg + sizeof(int), "%s", client_hostname); // put client_hostname inside buffer
        sprintf(handleRequestArg + sizeof(int) + MAXLINE, "%s", client_port); // put client_port inside buffer
       Pthread_create(&tid, NULL, handleRequestPtr, handleRequestArg);
       printf("Connected to (%s, %s)\n", client_hostname, client_port);
    }

    return 0;
}

/*
 * handleRequestPtr : each thread will execute this. it parses the argument into connfd, hostname, port to pass to actual handleRequest function.
 */
void *handleRequestPtr(char *vargp) {
    int connfd = *((int *)vargp);
    char *hostname = vargp + sizeof(int);
    char *port = vargp + sizeof(int)+ MAXLINE;
    Pthread_detach(pthread_self());
   handleRequest(connfd, hostname, port); 
   Close(connfd);
   return NULL;
}

// handlRequest : 
// read HTTP header and check validity
// forward it to tiny server
// listen to response from tiny
// forward it to client
void handleRequest(int connfd, char *hostname, char * port){
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; //buf here is the user buffer
    rio_t rio; // internal buffer that stores chunks of information from client ( request like GET / HTTP/1.1 )
    rio_t rio2; // internal buffer than stores chunks of information from server (html content?)
    int clientfd; // socket descriptor between proxy and server
    size_t b; // number of bytes recieved from server
    char modified_version[MAXLINE];

    char host[MAXLINE],rest[MAXLINE];
    char portnum[10];
    char hdr[MAXBUF]; // buffer for header information
    // read request line and headers
    // parse the request 
    Rio_readinitb(&rio, connfd); // associate buffer with connfd

    // read first line : GET http://...
    Rio_readlineb(&rio, buf, MAXLINE);


    sscanf(buf, "%s %s %s", method, uri, version); // gets input in this form from the client
    // check for valid HTTP request
    if (strcasecmp(method, "GET")){
         printf("method : %s not implemented\n", method);
        return;
    }

    // parse uri into host, portnum, rest 
    parse_uri(uri, host, portnum, rest);
    strncpy(modified_version, parse_version(version), MAXLINE);

    // concatenate and make new GET request to send to server
    strcat(method, " /");
    strcat(method, rest);
    strcat(method, " ");
    strcat(method, modified_version);
    strcat(method, " \n");

    // write GET request and all request header info into hdr buffer
    sprintf(hdr, "%s\n", method);
    readRequestHdrs(&rio, buf, hdr+strlen(method), host);


    // connect to the server
    clientfd = Open_clientfd(host, portnum); // establish connection with a server. now proxy and write and read! (hostname, port)

    Rio_readinitb(&rio2, clientfd);
    // send request from client to server : write request recieved from client to clientfd;
    Rio_writen(clientfd, hdr, MAXBUF); 

    while ((b = Rio_readlineb(&rio2, buf, MAXLINE)) != (size_t)NULL){ // read from tiny
        Rio_writen(connfd, buf, b); // write to client
   }

}

void readRequestHdrs(rio_t *rp,char *buf,char *hdr, char *host) {
    int host_is_given = 0;
    while(strcmp(buf, "\r\n")) {
       Rio_readlineb(rp, buf, MAXLINE);
       if (strncmp(buf, "Host", 4) == 0){
           host_is_given =1;
       }
      sprintf(hdr, "%s\n", buf);
      hdr += strlen(buf);
      
    }
    
    if (!host_is_given){
        char hostheader[] = "Host : ";
        strcat(hostheader, host);
        strcat(hostheader, "\r\n");
        sprintf(hdr, "%s\n", hostheader);
        hdr += strlen(hostheader);
    }
    char connectionheader[] = "Connection : close\r\n";
    sprintf(hdr, "%s\n", connectionheader);
    hdr += strlen(connectionheader);
    char proxyconnheader[] = "Proxy-Connection : close\r\n";
    sprintf(hdr,  "%s\n", proxyconnheader);
    hdr += strlen(proxyconnheader);
    sprintf(hdr, "%s\n", user_agent_hdr);


    return;
}

char *parse_version(char *version){
    char p[MAXLINE], c[3];
    sscanf(version, "%[^/]/%[^\n]",p, c);
    if (strncmp(c, "1.1",3)==0){
        c[2] = '0';
        strcat(p, "/");
        strcat(p,c);
        return p;
    }
    return version;

}


void parse_uri(char *uri, char *host, char* portnum, char *rest) {
    // if uri starts with http
    if (strncmp(uri, "http", 4) ==0) {
        if (sscanf(uri, "http://%[^:]:%[^/]/%[^\n]",host, portnum, rest) ==3) {
        }
        else if (sscanf(uri, "http://%[^/]/%[^\n]", host, rest) ==2) {
        }
        else if (sscanf(uri, "http://%[^:]:%[^\n]", host, portnum) ==2){
        }
        else if (sscanf(uri, "http://%[^/]/", host) == 1){
            strcat(portnum, "80");
            strcat(rest, "");
        }

    }
}
