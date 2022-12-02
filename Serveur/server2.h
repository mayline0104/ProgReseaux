#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define MAX_GROUPS 10

#define BUF_SIZE    1024
#define GROUP_NAME_SIZE 10
#define COMMAND_SIZE 7

#include "client2.h"

typedef struct
{
   Client subscribers[MAX_CLIENTS];
   char name[GROUP_NAME_SIZE];
   int subscribers_count; 
   char creator[MAX_CLIENTS];
}Group;

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void send_message_to_one_friend(Client *clients, char *receiver, Client sender, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);
static void display_users(SOCKET sock, Client *clients, int actual); 
static char *date_heure(void);
static void create_group(Group *groups, char *name, int *pactualGroup, char *creator);
static void join_group(Group *groups, char* name, Client *pclient);
static void leave_group(Group *groups, char *name, Client *pclient);
static void save_message(Client *pclient, char *message); 
static void clear_history_client(Client *pclient);
static void leave_all_groups(Group *groups, Client *pclient);
static void delete_group(Group *groups, char *groupName, Client client);


#endif /* guard */
