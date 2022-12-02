#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "server2.h"
#include "client2.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int actualGroup = 0; 
   int *pactualGroup = &actualGroup;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];
   Group groups[MAX_GROUPS]; 
   FILE *historique; 

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         
         
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }
         printf("test 1"); 
         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            printf("test 2");
            /* disconnected */
            continue;
         }
         printf("test 3");
         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);
         
         display_users(csock, clients, actual); 
         
         Client c = { csock };
         int exist = 0;
         //check if buffer already exists in the array
         for(int i=0; i<actual; i++){
            if(strcmp(clients[i].name, buffer) == 0){
               exist = 1;
            }
         }
         while (exist == 1){
            printf("test 4");
            //if it does, ask for a new name
            write_client(csock, "This name already exists, please choose another one: ");
            read_client(csock, buffer);
            exist = 0;
            for(int i=0; i<actual; i++){
            if(strcmp(clients[i].name, buffer) == 0){
               exist = 1;
            }
         }
         }
         strncpy(c.name, buffer, BUF_SIZE - 1);
         printf("buffer : %s ", buffer); 
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               printf("buffer: %s", buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
                  printf("bufffer afficher"); 
                  printf(buffer);
                  if(buffer[0] == 'p'){    
                     display_users(clients[i].sock, clients, actual);
                     write_client(clients[i].sock, "choisis le user a qui tu vas parler\n");
                     char clientchar[44];
                     int clientNUM; 
                     read_client(clients[i].sock, clientchar);
                     clientNUM = atoi(clientchar); 
                     char message [255] ; 
                     strcat(message, "conversation avec "); 
                     strcat(message, clients[clientNUM].name); 
                     write_client(clients[i].sock, message);
                     read_client(clients[i].sock, buffer);
                     while(!(buffer[0] == 'q')){
                        write_client(clients[clientNUM].sock, buffer); 
                        read_client(clients[i].sock, buffer);
                     }
                     write_client(clients[clientNUM].sock, "fin conversation");
                     write_client(clients[i].sock, "fin conversation");
                  } else {
                     if(buffer[0]=='@') {
                        //get the name of the user
                        char name[BUF_SIZE];
                        int j = 0;
                        for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                        {
                           name[j - 1] = buffer[j];
                        }
                        name[j - 1] = 0;
                        //get what is after the first space
                        char message[BUF_SIZE];
                        for(j = j + 1; j < BUF_SIZE; j++)
                        {
                           message[j - (strlen(name) + 2)] = buffer[j];
                        }
                        message[j - (strlen(name) + 2)] = 0;
                        printf("new buffer : %s", message);
                        send_message_to_one_friend(clients, name, client, actual, message, 0);
                        
                     } else {
                        if(buffer[0] == '/') {
                           char *command = NULL;                             
                           command = (char *) malloc(COMMAND_SIZE * sizeof(char));
                           strncpy(command, buffer, COMMAND_SIZE); 
                           if(!strncmp(command, "/create",COMMAND_SIZE)){
                              char *message = NULL;
                              int j = 0;
                              for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                               {}
                               message = (char *) malloc(GROUP_NAME_SIZE * sizeof(char)); 
                               strncpy(message, buffer + j + 1, GROUP_NAME_SIZE); 
                               create_group(groups, message, pactualGroup, clients[i].name);
                               free(message); 
                               message = (char *) malloc(BUF_SIZE * sizeof(char));
                               sprintf(message, "group: %s is created successfully", groups[*pactualGroup - 1].name);   
                               write_client(clients[i].sock, message);
                               free(message);
                           }
                           else if(!strncmp(command, "/join",COMMAND_SIZE - 2)){
                              char groupName[GROUP_NAME_SIZE]; 
                              int j = 0;
                              for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                               {}
                              strncpy(groupName, buffer + j + 1, GROUP_NAME_SIZE);
                              Client *pClient = &(clients[i]);
                              // static void join_group(Group *groups, char* name, Client *pclient)  
                              Group *pgroup = join_group(groups, groupName, pClient);
                              char *message = (char *) malloc(BUF_SIZE * sizeof(char));
                              if(pgroup != NULL) {
                                 sprintf(message, "joined group: %s successfully", groupName);
                                 write_client(clients[i].sock, message);
                                 sprintf(message, "[group: %s] %s: [joined group]", groupName, clients[i].name);
                                 send_message_to_all_clients(pgroup->subscribers, clients[i], pgroup->subscribers_count, message, 1);
                                 }
                              else {
                                 sprintf(message, "Can't find group: %s, try to create new group with /create", groupName);
                                 write_client(clients[i].sock, message);
                              }
                              free(message); 
                           }
                           else if(!strncmp(command, "/leave",COMMAND_SIZE - 1)) {
                              char groupName[GROUP_NAME_SIZE]; 
                              int j = 0;
                              for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                               {}
                              strncpy(groupName, buffer + j + 1, GROUP_NAME_SIZE);
                              Client *pClient = &(clients[i]);
                              // static void leave_group(Group *groups, char *name, Client *pclient)
                              //if there is no group name then leave all groups
                              if(groupName[0] == '\0') 
                              {
                                 Group *pgroup_list = leave_all_groups(groups, pClient);
                                 char *message = (char *) malloc(BUF_SIZE * sizeof(char));
                                 for(int k = 0; k < MAX_GROUPS; k++){
                                    if(strcmp(pgroup_list[k].name, "\0") ){
                                       sprintf(message, "left group: %s successfully", pgroup_list[k].name);
                                       write_client(clients[i].sock, message);
                                       sprintf(message, "[group: %s] %s: [left group]", pgroup_list[k].name, clients[i].name);
                                       send_message_to_all_clients( pgroup_list[k].subscribers, clients[i],  pgroup_list[k].subscribers_count, message, 1);
                                    }
                                 }
                                 sprintf(message, "left all groups successfully");
                                 write_client(clients[i].sock, message); 
                                 free(message); 
                              } else {
                                 Group *pgroup = leave_group(groups, groupName, pClient);
                                 char *message = (char *) malloc(BUF_SIZE * sizeof(char));
                                 if(pgroup != NULL) {
                                    sprintf(message, "left group: %s successfully", groupName);
                                    write_client(clients[i].sock, message);
                                    sprintf(message, "[group: %s] %s: [left group]", groupName, clients[i].name);
                                    send_message_to_all_clients(pgroup->subscribers, clients[i], pgroup->subscribers_count, message, 1);
                                 }
                                 else {
                                    sprintf(message, "can't leave group %s because it is not found.", groupName);
                                 }
                                 free(message); 
                              }
                           }
                           else if(!strncmp(command, "/send", COMMAND_SIZE - 2)) {
                              char groupName[GROUP_NAME_SIZE]; 
                              char message[BUF_SIZE]; 
                              int j = 0;
                              for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                               {}
                              strncpy(groupName, buffer + j + 1, GROUP_NAME_SIZE);
                              for(int k = 0; k < GROUP_NAME_SIZE ; k++) {
                                 if(groupName[k] == ' '){
                                    groupName[k] = '\0';
                                    break;  
                                 }
                              }
                              Client *pClient = &(clients[i]);
                              strcpy(message, buffer + j + 1 + strlen(groupName) + 1);
                              //add_message(messages, clients[i], message, groupName, actualMessage);
                              //actualMessage++;
                              char *messageToSend = malloc(BUF_SIZE * sizeof(char)); 
                              sprintf(messageToSend, "[group: %s] %s: %s", groupName, clients[i].name, message); 
                              for(int k = 0; k < MAX_GROUPS; k++){
                                 if(!strcmp(groups[k].name, groupName)){
                                    Client *psubscriber = &groups[k].subscribers; 
                                    for(int l =0; l < MAX_CLIENTS; l++){
                                       if(!strcmp((psubscriber+l)->name, clients[i].name)){
                                          send_message_to_all_clients(groups[k].subscribers, clients[i], groups[k].subscribers_count, messageToSend, 1);                                                 
                                          break; 
                                       }
                                    }
                                    
                                    break; 
                                 }
                              }
                              free(messageToSend); 
                           }
                           else if(!strncmp(command, "/all", COMMAND_SIZE -3)) {
                              write_client(clients[i].sock, "all groups: \n");
                              char *message= malloc(BUF_SIZE * sizeof(char));
                              char emptyGroup[GROUP_NAME_SIZE];
                              strcpy(emptyGroup, "\000\000\000\000\000\000\000\000\000");   
                              int k = 0;
                              for(k=0; k < MAX_GROUPS; k++){
                                 if(strcmp(groups[k].name, emptyGroup)){
                                    sprintf(message, "- %s.\n", groups[k].name);
                                    write_client(clients[i].sock, message); 
                                 }
                              }
                              free(message); 
                           }
                           else if(!strncmp(command, "/grpin", COMMAND_SIZE)){ 
                              write_client(clients[i].sock, "all joined groups: \n"); 
                              // groups -> subscribers -> trouve 
                              int k =0; 
                              char *message = malloc(BUF_SIZE * sizeof(char)); 
                              for(k=0; k < MAX_GROUPS; k++){
                                 int l =0;
                                 for(l=0; l < MAX_CLIENTS; l++)
                                 if(!strcmp(groups[k].subscribers[l].name, clients[i].name)){
                                    sprintf(message, "- %s.\n", groups[k].name); 
                                    write_client(clients[i].sock, message); 
                                 }
                              }
                              free(message); 
                           }
                           else if(!strncmp(command, "/users", COMMAND_SIZE)){
                              display_users(clients[i].sock, clients, actual); 
                           }

                           else if(!strncmp(command, "/clear", COMMAND_SIZE)){
                              clear_history_client(&clients[i]); 
                           }
                           else if(!strncmp(command, "/delete", COMMAND_SIZE)){
                              //delete group
                              char groupName[GROUP_NAME_SIZE];
                              int j = 0;
                              for(j = 1; j < BUF_SIZE && buffer[j] != ' '; j++)
                               {}
                              strncpy(groupName, buffer + j + 1, GROUP_NAME_SIZE);
                              delete_group(groups, groupName, clients[i]);
                           }
                           free(command); 
                        } else if(buffer[0] == '!') {
                              //display_messages(messages, clients[i], actualMessage);

                        } else {
                         send_message_to_all_clients(clients, client, actual, buffer, 0);
                        }

                     }
                     
                  }
                  
               }
               break;
            }
         }
         
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         char *date = date_heure();
         strncpy(message, date, BUF_SIZE - 1);
         free(date);
         
         if(from_server == 0)
         {
            strncat(message, sender.name, sizeof message - strlen(message) - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
      save_message(&clients[i], message); 
   }
}

