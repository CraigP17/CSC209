#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "socket.h"

#ifndef PORT
    #define PORT 50147
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: "
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define QUIT_MSG "quit"
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5

struct client {
    int fd;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE]; // Used to hold input from the client
    char *in_ptr; // A pointer into inbuf to help with partial reads
    struct client *next;
};


// Provided functions.
void add_client(struct client **clients, int fd, struct in_addr addr);
void remove_client(struct client **clients, int fd);

// These are some of the function prototypes that we used in our solution
// You are not required to write functions that match these prototypes, but
// you may find them helpful when thinking about operations in your program.

// Send the message in s to all clients in active_clients.
void announce(struct client **active_clients, char *s);

// Move client c from new_clients list to active_clients list.
void activate_client(struct client *c,
    struct client **active_clients_ptr, struct client **new_clients_ptr);

// Remove client from the user_list, precondition that client is already
// present in the user_list
void remove_follow(struct client **user_list, struct client *client);

// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails.
fd_set allset;

/*
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++) {
        p->message[i][0] = '\0';
    }

    // initialize list of following to NULL
    for (int i = 0; i < FOLLOW_LIMIT; i++) {
      p->following[i] = NULL;
    }

    // initialize followers list to NULL
    for (int i = 0; i < FOLLOW_LIMIT; i++) {
      p->followers[i] = NULL;
    }

    *clients = p;
}

/*
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd) {
    struct client **p;

    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        // Remove the client from other clients' following/followers lists

        // Go through each 'Following' in clients Following Lists
        // and remove (*p) client from their followers list
        for (int i = 0; i < FOLLOW_LIMIT; i++) {
          struct client *following = (*p)->following[i];
          if (following != NULL) {
            remove_follow(following->followers, (*p));
          }
        }

        // Go through each of the 'Follower's Following List and
        // Remove (*p) from client's follower's following list
        for (int j = 0; j < FOLLOW_LIMIT; j++) {
          struct client *follower = (*p)->followers[j];
          if (follower != NULL) {
            remove_follow(follower->following, (*p));
          }
        }

        // Remove the client
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr,
            "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}

// Remove a new client from the clients lists, do not need to announce because
// they are a new client
void remove_new_client(struct client **clients, int fd) {
  struct client **p;

  // Iterate to client fd, reassign the previous client's client->next
  // to c->next to remove client
  for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
      ;
  *p = ((*p)->next);
  fprintf(stdout, "Removed New Client: %d\n", fd);
}

// Move client c from new_clients list to active_clients list.
void activate_client(struct client *c,
    struct client **active_clients_ptr, struct client **new_clients_ptr) {
      struct client **p;
      // Get p to point to client c in new_clients
      for (p = new_clients_ptr; *p && (*p)->fd != c->fd; p = &(*p)->next)
          ;
      // Remove client c from new_clients by reassigning the previous
      // client's client->next to c->next
      *p = ((*p)->next);

      // Announce the new client to everyone
      char connect_msg[BUF_SIZE + 20];
      sprintf(connect_msg, "%s has connected\r\n", c->username);
      announce(active_clients_ptr, connect_msg);

      // Add client c to active_clients by setting it as head of the list
      c->next = *active_clients_ptr;
      *active_clients_ptr = c;

      // Tell serve about activation
      fprintf(stdout, "[%d] %s has been activated\n", c->fd, c->username);
    }

// Reads the Clients input from the socket, saving the string to their inbuf
// takes client_list to remove client from in case socket closed
// returns 1 if the user was removed from disconnection
int read_string_from(struct client *user, struct client **clients_list) {
  int char_left = BUF_SIZE - strlen(user->inbuf);
  int bytes_read = read(user->fd, user->in_ptr, char_left);
  if (bytes_read == -1) {
    // server error
    perror("Read Error");
    exit(1);
  } else if (bytes_read == 0) {
    // 0 bytes read, disconnected client user so remove them and announce it
    fprintf(stdout, "Client [%d] Disconnected\n", user->fd);
    if (strcmp(user->username, "\0") != 0) {
      // Active client to be removed
      char disconnect_msg[BUF_SIZE + 20];
      sprintf(disconnect_msg, "%s has disconnected\r\n", user->username);
      remove_client(clients_list, user->fd);
      announce(clients_list, disconnect_msg);
      return 1; // User removed
    }
    // New client to be removed
    remove_new_client(clients_list, user->fd);
    return 0;
  } else {
    fprintf(stdout, "[%d] Read %d Characters\n", user->fd, bytes_read);
    user->in_ptr[bytes_read] = '\0';
    // Set inbuf ptr to end. (ptr reset in checked if /r/n terminated)
    user->in_ptr = &(user->in_ptr[bytes_read]);
    return 0;
  }
}

// Write a message to a client with error and connection checking
void write_msg(struct client *client, char *message,
    struct client **clients_list) {
    if (write(client->fd, message, strlen(message)) == -1) {
      // Client Disconnected
      fprintf(stderr, "Client [%d] Disconnected from Write\n", client->fd);
      if (strcmp(client->username, "\0") == 0) {
        // Active Client, announce to users they have disconnected
        char disconnect_msg[BUF_SIZE + 20];
        sprintf(disconnect_msg, "%s has disconnected\r\n", client->username);
        remove_client(clients_list, client->fd);
        announce(clients_list, disconnect_msg);
        return;
      }
      // New client to be removed
      remove_new_client(clients_list, client->fd);
    }
}

// Check if a username is already being used by someone else
// return 1 is already taken, 0 when not
int check_taken(char *name, struct client **active_clients) {
    struct client *curr_user = *(active_clients);
    while (curr_user != NULL) {
      if (strcmp(name, curr_user->username) == 0) {
        // attempt username is taken by someone else
        fprintf(stdout, "Username %s taken\n", name);
        return 1;
      }
      curr_user = curr_user->next;
    }
    return 0;
}

// Helper function to check a client from the list of new_clients that they
// entered their username and the username is:
// not empty string, username not taken
int check_client(struct client *newbie,
    struct client **active_clients_ptr, struct client **new_clients_ptr) {

    int removed = read_string_from(newbie, new_clients_ptr);
    if (removed == 1) {
      // User disconnected
      return 0;
    }
    if (strstr(newbie->inbuf, "\r\n") == NULL) {
      // Full line not read from client, move onto next fd
      return 0;
    } else {
      // Can reset inbuf pointer for new reads
      newbie->in_ptr = &(newbie->inbuf[0]);
      // Can remove \r\n and then check validity
      newbie->inbuf[strlen(newbie->inbuf) - 2] = '\0';
      fprintf(stdout, "[%d] Finished Reading: %s\n", newbie->fd, newbie->inbuf);
      if (newbie->inbuf[0] == '\0') {
        // User entered empty string as username
        fprintf(stdout, "[%d] Empty Username Sent\n", newbie->fd);
        char *empty_msg = "Enter a Non-Empty Username\r\n";
        write_msg(newbie, empty_msg, new_clients_ptr);
        return 0;
      } else if (check_taken(newbie->inbuf, active_clients_ptr) == 1) {
        // Username already taken
        char *taken_msg = "Sorry that username is taken. Enter a new one\r\n";
        write_msg(newbie, taken_msg, new_clients_ptr);
        return 0;
      } else {
        // Username not empty and not taken, valid username
        // save their username, return 1 for passing all checks
        sprintf(newbie->username, "%s", newbie->inbuf);
        fprintf(stdout, "%s is a valid username\n", newbie->username);
        return 1;
      }
    }
}

// Send the message in s to all clients in active_clients.
void announce(struct client **active_clients, char *s) {
  struct client *curr_client = *active_clients;
  while (curr_client != NULL) {
    write_msg(curr_client, s, active_clients);
    curr_client = curr_client->next;
  }
}

// Called when a user quits, removes them from active_clients list and
// announces to the server and to users that they have quit
void quit(struct client *user, struct client **active_clients_ptr) {
  fprintf(stdout, "[%d] %s has quit\n", user->fd, user->username);
  char quit_msg[BUF_SIZE + 15];
  sprintf(quit_msg, "%s has quit\r\n", user->username);
  remove_client(active_clients_ptr, user->fd);
  announce(active_clients_ptr, quit_msg);
  fprintf(stdout, "Successfuly Removed\n");
}

// Check is a name is an active user in the user_list and return their client
// struct, else return NULL is not user with such name exists
struct client *check_exists(struct client **user_list, char *name) {
  struct client *curr_user = *user_list;
  fprintf(stdout, "Searching if %s exists\n", name);
  while (curr_user != NULL) {
    if (strcmp(curr_user->username,name) == 0) {
      fprintf(stdout, "Found active user %s\n", name);
      return curr_user;
    }
    curr_user = curr_user->next;
  }
  // No user with such name exists in list
  fprintf(stdout, "%s is not an active client\n", name);
  return NULL;
}

// check whether a client is already present in the user_list, return 1
// if the client is present, 0 if not
int check_follow(struct client **user_list, struct client *client) {
  for (int i = 0; i < FOLLOW_LIMIT; i++) {
    if (user_list[i] != NULL) {
      if (user_list[i]->fd == client->fd) {
        // client is present in the list
        return 1;
      }
    }
  }
  // client not present
  return 0;
}

// checks whether is space in the user_list follower list for a user to be
// added, returns 1 if space available, 0 is full
int check_space(struct client **user_list) {
  int i = 0;
  while (i < FOLLOW_LIMIT) {
    if (user_list[i] == NULL) {
      // Space in follower list found
      return 1;
    }
    i++;
  }
  return 0;
}

// Adds client to the user_list, precondition that user_list has an empty NULL
// spot in list to add the client
// precondition followed in use cases in follow()
void add_follow(struct client **user_list, struct client *client) {
  int i = 0;
  while (i < FOLLOW_LIMIT) {
    if (user_list[i] == NULL) {
      // Space in follower list found
      user_list[i] = client;
      return;
    }
    i++;
  }
  // No follow space, shouldnt get here by precondition
  return;
}

// Remove client from the user_list, precondition that client is already
// present in the user_list
// precondition followed in use cases in unfollow()
void remove_follow(struct client **user_list, struct client *client) {
  for (int i = 0; i < FOLLOW_LIMIT; i++) {
    if (user_list[i] != NULL) {
      if (user_list[i]->fd == client->fd) {
        // Found client, now remove them
        user_list[i] = NULL;
        return;
      }
    }
  }
}

// Called when a user wants to follow another user
// Checks follow_user exists and then adds each other to lists permitted they
// are under max limit of follows
void follow(struct client *user, char *follow_name,
  struct client **active_clients_ptr) {
    // Check user exists
    struct client *follow_user = check_exists(active_clients_ptr, follow_name);
    if (follow_user == NULL) {
      // Username not an active user
      char inactive_msg[BUF_SIZE + 15];
      sprintf(inactive_msg, "%s not found\r\n", follow_name);
      write_msg(user, inactive_msg, active_clients_ptr);
      return;
    }

    // Check if they already follow this user
    if ((check_follow(user->following, follow_user)) == 1) {
      // User already follows this client
      char follow_msg[BUF_SIZE + 30];
      fprintf(stdout, "%s already follows %s\n", user->username,
       follow_user->username);
      sprintf(follow_msg, "You already follow %s", follow_user->username);
      write_msg(user, follow_msg, active_clients_ptr);
      return;
    }
    // Check space in both follower list and following list:
    // Check for space in the follow_user's follower list
    if ((check_space(follow_user->followers)) == 0) {
      // Insufficient space in Follow_user's followers
      char max_msg[BUF_SIZE + 35];
      fprintf(stdout, "Unsuccessful follow: %s at max cap\n", follow_name);
      sprintf(max_msg, "%s is at max follower capacity\r\n", follow_name);
      write_msg(user, max_msg, active_clients_ptr);
      return;
    }
    // Check user's following for capacity
    if ((check_space(user->following)) == 0) {
      // Insufficient space in user's following
      fprintf(stdout, "Unsuccessful follow: %s at max cap\n", user->username);
      char *max_space_msg = "You are at max following capacity\r\n";
      write_msg(user, max_space_msg, active_clients_ptr);
      return;
    }

    // Add user to follow_user's followers if space
    add_follow(follow_user->followers, user);
    // Add follow_user to user's following if space
    add_follow(user->following, follow_user);
    fprintf(stdout, "%s successfuly follows %s\n", user->username, follow_name);
  }

// Called when a client wants to unfollow a user,
// checks whether that user exists, is being followed by the client
// removes each other from their respective follow/following lists
void unfollow(struct client* client, char *name, struct client **active_list) {
  // Check if user exists and save that user ptr
  struct client *user_ptr = check_exists(active_list, name);
  if (user_ptr == NULL) {
    // Username not an active user
    char inactive_msg[BUF_SIZE + 15];
    sprintf(inactive_msg, "%s not found\r\n", name);
    write_msg(client, inactive_msg, active_list);
    return;
  }

  // Check client is actually following the user
  if ((check_follow(client->following, user_ptr)) == 0) {
    // Client is not following that user
    char nfollow_msg[BUF_SIZE + 25];
    fprintf(stdout, "%s is not following %s\n", client->username, name);
    sprintf(nfollow_msg, "You are not following %s\r\n", name);
    write_msg(client, nfollow_msg, active_list);
    return;
  }

  // Remove client from user_ptr's followers list
  remove_follow(user_ptr->followers, client);
  // Remove user_ptr (name) from client's following list
  remove_follow(client->following, user_ptr);
  fprintf(stdout, "%s successfuly unfollows %s\n", client->username, name);
}

// Displays the previously sent messages of those users this user is following
void show(struct client *user, struct client **active_list) {
  // Check client exists and obtain client struct
  for (int i = 0; i < FOLLOW_LIMIT; i++) {
    if ((user->following[i]) != NULL) {
      struct client *client = user->following[i];
      int j = 0;
      // Iterate through all messages, stop when encountering first empty string
      // as that signifies no more messages due to implementation of it
      while ((j < MSG_LIMIT) && ((strcmp(client->message[j], "\0")) != 0)) {
        char client_msg[BUF_SIZE + 150];
        sprintf(client_msg, "%s: %s\r\n", client->username, client->message[j]);
        write_msg(user, client_msg, active_list);
        j++;
      }
    }
  }
  fprintf(stdout, "Messages shown\n");
}

// Send a message to all the clients in the user's Followers list if under
// message capacity (8)
void message(struct client *user, char *message, struct client **active_list) {
  int i = 0;
  int space_found = 0;
  while (i < MSG_LIMIT) {
    if (strcmp(user->message[i], "\0") == 0) {
      // Not at message capacity so can save message
      space_found = 1;
      sprintf(user->message[i], "%s", message);
      break;
    }
    i++;
  }
  if (space_found == 0) {
    fprintf(stdout, "%s at message capacity\n", user->username);
    char *no_space_msg = "You have reached message limits\r\n";
    write_msg(user, no_space_msg, active_list);
    return;
  }
  // Send message to all followers
  for (int j = 0; j < FOLLOW_LIMIT; j++) {
    if ((user->followers[j]) != NULL) {
      char full_msg[BUF_SIZE + 150];
      sprintf(full_msg, "%s: %s\r\n", user->username, message);
      write_msg(user->followers[j], full_msg, active_list);
    }
  }
  fprintf(stdout, "Message sent\n");
}

// Called when there is a message to read from an active user socket
// Calls the associated action to that command after formatting the string
void read_active_user(struct client *user, struct client **active_list) {
  int removed = read_string_from(user, active_list);
  if (removed == 1) {
    // User was removed from disconnection so dont continue
    return;
  }
  if ((strstr(user->inbuf, "\r\n")) == NULL) {
    // full line not read yet, move onto next fd
    return;
  } else {
    // Can reset inbuf pointer and remove \r\n
    user->in_ptr = &(user->inbuf[0]);
    user->inbuf[strlen(user->inbuf) - 2] = '\0';
    fprintf(stdout, "[%d] Finished Reading: %s\n", user->fd, user->inbuf);

    if (strcmp(user->inbuf, "\0") == 0) {
      // User entered empty string
      fprintf(stdout, "Empty String sent\n");
      char *empty_msg = "Enter a Non-Empty Command\r\n";
      write_msg(user, empty_msg, active_list);
      return;
    }
    char *space = strstr(user->inbuf, " ");
    if (space == NULL) {
      // One word command was entered
      fprintf(stdout, "[%d] %s sent: %s\n", user->fd, user->username,
       user->inbuf);
      if ((strcmp(user->inbuf, SHOW_MSG)) == 0) {
        fprintf(stdout, "%s wants to show\n", user->username);
        show(user, active_list);
        return;
      } else if ((strcmp(user->inbuf, QUIT_MSG)) == 0) {
        fprintf(stdout, "%s wants to quit\n", user->username);
        quit(user, active_list);
        return;
      } else {
        char invalid_msg[BUF_SIZE + 30];
        fprintf(stdout, "Invalid Command\n");
        sprintf(invalid_msg, "'%s' is an invalid command\r\n", user->inbuf);
        write_msg(user, invalid_msg, active_list);
        return;
      }
    } else {
      // user->inbuf in form: command_{(tail)?}\0
      int total_len = strlen(user->inbuf);
      int command_len = total_len - strlen(space) + 1; // "command "
      int tail_len = strlen(space);                    // "_tail"
      space[0] = '\0'; // Separate two words           // "command\0tail"

      // 2nd Arg
      char *tail = malloc(sizeof(char) * tail_len);
      if (tail == NULL) {
          perror("malloc");
          exit(1);
        }
      // strncpy(tail, &(space[1]), tail_len);            // "tail "
      // tail[tail_len] = '\0';                           // "tail\0"
      sprintf(tail, "%s", &(space[1]));

      // 1st Arg
      char command[command_len];
      sprintf(command, "%s", user->inbuf);
      // strncpy(command, user->inbuf, command_len);      // "command"
      // command[command_len] = '\0';                     // "command\0"

      // Call corresponding command
      if ((strcmp(command, FOLLOW_MSG)) == 0) {
        fprintf(stdout, "%s wants to follow %s\n", user->username, tail);
        follow(user, tail, active_list);
      } else if ((strcmp(command, UNFOLLOW_MSG)) == 0) {
        fprintf(stdout, "%s wants to unfollow %s\n", user->username, tail);
        unfollow(user, tail, active_list);
      } else if ((strcmp(command, SEND_MSG)) == 0){
        fprintf(stdout, "%s wants to send message %s\n", user->username, tail);
        message(user, tail, active_list);
      } else {
        char invalid_mg[165];
        fprintf(stdout, "Invalid Command\n");
        sprintf(invalid_mg, "'%s %s' is an invalid command\r\n", command, tail);
        write_msg(user, invalid_mg, active_list);
      }
      free(tail);
      return;
    }
  }
}

int main (int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal.
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names).
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or
    // or receive announcements.
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);
    free(server);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(1);
        } else if (nready == 0) {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr,
                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        // TODO: handle input from a new client who has not yet
                        // entered an acceptable name

                        // Clients name passes the checks and they can be
                        // activated as a client
                        fprintf(stdout, "[%d] Checking name entry\n", p->fd);
                        int check;
                        if ((check = check_client(p, &active_clients,
                           &new_clients)) == 1) {
                          activate_client(p, &active_clients, &new_clients);
                          handled = 1;
                          break;
                        }

                    }
                }

                if (!handled) {
                    // Check if this socket descriptor is an active client
                    for (p = active_clients; p != NULL; p = p->next) {
                        if (cur_fd == p->fd) {
                            // TODO: handle input from an active client
                            read_active_user(p, &active_clients);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
