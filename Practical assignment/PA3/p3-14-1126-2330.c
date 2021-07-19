#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 12014    // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    int numbytes;
    char buf[MAXDATASIZE];
    //newly add for PA3//////////
    char *pch1;
    char *pch2;
    char *filename;
    FILE *requested_file;
    int stringlen,end;
    char tmpstring[50];
    /////////////////////////////
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) {  // main accept() loop
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
        if ((numbytes=recv(new_fd,buf,MAXDATASIZE-1,0)) == -1){
            perror("recv");
            exit(1);
        }
        buf[numbytes]='\0';
        printf("Received: %s\n",buf);
        pch1 = strtok(buf," ");        //get the first word from buf, words seperated by space
        pch2 = strtok(NULL," ");       //get the subsequent word from buf
        if(strcmp(pch1,"GET")==0){        //Check whether the message is a GET message
            //remove"/"
            filename = malloc((strlen(pch2)+1)*sizeof(char));
            for(int i=1;i<strlen(pch2);i++){
                filename[i-1] = pch2[i];
            }
            filename[strlen(pch2)] = '\0';

            requested_file = fopen(filename, "rb");
            printf("filename: %s\n",filename);
            if(requested_file!=NULL){  //Check if the file, </path/name>, exists
                stringlen = sprintf(tmpstring,"File found\n");
                send(new_fd,tmpstring,stringlen,0);
               
                fseek(requested_file, 0, SEEK_END);       // move to the end of the file
                end=ftell(requested_file);                // get the position of the end of file
                stringlen=sprintf (tmpstring, "file size: %d\n",end);
                send(new_fd, tmpstring, stringlen, 0);
            }
            else{  //If the file not exists, send back ‘File not existed’string.
                stringlen = sprintf(tmpstring,"File not existed\n");
                send(new_fd,tmpstring,stringlen,0);
            }
        }
        else{  //If not a GET message, send back ‘Bad request’string
            stringlen = sprintf(tmpstring,"Bad request\n");
            send(new_fd,tmpstring,stringlen,0);
        }
        close(new_fd);
    }
    return 0;
}

