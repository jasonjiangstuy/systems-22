#include "pipe_networking.h"


int main() {

  int to_server;
  int from_server;

  from_server = client_handshake( &to_server );
  while (1) {
    printf("what is your input (lower or uppercase)\n");
    char *array = calloc(50, sizeof(char));
    fgets(array, 50, stdin);

    if (array[49] != '\0' && array[49] != '\n'){
      printf("%s\n", "sorry input too large");
      break;
    }

    if (array[strlen(array) -1 ] == '\n'){
      array[strlen(array) -1] = '\0';
    }
    // printf("%s\n", array);
    // send to server
    write(to_server, array, 50);
    // recieve from server
    read(from_server,array, 50);
    printf("child rot13: %s\n", array);
  }
}
