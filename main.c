#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "crawler.h"
int main(int argc, char** argv){
    if (argc != 2)
    {
        fprintf(stderr, "usage %s hostname\n", argv[0]);
        exit(0);
    }
    int url_count = 0;
    /*initialize the list used to store all fetched urls*/
    struct url_list check_list= init_check_list(argv[1]);
    /*get the all but first component of host from stdin*/
    char* host_second_comp = 
    get_all_but_first_content(split_url(argv[1]).host);
    /*number used to represent the status code of each request*/
    int status_code = -1;
    /*store the actual size read in*/
    char* content_length;
    /*to check if authorization is needed,
    -1 if no need, 1 if need*/
    int auth_need = -1;
    /*loop through the list of urls got from htmls*/
    for(int i = 0; i < check_list.count; i++){
        /*stop if fetched 100 urls*/
        if(i == 100){
            break;
        }
        char* url = check_list.urls[i];
        /*split the url into host and page*/
        struct url_format formed_url;
        formed_url = split_url(url);
        struct hostent* server = get_server(formed_url.host);
        int sockfd;
        /*create the buffer used to store html*/
        char* buffer = calloc(BUF_SIZE, sizeof(char));
        if(!buffer){
            fprintf(stderr, "Calloc failed!\n");
            exit(0);
        }
        sockfd = create_socket(server);
        /*if authorization is needed, send request with the 
        authorization header, send without otherwise*/
        if(auth_need > 0){
            send_request(sockfd, server, formed_url.page, AUTH_REQUEST);
            auth_need = -1;
        }else{
            send_request(sockfd, server, formed_url.page, REQUEST);
        }
        /*count the actual size read in*/
        content_length = count_length(sockfd, buffer);
        /*if the actual size doesn't equal to the size
        shown in the Content-length header, skip this url*/
        if(check_content_length(buffer, content_length)!=0){
            close(sockfd);
            continue;
        }
        /*if the MIME type is not text/html, skip this url*/
        if(check_mime_type(buffer) != 0){
            close(sockfd);
            continue;
        }
        /*check the status code in the html file*/
        status_code = check_status_code(buffer);
        switch(status_code){
            /*for 200, 404, 410, 414, fetch the page and continue*/
            case 0:
                check_list = parseHtml(buffer, server, 
                check_list, formed_url, host_second_comp);
                break;
            /*for 503, 504, refetch this page*/
            case 2:
                break;
            /*for 301, fetch the page and 
            redirect with url found in the html*/
            case 3:
                check_list = add_check_list
                (check_list, get_new_url_for_redirect(buffer));
                break;
            /*for 401, refetch with authorization*/
            case 4:
                auth_need = 1;
                break;
            default:
                fprintf(stderr, "Status code not included\n");
                exit(0);
        }
        close(sockfd);
        if(status_code == 2 || status_code == 4){
            i--;
            continue;
        }
        free(buffer);
    }
    /*printn out the list of urls fetched and free them*/
    for(int i = 0; i < check_list.count; i++){
        /*fill the url till fully specified form*/
        check_list.urls[i] = fix_for_print(check_list.urls[i]);
        printf("%s\n", check_list.urls[i]);
        free(check_list.urls[i]);
    }
    free(check_list.urls);
    return 0;
}
