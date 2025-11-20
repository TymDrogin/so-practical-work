#include "vehicle.h"


vehicle_t* create_vehicle(id_generator* g, char* client_name, char* destination, int distance_to_travel) {
    if (g == NULL || client_name == NULL || destination == NULL || distance_to_travel <= 0) {
        perror(ERROR "Invalid arguments have been passed to the create_vehicle");
        exit(EXIT_FAILURE);

    }
    vehicle_t* v = (vehicle_t*)malloc(sizeof(vehicle_t));
    if(v == NULL) {
        perror(ERROR "Could not allocate memory for the vehicle");
        exit(EXIT_FAILURE);
    }

    // Generate vehicle ID
    v->id = generate_id(g);  

    v->client_name = strdup(client_name);
    if (v->client_name == NULL) {
        perror(ERROR "Could not allocate memory for client_name");
        exit(EXIT_FAILURE);
    }
    v->destination = strdup(destination);
    if (v->destination == NULL) {
        perror(ERROR "Could not allocate memory for destination");
        exit(EXIT_FAILURE);
    }
    v->distance_to_travel = distance_to_travel;

    v->is_performing_service = false;

    // Spin the vehicle process
    run_vehicle_process(v);

    return v;
}
void print_vehicle_info(const vehicle_t* v) {
    if (v == NULL) {
        printf("[VEHICLE] (null pointer)\n");
        return;
    }

    printf("=== VEHICLE INFO ===\n");
    printf("ID: %d\n", v->id);
    printf("PID: %d\n", v->pid);

    printf("Client Name: %s\n", v->client_name ? v->client_name : "(null)");
    printf("Destination: %s\n", v->destination ? v->destination : "(null)");

    printf("Distance To Travel: %d\n", v->distance_to_travel);
    printf("Distance Traveled: %d\n", v->distance_traveled);

    printf("Is Alive: %s\n", v->is_alive ? "true" : "false");
    printf("Is Performing Service: %s\n", v->is_performing_service ? "true" : "false");

    printf("Pipe FD: [%d, %d]\n", v->fd[0], v->fd[1]);
    printf("====================\n");
}

void start_vehicle_service(vehicle_t* v) {
    if(v == NULL) {
        perror(ERROR "start_vehicle: Passed a null pointer to the vehicle");
        exit(EXIT_FAILURE);
    }
    if(!v->is_alive) {
        perror(ERROR "start_vehicle: Attempted to start vehicle with the alive flag 0");
        exit(EXIT_FAILURE);

    }
    if(v->is_performing_service) {
        perror("start_vehice: Attempted to start the vehicle that is already performing the service");
        exit(EXIT_FAILURE);
    }
    if(v->distance_to_travel <= 0) {
        perror(ERROR "Bad distance to travel has been set");
        exit(EXIT_FAILURE);
    }

    // Start the vehicle
    if(kill(v->pid, SIGUSR2) == -1) {
        perror("start_vehicle: failed to send SIGUSR2");
        exit(EXIT_FAILURE);
    }

    v->is_performing_service = true;
    return;

};
void cancel_vehicle_service(vehicle_t* v) {
    if(v == NULL) {
        perror(ERROR "cancel_vehicle_service: Got NULL pointer to the vehicle");
        exit(EXIT_FAILURE);
    }
    if(!v->is_alive) {
        perror(ERROR "cancel_vehicle_servise: Can't stop the vehicle that has been already stopped");
        exit(EXIT_FAILURE);
    }
    v->is_alive = false;
    v->is_performing_service = false;
    kill(v->pid, SIGUSR1);
    waitpid(v->pid, NULL, 0);

}
// Forks + execs
void run_vehicle_process(vehicle_t* v) {
    if (v == NULL) {
        perror(ERROR "Run vehicle function got null");
        exit(EXIT_FAILURE);

    }

    //if(pipe(v->fd) != 0) {
    //    perror(ERROR "Could not create an annonimous pipe for the vehicle");
    //    exit(EXIT_FAILURE);
    //}

    pid_t pid = fork();
    if (pid == 0) {

        // Close the read end, child only writes
        //close(v->fd[0]);

        //// Redirect stdout → pipe write end
        //if (dup2(v->fd[1], STDOUT_FILENO) < 0) {
        //    perror("dup2");
        //    exit(EXIT_FAILURE);
        //}

        //// After duplicating, close original fd
        //close(v->fd[1]);

        char distance[20], id[20];
        snprintf(distance, sizeof(distance), "%d", v->distance_to_travel);
        snprintf(id, sizeof(id), "%d", v->id);

        char* args[] = {
            "vehicle", 
            v->client_name, 
            v->destination, 
            distance, 
            id, 
            NULL,
        };  

        char envbuf[64];
        snprintf(envbuf, sizeof(envbuf), "TIMER_TICK_SPEED_MILLISECONDS=%d", TIMER_TICK_SPEED_MILLISECONDS);
        char* envp[] = {
            envbuf,
            NULL
        };

        execve(PATH_TO_VEHICLE_EXECUTABLE, args, envp);

        perror(ERROR "Exec failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent 
        v->pid = pid;
        v->is_alive = true;

        // Parent closes the write end
        //close(v->fd[1]);
        return;
    }
}

float get_percentage_of_distance_traveled(const vehicle_t* v) {
    return (float)v->distance_traveled / (float)v->distance_traveled;
}


void destroy_vehicle(vehicle_t* v) {
    if (!v) return;

    if(v->is_alive) {
        kill(v->pid, SIGUSR1);
        waitpid(v->pid, NULL, 0);
    } 
    free(v->client_name);
    free(v->destination);
    free(v);


    v = NULL;
    return;
};

