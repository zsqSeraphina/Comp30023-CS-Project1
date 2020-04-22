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
#include <regex.h>

#include "crawler.h"

/*extract urls satisfies the requirement from the html*/
struct url_list parseHtml(char* buffer, struct hostent * server, 
struct url_list check_list, struct url_format formed_url, char* host_cmp){
    /*read through each line from the html
    lines are splitted by \r\n*/
    char* line = strtok(buffer, HTML_NEWLINE);
    while(line){
        /*firstly, get the line that may include the url*/
        char* get_from_line = string_regex(HTML_URL_REG, line, 10, 100);
        /*secondly, get the string 'href=' and things inside the quote*/
        char* get_url_line = string_regex(LINE_URL_REG, get_from_line, 1, 50);
        /*finally, get the string inside of the quote*/
        char* extract_url = string_regex(URL_REG, get_url_line, 1, 10);
        /*if such string satisfies the 3 above formats can be found*/
        if(extract_url){
            /*some urls with form like 'dog.html'
            needs to fill up the host part*/
            extract_url = fix_url(extract_url, host_cmp, formed_url);
            /*check if the extracted url has all but first 
            component same as the url from stdin*/
            if(check_all_but_first_content(extract_url, host_cmp) == 0){
                /*add this url into the list of urls
                if its not already in there*/
                if(check_url_in_list(check_list, extract_url) != 0){
                    check_list = add_check_list(check_list, extract_url);
                }
            }
            
        }
        free(get_from_line);
        free(get_url_line);
        line = strtok(NULL, HTML_NEWLINE);
    }
    return check_list;
}


/*check if a tring match a pattern, 
return the string if found, return NULL otherwise*/
char* string_regex(char* pattern, char* cmp, size_t nmatch, int match_num){
    regex_t regex;
    regmatch_t match[match_num];
    int return_value;
    char* cmp_buffer = NULL;
    /*compile the regex pattern and place it to regex*/
    return_value = regcomp(&regex, pattern, 0);
    if(return_value != 0){
        fprintf(stderr, "Error, Regex compilation failed!");
        exit(0);
    }
    /*check if the string going to compare exists*/
    if(cmp){
        /*use the pattern to find the result from the string*/
        return_value = regexec(&regex, cmp, nmatch, match, 0);
        if(return_value != 0 && return_value != REG_NOMATCH){
            fprintf(stderr, "Error, regex error!");
            exit(0);
        }else if(return_value == 0){
            /*store the string between the start and end point 
            in the input string if a match is found*/
            cmp_buffer = 
            calloc((match[0].rm_eo-match[0].rm_so+1), sizeof(char));
            if(!cmp_buffer){
                fprintf(stderr, "Calloc failed!\n");
                exit(0);
            }
            strncpy(cmp_buffer, 
            cmp+match[0].rm_so, match[0].rm_eo-match[0].rm_so);
        }
    }
    return cmp_buffer;
}

