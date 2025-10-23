#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define COLOR_RED     "\x1b[31m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"

#define INFO COLOR_PURPLE "[INFO] " COLOR_RESET
#define ERROR COLOR_RED "[ERROR] " COLOR_RESET
#define USAGE COLOR_RED "[USAGE] " COLOR_RESET






int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR " Invalid number of arguments.\n");
        printf(USAGE "%s <client_name>\n", argv[0]);
        return 1;
    }

    // Perchance add an auth block followed by a command event loop 

    char* client_name = argv[1];
    printf(INFO "Client Name: %s\n", client_name);
    // Handles stupid case when running from build/ directory
    // Instead of the client directory
    FILE *file = fopen("clients.txt", "r");
    if (file == NULL) {
        file = fopen("../clients.txt", "r");
        if (file == NULL) {
        perror(ERROR "Could not open clients.txt");
        return 1;
        }
    }
    
    // Find client
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, client_name) == 0) {
            found = 1;
            break;
        }
    }
    // Perchance add client
    fclose(file);

    if (found) {
        printf(INFO "Client '%s' found in the list.\n", client_name);
    } else {
        printf(INFO " Client '%s' not found in the list.\n", client_name);
    }






    return 0;
}