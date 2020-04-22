#define PORT_NUM 80
#define BUF_SIZE 1000000
#define PROTOCOL "http://"
#define RELATIVE_PROTOCOL "//"
#define NEWLINE "\n"
#define NULL_TERMINATE "\0"
#define PAGE_START "/"
#define HTML_NEWLINE "\r\n"
#define CARR_RETURN "\r"
#define CONT_LENG "Content-Length:"
#define CONT_TYPE "Content-Type:"
#define TYPE_PLAIN "text/plain"
#define TYPE_HTML "text/html"
#define BLANK " "
#define REQUEST "GET %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/html\r\n\
User-Agent: szhou7\r\nConnection: Close\r\n\r\n"
#define AUTH_REQUEST "GET %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/html\
\r\nUser-Agent: szhou7\r\nAuthorization: Basic c3pob3U3OnBhc3N3b3Jk\r\n\
Connection: Close\r\n\r\n"



int create_socket(struct hostent * server);
struct hostent* get_server(char* url);
void send_request(int sockfd, struct hostent * server,
char* page, char* format);
char* count_length(int sockfd, char* buffer);
int check_status_code(char* buffer);
char* clean_protocal(char* url);
char* get_content_length(char* buffer);
int check_content_length(char* buffer, char* length_count);
int check_mime_type(char* buffer);
int count_header_length(char* buffer);