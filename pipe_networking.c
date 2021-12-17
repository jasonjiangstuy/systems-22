#include "pipe_networking.h"

static void sighandler(int signo) {
  if ( signo == SIGINT ){
    remove("mario");
    exit(0);
  }
}


/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
  signal(SIGINT, sighandler);
  int b, from_client;
  printf("[server] handshake: making wkp\n");
  b = mkfifo(WKP, 0600);
  if ( b == -1 ) {
    printf("mkfifo error %d: %s\n", errno, strerror(errno));
    exit(-1);
  }

  //open & block
  from_client = open(WKP, O_RDONLY, 0);
  //remove WKP
  printf("[server] handshake: removed wkp\n");
  remove(WKP);

  if (fork()){
    // parent
    close(from_client);
  }else{
    // child
    server_connect(from_client);

    exit(0);
  }
  return from_client;
}

/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
  //read initial message
  int b;
  char buffer[HANDSHAKE_BUFFER_SIZE];
  //read initial message
  b = read(from_client, buffer, sizeof(buffer));
  int to_client  = 0;
  to_client = open(buffer, O_WRONLY, 0);
  //create SYN_ACK message
  srand(time(NULL));
  int r = rand() % HANDSHAKE_BUFFER_SIZE;
  sprintf(buffer, "%d", r);

  write(to_client, buffer, sizeof(buffer));
  //rad and check ACK
  read(from_client, buffer, sizeof(buffer));
  int ra = atoi(buffer);
  if (ra != r+1) {
    printf("[server] handshake received bad ACK: -%s-\n", buffer);
    exit(0);
  }//bad response
  printf("[server] handshake received: -%s-\n", buffer);

  // do the thing
  printf("\n%s\n\n", "HandShake Established");
  char *array = calloc(50, sizeof(char));
  while (read(from_client,array, 50)) {
    // rot13
    int i = 0;
    int boundleft;
    int boundright;
    for (; i < strlen(array); i++){
      // used to exceed the char size limit of 127
      int target = array[i];
      int charater = 0;
      if (65 <= target && target <= 90){
        // cap charaters
        boundleft = 65;
        boundright = 90;
        charater = 1;

      }else if(97 <= target && target <= 122){
        // lower charaters
        boundleft = 97;
        boundright = 122;
        charater = 1;
      }
      target = target + 13;
      if (charater){
        if (boundright - target < 0){
          // pass the alphabet
          // wrap around
          target = boundleft + target - boundright - 1;
        }
      }

      array[i] = target;
    }
    write(to_client, array, 50);
  }
  printf("\n%s\n\n", "Client Exited");
  // read returns 0
  free(array);

  return to_client;
}


// /*=========================
//   server_handshake
//   args: int * to_client
//
//   Performs the server side pipe 3 way handshake.
//   Sets *to_client to the file descriptor to the downstream pipe.
//
//   returns the file descriptor for the upstream pipe.
//   =========================*/
// int server_handshake(int *to_client) {
//
//   int b, from_client;
//   char buffer[HANDSHAKE_BUFFER_SIZE];
//
//   printf("[server] handshake: making wkp\n");
//   b = mkfifo(WKP, 0600);
//   if ( b == -1 ) {
//     printf("mkfifo error %d: %s\n", errno, strerror(errno));
//     exit(-1);
//   }
//   //open & block
//   from_client = open(WKP, O_RDONLY, 0);
//   //remove WKP
//   remove(WKP);
//
//   printf("[server] handshake: removed wkp\n");
//
//
//   *to_client = open(buffer, O_WRONLY, 0);
//   //create SYN_ACK message
//   srand(time(NULL));
//   int r = rand() % HANDSHAKE_BUFFER_SIZE;
//   sprintf(buffer, "%d", r);
//
//   write(*to_client, buffer, sizeof(buffer));
//   //rad and check ACK
//   read(from_client, buffer, sizeof(buffer));
//
//   return from_client;
// }


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[HANDSHAKE_BUFFER_SIZE];
  char ppname[HANDSHAKE_BUFFER_SIZE];

  //make private pipe
  printf("[client] handshake: making pp\n");
  sprintf(ppname, "%d", getpid() );
  mkfifo(ppname, 0600);

  //send pp name to server
  printf("[client] handshake: connecting to wkp\n");
  *to_server = open( WKP, O_WRONLY, 0);
  if ( *to_server == -1 ) {
    printf("open error %d: %s\n", errno, strerror(errno));
    exit(1);
  }

  write(*to_server, ppname, sizeof(buffer));
  //open and wait for connection
  from_server = open(ppname, O_RDONLY, 0);

  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  printf("[client] handshake: received -%s-\n", buffer);

  //remove pp
  remove(ppname);
  printf("[client] handshake: removed pp\n");

  //send ACK to server
  int r = atoi(buffer) + 1;
  sprintf(buffer, "%d", r);
  write(*to_server, buffer, sizeof(buffer));

  return from_server;
}
