#include "logic.h"
#include "utils.h"

static char* client_name;
static char* destination;
static int distance_to_travel;
static int tenth_of_distance_to_travel;
static int vehicle_id;


static int distance_traveled = 0;
static bool was_completed = false;

static int timer_tick_speed_ms;

static volatile sig_atomic_t service_active = 0;

void termination_routine() { 
    send_termination_message_to_controller();
    //send_termination_message_to_client(client_name, CLIENT_PIPE_PATH_BASE, destination, was_completed);
    exit(0);
}

void service_start_signal_handler(int signum) {
    service_active = 1;
}
void service_termination_signal_handler(int signum) {
    service_active = 0;
}
bool read_cmdline_args(int argc, char* argv[]) {
    if (argc != 5) {
        perror(ERROR "Invalid number of command line arguments have been passed to the vehicle");
        return false;
    }

    client_name = argv[1];
    destination = argv[2];
    distance_to_travel = atoi(argv[3]);
    vehicle_id = atoi(argv[4]);

    tenth_of_distance_to_travel = distance_to_travel / 10;

    if (tenth_of_distance_to_travel == 0) {
        perror(ERROR "Invalid tenth of distance has been calculated");
    }
 
    if(distance_to_travel <= 0) {
        perror(ERROR "Invalid distance to travel has been passed to the vehicle");
        return false;

    }
    if(vehicle_id < 0) {
        perror(ERROR "Invalid id has been passed to the vehicle");
        return false;

    }
    return true;
}

void init(int argc, char* argv[]) {
    timer_tick_speed_ms = read_tick_spead_from_enviroment();
    
    struct sigaction start_action = {0};
    start_action.sa_handler = service_start_signal_handler;
    sigaction(SIGUSR2, &start_action, NULL);

    struct sigaction termination_action = {0};
    termination_action.sa_handler = service_termination_signal_handler;
    sigaction(SIGUSR1, &termination_action, NULL);

    if(!read_cmdline_args(argc, argv)) {
        perror(ERROR "Could not read command line arguments");
        exit(EXIT_FAILURE);
    }

    // Unbuffer stdout so that the redirected output is immediate

    // Check to not get stuck in the loop 
    //if(is_named_pipe_exists(CLIENT_PIPE_PATH_BASE, client_name) == false) {
    //    termination_routine();
    //}


    send_arrived_message_to_controler();
    //send_arrived_message_to_client(client_name, CLIENT_PIPE_PATH_BASE, destination);
    // UNCOMMENT FOR FINAL VERSION


}



void main_loop() {
    // Wait for SIGUSR1 or SIGUSR2
    pause();

    if(service_active) {
        send_service_start_message_to_controller();
        //send_service_start_message_to_client(client_name, CLIENT_PIPE_PATH_BASE, destination);
    } else {
        termination_routine();
    }

    // Service in action and hasnt been cancelled 
    while(service_active) {
        distance_traveled++;
        if (distance_traveled % tenth_of_distance_to_travel == 0) {
                // Notify controller send_termination_message_to_controller()
            send_tenth_of_distance_traveled_message_to_controller(tenth_of_distance_to_travel);
        }

        if(distance_traveled >= distance_to_travel) {
            was_completed = true;
            break;
        }
        usleep(timer_tick_speed_ms * 1000);
    }
    termination_routine();
    return;
}
