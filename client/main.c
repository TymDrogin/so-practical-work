#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("[ERROR] Invalid number of arguments.\n");
        printf("Usage: %s <client_name>\n", argv[0]);
        return 1;
    }

    char* client_name = argv[1];
    printf("Client Name: %s\n", client_name);


    FILE *file = fopen("clients.txt", "r");
    if (file == NULL) {
        perror("[ERROR] Could not open clients.txt");
        return 1;
    }
    
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, client_name, strlen(client_name)) == 0) {
            found = 1;
            break;
        }
    }

    if (found) {
        printf("Client '%s' found in the list.\n", client_name);
    } else {
        printf("Client '%s' not found in the list.\n", client_name);
    }






    return 0;
}