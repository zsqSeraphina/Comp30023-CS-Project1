#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "socket.h"

/*this part of code was copied from lab4*/
int create_socket(struct hostent * server){
    int sockfd = 0, portno;
    struct sockaddr_in serv_addr;

    portno = PORT_NUM;


    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    bcopy(server->h_addr_list[0], 
    (char *)&serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port = htons(portno);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(0);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(0);
    }
    return sockfd;
    
}
/*get server with the url*/
struct hostent* get_server(char* url){
    struct hostent* server = gethostbyname(url);
    if (server == NULL){
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    return server;
}



/*send request to get the html file 
some part of code  may from 'https://www.lemoda.net/c/fetch-web-page/'*/
void send_request(int sockfd, struct hostent * server,
char* page, char* format){
    char header[BUF_SIZE];
    int req = snprintf(header, BUF_SIZE, format, page, server->h_name);
    if(req == -1){
        fprintf(stderr, "ERROR, header failed\n");
        exit(0);
    }
    int request = send(sockfd, header, strlen (header), 0);
    if(request == -1){
        fprintf(stderr, "ERROR, send failed\n");
        exit(0);
    }
}


/*store the html file read from the web into a string 
some part of code  may from 'https://www.lemoda.net/c/fetch-web-page/'*/
char* count_length(int sockfd, char* buffer){
    int num;
    int count = 0;
    if((num = read (sockfd, buffer, BUF_SIZE))>0){
        /*count the actual size read in*/
        count += num;
    }
    /*dealete the size header part from the size of total read in,
    -2 for new line*/
    count = count - count_header_length(buffer) - 2;
    char* length_count = calloc(6, sizeof(char));
    if(!length_count){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    /*change the counted length into string type since the
    length shown in the 'Contrnt-length' header is in string type*/
    sprintf(length_count, "%d", count);
    return length_count;
}

/*count the size of header part of the file read in*/
int count_header_length(char* buffer){
    char* temp_buffer = calloc(strlen(buffer)+1, sizeof(char));
    if(!temp_buffer){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    temp_buffer = strcpy(temp_buffer, buffer);
    char* line = strtok(temp_buffer, NEWLINE);
    int count = 0;
    /*add the size of each line untile reach the line 
    with only carriage return, which should be '\r\n',
    but the strtok cuts the '\n' off, 
    this line devides the header part and the content part*/
    while(line){
        if(strcmp(line, CARR_RETURN) == 0){
            break;
        }
        count += (strlen(line)+1)*sizeof(char);
        line = strtok(NULL, NEWLINE);
    }
    return count;
}

/*return 0 for 200, process to parse html,
return 1 for 404, 410, 414, visit failed, but still need to parse html,
return 2 for 503, 504, temporarily failed, try again,
return 3 for 301, 4 for 401, I'll deal with this later,
return -1 for any other status code*/
int check_status_code(char* buffer){
    char* temp_buffer = calloc(strlen(buffer)+1, sizeof(char));
    if(!temp_buffer){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    temp_buffer = strcpy(temp_buffer, buffer);
    /*get the first line of the html file to check the status code*/
    char* line = strtok(temp_buffer, HTML_NEWLINE);
    if(strstr(line, "200") != NULL){
        free(temp_buffer);
        return 0;
    }else if((strstr(line, "404") != NULL) || 
    (strstr(line, "410") != NULL) || (strstr(line, "414") != NULL)){
        free(temp_buffer);
        return 0;
    }else if((strstr(line, "503") != NULL) || 
    (strstr(line, "504") != NULL)){
        free(temp_buffer);
        return 2;
    }else if((strstr(line, "301") != NULL)){
        free(temp_buffer);
        return 3;
    }else if((strstr(line, "401") != NULL)){
        free(temp_buffer);
        return 4;
    }
    free(temp_buffer);
    return -1;
}


/*get the content length shown in the header*/
char* get_content_length(char* buffer){
    char* temp_buffer = calloc(strlen(buffer)+1, sizeof(char));
    if(!temp_buffer){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    temp_buffer = strcpy(temp_buffer, buffer);
    /*read through each line in the buffer
    until the 'Content-length header is found'*/
    char* line = strtok(temp_buffer, HTML_NEWLINE);
    char* count_length_header = NULL;
    while(line){
        if(strstr(line, CONT_LENG)){
            /*get the leangth part of the header*/
            strtok(line, BLANK);
            count_length_header = strtok(NULL, NULL_TERMINATE);
        }
        line = strtok(NULL, HTML_NEWLINE);
    }
    return count_length_header;
}

/*compare the counted actual content length with
the content length shown in the header*/
int check_content_length(char* buffer, char* length_count){
    char* count_length_header = strdup(get_content_length(buffer));
    int result  =  strcmp(count_length_header, length_count);
    free(length_count);
    free(count_length_header);
    return result;
}

/*check the MIME-TYPE shown in the header
since only needs to fetch url with text/html type,
retrun 0 if content type satisfies the requirment, 1 otherwise*/
int check_mime_type(char* buffer){
    char* temp_buffer = calloc(strlen(buffer)+1, sizeof(char));
    if(!temp_buffer){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    temp_buffer = strcpy(temp_buffer, buffer);
    char* line = strtok(temp_buffer, HTML_NEWLINE);
    int content_type = -1;
    /*read through each line 
    until find the line with Content-Type header*/
    while(line){
        if(strstr(line, CONT_TYPE) != NULL){
            if(strstr(line, TYPE_PLAIN) != NULL){
                content_type = 0;
                break;
            }else if(strstr(line, TYPE_HTML) != NULL){
                content_type = 0;
                break;
            }
        }
        line = strtok(NULL, HTML_NEWLINE);
    }
    free(temp_buffer);
    return content_type;
}