/*initialize the check list with the url from stdin,
use for storing the urls fetched*/
struct url_list init_check_list(char* input){
    struct url_list list;
    list.urls = calloc(strlen(input), sizeof(char*));
    if(!list.urls){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    list.urls[0] = strdup(input);
    list.count = 1;
    return list;
}


/*add the url found in html file into the list of urls*/
struct url_list add_check_list(struct url_list check_list, char* extract_url){
    char** temp = check_list.urls;
    temp = realloc(check_list.urls, (check_list.count+1)*sizeof(char*));
    if(!temp){
        fprintf(stderr, "Realloc failed!\n");
        exit(0);
    }
    check_list.urls = temp;
    check_list.urls[check_list.count] = strdup(extract_url);
    check_list.count++;
    return check_list;
}


/*check if two urls goes to the same page,
0 means they are the same, 1 means not*/
int check_same_url(char* ori_url, char* cmp_url){
    /*only compare the host and page part*/
    ori_url = clean_protocal(ori_url);
    cmp_url = clean_protocal(cmp_url);
    if(strcmp(ori_url, cmp_url) == 0){
        return 0;
    }
    return 1;
}

/*check if the url already has a same url in the list
0 means it already exists, 1 means not*/
int check_url_in_list(struct url_list check_list, char* cmp_url){
    for(int i = 0; i < check_list.count; i++){
        if(check_same_url(check_list.urls[i], cmp_url) == 0){
            return 0;
        }
    }
    return 1;
}

/*fix the urls like 'dog.html' with host and path 
from the url where it is been found*/
char* fix_url(char* url, char* url_second_content, 
struct url_format formed_url){
    char* host = calloc(strlen(formed_url.host)+1, sizeof(char));
    if(!host){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    strcpy(host, formed_url.host);
    /*if the url does not has a host part same as from the stdin
    add the front part to the url*/
    if(strstr(url, url_second_content) == NULL){
        char* page;
        char* temp_page = formed_url.page;
        char* path = calloc(strlen(temp_page)+10, sizeof(char));
        if(!path){
            fprintf(stderr, "Calloc failed!\n");
            exit(0);
        }
        page = strrchr(temp_page, PAGE_START_CHAR);
        strncpy(path, temp_page, strlen(temp_page)-strlen(page));
        strcat(host, path);
        strcat(host, PAGE_START);
        strcat(host, url);
        return host;
    }
    free(host);
    return url;
}

/*get the part of host other than the first part*/
char* get_all_but_first_content(char* host){
    /*if the host hast a '.' in it, get the part after the '.'*/
    if(strstr(host, HOST_SEPERATE) != NULL){
        char* h_name_temp = malloc((strlen(host)+1)*sizeof(char));
        if(!h_name_temp){
            fprintf(stderr, "Malloc failed!\n");
            exit(0);
        }
        strcpy(h_name_temp, host);
        char* token = strtok(h_name_temp, HOST_SEPERATE);
        token = strtok(NULL, NULL_TERMINATE);
        memset(host, 0, strlen(host));
        strcpy(host, token);
        free(h_name_temp);
    }
    return host;
}

/*check if the url is all but the frst components 
of the host match the host of the URL on the command line
return 0 if match, 1 if not match*/
int check_all_but_first_content(char* url, char* host_cmp){
    char* host = split_url(url).host;
    if(strstr(host, HOST_SEPERATE)){
        host = strchr(url, HOST_SEPERATE_CHAR);
        host += 1;
    }
    if(strstr(host, host_cmp)){
        return 0;
    }else{
        return 1;
    }
}


/*get the url in the html file when redirect is needed*/ 
char* get_new_url_for_redirect(char* buffer){
    char* temp_buffer = calloc(strlen(buffer)+1, sizeof(char));
    if(!temp_buffer){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    temp_buffer = strcpy(temp_buffer, buffer);
    char* line = strtok(temp_buffer, HTML_NEWLINE);
    char* extract_url = NULL;
    /*read through each line in the html until the 
    Location header is found*/
    while(line){
        if(strstr(line, LOCATION) != NULL){
            /*use regex to get the url from the line with Location header*/
            extract_url = string_regex(URL_REG, line, 10, 10);
            if(extract_url == NULL){
                fprintf(stderr, "Error, redirect link not found!\n");
                exit(0);
            }
            return extract_url;
        }
        line = strtok(NULL, HTML_NEWLINE);
        if(line == NULL && temp_buffer != NULL){
            fprintf(stderr, "Error, location header not found!\n");
            exit(0);
        }
    }
    return extract_url;
}


/*split the url from stdin into the host and page two parts*/
struct url_format split_url(char* url){
    char* temp = strdup(url);
    char* temp_url = clean_protocal(temp);
    struct url_format formed_url;
    /*cut the page part and
    *store it in another string*/
    char* page_buf = NULL;
    char* host_buf = strdup(url);
    if(strstr(temp_url, PAGE_START) != NULL){
        page_buf = strchr(temp_url, PAGE_START_CHAR);
        memset(host_buf, 0, strlen(host_buf));
        strncpy(host_buf, temp_url, strlen(temp_url)-strlen(page_buf));
    }
    char* page;
    /*if the url has no page part, use '/' as page part*/
    if(page_buf == NULL){
        page = PAGE_START;
    }else{
        page = calloc(strlen(page_buf)+2, sizeof(char));
        if(!page){
            fprintf(stderr, "Calloc failed!\n");
            exit(0);
        }
        strcat(page, page_buf);
    }
    /*add the page and host into the struct*/
    formed_url.host = host_buf;
    formed_url.page = page;
    if(formed_url.host == NULL || formed_url.page == NULL){
        fprintf(stderr, "Split failed!\n");
        exit(0);
    }
    free(temp);
    return formed_url;
}


/*clean up the protoccal part of the url*/
char* clean_protocal(char* url){
    char* temp_url = strdup(url);
    /*remove the protocol part of the url
    depends on if it's start with 'http://' or '//'*/
    if(strstr(temp_url, PROTOCOL)!=NULL){
        temp_url += PROTOCOL_LEN;
    }else if(strstr(temp_url, RELATIVE_PROTOCOL)!=NULL){
        temp_url += RELA_PROTOCOL_LEN;
    }
    return temp_url;
}


/*fill the url up till fully specified form*/
char* fix_for_print(char* url){
    /*allocate 8 more size since the url may miss up until 7 
    chars(http//:) and one for \0*/
    char* temp_url = calloc(strlen(url)+8, sizeof(char));
    if(!temp_url){
        fprintf(stderr, "Calloc failed!\n");
        exit(0);
    }
    /*if the url does not have the whole http://, add to it*/
    if(strstr(url, PROTOCOL) == NULL){
        if(strstr(url, RELATIVE_PROTOCOL) == NULL){
            strcpy(temp_url, PROTOCOL);
            strcat(temp_url, url);
            char* temp = url;
            url = temp_url;
            free(temp);
            return url;
        }else{
            /*if the url does not have the http:, add to it*/
            strcpy(temp_url, HTTP_PROT);
            strcat(temp_url, url);
            char* temp = url;
            url = temp_url;
            free(temp);
            return url;
        }
    }
    free(temp_url);
    return url;
}