static void send_message_to_one_friend(Client *clients, char *receiver, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      if(strcmp(receiver, clients[i].name) == 0)
      {
         if(from_server == 0)
         {
            char *date = date_heure();
            strncpy(message, date, BUF_SIZE - 1);
            free(date);
            strncat(message, sender.name, sizeof message - strlen(message) - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
         save_message(&clients[i], message);
         save_message(&sender, message); 
      }
   }
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

static void display_users(SOCKET sock, Client* clients, int actual){
   printf("listes des clients connectes\n");
   char *message; 
   message = malloc(BUF_SIZE * sizeof(char));  
   for(int i = 0; i < actual; i++){
      sprintf(message, "-%s. \n", clients[i].name); 
      write_client(sock,message); 
   }
   free(message); 
}

static void create_group(Group *groups, char *name, int *pactualGroup, char *creator){
   Client subscribers[MAX_CLIENTS];  
   Group groupCreated = {subscribers}; 
   strncpy(groupCreated.name, name, GROUP_NAME_SIZE);
   strncpy(groupCreated.creator, creator, MAX_CLIENTS);
   groupCreated.subscribers_count = 0; 
   groups[*pactualGroup] = groupCreated;
   *pactualGroup = *pactualGroup + 1;  
   printf("Group %s created successfully.", groupCreated.name);
}

static Group *join_group(Group *groups, char *name, Client *client){
   for(int i = 0; i < MAX_GROUPS; i++) {
      if(!strcmp(groups[i].name, name)) {
         int *pindex = &groups[i].subscribers_count; 
         Client *psubscriber = &groups[i].subscribers[*pindex]; 
         strcpy(psubscriber->name,client->name);
         psubscriber->sock = client->sock; 
         *pindex = *pindex + 1; 
         return &groups[i]; 
      }
   }
   return NULL; 
}

static Group *leave_group(Group *groups, char *name, Client *pclient){
   for(int i=0; i< MAX_GROUPS; i++) {
      if(!strcmp(groups[i].name, name)){
           // find elem to remove
           int *pindex = &groups[i].subscribers_count; 
           int indexSub = NULL; 
           for(int j=0; j < MAX_CLIENTS; j++){
            if(pclient->sock == groups[i].subscribers[j].sock) {
               indexSub = j; 
               break; 
            }
           }
            // move elements on top of removed ele to left until we get a soc = 0 
           for(int j= indexSub; j < MAX_CLIENTS - 1; j++) {
            groups[i].subscribers[j] = groups[i].subscribers[j+1];
                        if(groups[i].subscribers[j+1].sock == 0) 
               break;  
           }
           *pindex = *pindex - 1; 
           return &groups[i]; 
      }
   } 
   return NULL;     
}
static char *date_heure() {
   int h, min, day, mois, an;
   time_t now;
   // Renvoie l'heure actuelle
   time(&now);
   struct tm *local = localtime(&now);
   h = local->tm_hour;        
   min = local->tm_min;       
   day = local->tm_mday;          
   mois = local->tm_mon + 1;     
   an = local->tm_year + 1900;  
   char *date = malloc(100);
   sprintf(date, "%02d/%02d/%d %02d:%02d ", day, mois, an, h, min);
   return date;
}

static void save_message(Client *pclient, char *message) {
   char *nomFichier = malloc(BUF_SIZE * sizeof(char));
   sprintf(nomFichier, "historique_%s.txt", pclient->name); 
   pclient->historique = fopen(nomFichier, "a"); 
   fprintf(pclient->historique, "%s \n", message);
   free(nomFichier); 
   fclose(pclient->historique); 
}

static void clear_history_client(Client *pclient){
   char *nomFichier = malloc(BUF_SIZE * sizeof(char));
   sprintf(nomFichier, "historique_%s.txt", pclient->name);
   pclient->historique = fopen(nomFichier, "w"); 
   fprintf(pclient->historique, "\0");
   free(nomFichier); 
   fclose(pclient->historique); 
}

static Group *leave_all_groups(Group *groups, Client *pclient){
   Group *groupIn[MAX_GROUPS];
   int index = 0;  
   for(int i=0; i< MAX_GROUPS; i++) {
      int l = 0; 
      for(l=0; l < MAX_CLIENTS; l++)
         if(!strcmp(groups[i].subscribers[l].name, pclient->name)){
            groupIn[index++] = leave_group(groups, groups[i].name, pclient); 
         }
   }
  return groupIn; 
}

static void delete_group(Group *groups, char *groupName, Client client) {
   for(int i=0; i< MAX_GROUPS; i++) {
      if(!strcmp(groups[i].name, groupName)){
         if(strcmp(groups[i].creator, client.name)) {
            char *message = malloc(BUF_SIZE * sizeof(char));
            sprintf(message, "You can't delete the group: %s. Only the group creator can delete it.", groups[i].name);
            write_client(client.sock, message);
            free(message);
            return;
         }
         //parcourir subscribers et les supprimer du groupe
         for(int j=0; j < MAX_CLIENTS; j++) {
            if(groups[i].subscribers[j].sock != 0) {
               leave_group(groups, groupName, &groups[i].subscribers[j]);
            }
         }

         for(int j= i; j < MAX_GROUPS - 1; j++) {
            groups[j] = groups[j+1];
            if(groups[j+1].subscribers_count == 0) 
               break;  
         }
         char *message = (char *) malloc(BUF_SIZE * sizeof(char));
         sprintf(message, "group: %s deleted successfully.", groupName);
         write_client(client.sock, message);
         free(message);
         return;
      }
   }

   char *message = (char *) malloc(BUF_SIZE * sizeof(char));
   sprintf(message, "group: %s doesn't exist.", groupName);
   write_client(client.sock, message);
   free(message);
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
