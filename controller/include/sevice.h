// services yet to be scheduled should be in pending controller_entry_fifo
// services being serviced should be in active services list

// One service can be done per customer at the time
// That means a client can have a pipe with its name for easy communication
// e.g. /tmp/client_<client_name>_service_fifo
// The ve


// Each user has 0 to many service requests
// One is served at the time 
// User session can hold info about each user, 
// Its requested services, their status (pending, active, completed), etc.
// For each active service, the number of kilometers remaining to the destination should be tracked
// Requested services 
// vehicle can have a pipe with client name + destination 

// Sessi

// Fifo of pending requests
struct service_request {
    char customer_name[32];
    char location[32];
    int distance; // in meters
    int time; // in seconds/ticks from the start of the controller 
};

// Each service request corresponds to one service session
// One client can have one active service at the time

struct service {
    int id;
    struct service_request request;
    enum {ACTIVE, COMPLETED, CANCELLED } status;
    int kilometers_remaining; // for ACTIVE services
    int vehicle_pid; // PID of the vehicle servicing this request
};

// For each user, a user_service sesision must be created