#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#define MAXLEN 50

pthread_mutex_t mutex;
int result_index = -1;

struct ThreadArgs {
    int* load;
    size_t size;
};

void* find_and_assign_dep(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int* load = args->load;
    size_t size = args->size;
    int target_value = load[size - 1]; // assuming the last element is the target value
    int arr_size = size - 2;

    int i;
    int min_greater = 0;

    // Find the smallest number greater than or equal to target_value
    for (i = 0; i < arr_size; i++) {
        if (load[i] >= target_value && load[i] <= load[min_greater]){
            min_greater = i;
        }
    }

    pthread_mutex_lock(&mutex);
    
    result_index = min_greater+1;
    printf("Boarding...\n");
    sleep(3);
    printf("Takeoff...\n");
    sleep(2);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}
void* find_and_assign_arr(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    int* load = args->load;
    size_t size = args->size;
    int target_value = load[size - 1]; // assuming the last element is the target value
    int arr_size = size - 2;

    int i;
    int min_greater = 0;

    // Find the smallest number greater than or equal to target_value
    for (i = 0; i < arr_size; i++) {
        if (load[i] >= target_value && load[i] <= load[min_greater]){
            min_greater = i;
        }
    }

    pthread_mutex_lock(&mutex);
    
    result_index = min_greater+1;
    printf("Landing...\n");
    sleep(2);
    printf("Deboarding...\n");
    sleep(3);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

struct Plane {
    long mtype;
    int planeId;
    int planeType; // 0 for cargo, 1 for passenger
    int numPassengers; // Only for passenger planes
    int departureAirport;
    int arrivalAirport;
    int totalWeight;
    int departureStatus; // 0 = still boarding, 1 = ready for departure
    int arrivalStatus; // 0 = still at departure/ not arrived / still deboarding ; 1 = arrived and deboarded
    int terminationStat;
    int* loadCapacity; // Added field for load capacity
    size_t totalRunways; // Added field for total runways
};

int main() {
    struct Plane plane;
    int airportNumber;
    int numRunways;

    printf("Enter Airport Number: ");
    scanf("%d", &airportNumber);
    printf("Enter number of Runways: ");
    scanf("%d", &numRunways);

    int totalRunways = numRunways + 2;
    int* loadCapacity = (int*)malloc(totalRunways * sizeof(int));

    printf("Enter loadCapacity of Runways (give as a space separated list in a single line): ");
    for (int i = 0; i < numRunways; i++) {
        scanf("%d", &loadCapacity[i]);
    }
    loadCapacity[numRunways] = 15000;

    for (int i = 0; i <= numRunways; i++) {
        printf("%d ", loadCapacity[i]);
    }

    key_t key = ftok("airtrafficcontrol.c", 'A');
    if (key == -1) {
        printf("Error: In creating unique key\n");
        exit(1);
    }

    int msgid = msgget(key, 0644 | IPC_CREAT);
    if (msgid == -1) {
        printf("Error in creating message queue\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        plane.mtype = airportNumber;
        if (msgrcv(msgid, (void*)&plane, sizeof(plane) - sizeof(long), plane.mtype, 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
        }
        if(plane.terminationStat == 1){
            break;
        }
        int weightIndex = numRunways + 1;
        loadCapacity[weightIndex] = plane.totalWeight;

        if (plane.arrivalAirport == airportNumber) {
            pthread_t thread;

            struct ThreadArgs args = {loadCapacity, totalRunways};

            pthread_create(&thread, NULL, find_and_assign_arr, (void*)&args);
            

            pthread_join(thread, NULL);
            
            
            printf("Plane %d has has landed on Runway No. %d of Airport No. %d.\n", plane.planeId, result_index, airportNumber);
            
            plane.mtype = 300 + plane.arrivalAirport;
           

            if (msgsnd(msgid, (void*)&plane, sizeof(plane) - sizeof(long), 0) == -1) {
                printf("Error in sending to message queue");
                exit(1);
            }
            

        } else if (plane.departureAirport == airportNumber) {
            
            pthread_t thread;

            struct ThreadArgs args = {loadCapacity, totalRunways};

            pthread_create(&thread, NULL, find_and_assign_dep, (void*)&args);
            


            
            printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n", plane.planeId, result_index, airportNumber);
            
            
            
            plane.mtype = 100 + plane.departureAirport;
            

            if (msgsnd(msgid, (void*)&plane, sizeof(plane) - sizeof(long), 0) == -1) {
                printf("Error in sending to message queue");
                exit(1);
            }
            
        }
    }
    

    return 0;
}
