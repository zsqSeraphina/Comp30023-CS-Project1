#define PROTOCOL_LEN 7
#define RELA_PROTOCOL_LEN 2
#define HOST_SEPERATE "."
#define HOST_SEPERATE_CHAR '.'
#define NULL_TERMINATE "\0"
#define NEWLINE "\n"
#define PAGE_START_CHAR '/'
#define PAGE_START "/"
#define PROTOCOL "http://"
#define RELATIVE_PROTOCOL "//"
#define HTML_NEWLINE "\r\n"
#define HTTP_PROT "http:"
#define LOCATION "Location:"
#define HTML_URL_REG "<.*[a|A].*[h|H][r|R][e|E][f|F] *\
= *[\'|\"].*[\'|\"].*>.*</[a|A]>"
#define LINE_URL_REG "[hH][rR][eE][fF] *= *[\'\"][\\:a-zA-Z0-9/\\._-]\
*\\.[html]\\+[\'\"]"
#define URL_REG "[\\:a-zA-Z0-9/\\._-]*\\.[html]\\+"

typedef struct url_list{
    int count;
    char** urls;
}url_list;

typedef struct url_format{
    char* page;
    char* host;
}url_format;


struct url_list parseHtml(char* buffer, struct hostent * server, 
struct url_list check_list, 
struct url_format formed_url, char* host_second_component);
char* string_regex(char* pattern, char* cmp, size_t nmatch, int match_num);
struct url_list init_check_list(char* input);
int check_same_url(char* ori_url, char* cmp_url);
int check_url_in_list(struct url_list check_list, char* cmp_url);
struct url_list add_check_list(struct url_list check_list, char* extract_url);
char* fix_url(char* url, char* url_second_content, 
struct url_format formed_url);
char* get_all_but_first_content(char* host);
int check_all_but_first_content(char* url, char* host_cmp);
char* get_new_url_for_redirect(char* buffer);
struct url_format split_url(char* url);
char* clean_protocal(char* url);
char* fix_for_print(char* url);
