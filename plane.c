
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define maxPassengers 10
#define maxLuggageWeight 25
#define maxPassengerWeight 100
#define minPassengerWeight 10
#define avgCrewWeight 75
#define numPassPlaneCrew 7
#define numCarPlaneCrew 2

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
int main(){
    struct Plane plane;
    int numCargoItems;
    int avgCargoWeight;
    int departureAirport;
    int arrivalAirport;
    plane.totalWeight = 0;
    int totalPassengerWeight = 0;
    int totalCargoWeight = 0;
    plane.departureStatus = 0;
    plane.arrivalStatus = 0;
    key_t key;
    int msgid;
    key = ftok("airtrafficcontrol.c",'A');
    if(key==-1){
        printf("Error: In creating unique key\n");
        exit(1);
    }
    printf("Enter Plane ID: "); // Plane ID taken as input
    scanf("%d", &plane.planeId);
    plane.mtype= plane.planeId;
    printf("Enter Type of Plane: "); // Plane type taken as input
    scanf("%d", &plane.planeType);
    if (plane.planeType == 1) {
        while(1){
            printf("Enter Number of Occupied Seats: ");
            scanf("%d", &plane.numPassengers);
            if (plane.numPassengers >= 1 && plane.numPassengers <= maxPassengers) {
                printf("Invalid number of passengers. Must be between 1 and 10.\n");
                break;
            }
        }

        int passenger_pipe[plane.numPassengers][2];
        for (int i = 0; i < plane.numPassengers; i++) {
            if (pipe(passenger_pipe[i]) == -1) {
                perror("Pipe creation failed");
                return 1;
            }

            pid_t pid = fork();

            if (pid == -1) {
                perror("Fork failed");
                return 1;
            }

            if (pid == 0) { // Child process (passenger)
                close(passenger_pipe[i][0]); // Close unused read end

                int luggageWeight, bodyWeight;
                while(1){
                    printf("Enter Weight of Your Luggage (Passenger %d): ", i + 1);
                    scanf("%d", &luggageWeight);
                    if (luggageWeight < 0 || luggageWeight > maxLuggageWeight) {
                        printf("Invalid luggage weight. Must be between 0 and 25.\n");
                    }
                    else{
                        break;
                    }
                }
                while(1){
                    printf("Enter Your Body Weight (Passenger %d): ", i + 1);
                    scanf("%d", &bodyWeight);
                    if (bodyWeight < minPassengerWeight || bodyWeight > maxPassengerWeight) {
                        printf("Invalid body weight. Must be between 10 and 100.\n");
                        
                    }
                    else{
                        break;
                    }
                }

                // Send luggage and body weight to parent process
                write(passenger_pipe[i][1], &luggageWeight, sizeof(int));
                write(passenger_pipe[i][1], &bodyWeight, sizeof(int));

                close(passenger_pipe[i][1]); // Close write end
                exit(0);
            } else { // Parent process (plane)
                close(passenger_pipe[i][1]); // Close unused write end
            }
            wait(NULL);
        }

        // Read weights from passengers
        for (int i = 0; i < plane.numPassengers; i++) {
            int luggageWeight, bodyWeight;
            read(passenger_pipe[i][0], &luggageWeight, sizeof(int));
            read(passenger_pipe[i][0], &bodyWeight, sizeof(int));
            close(passenger_pipe[i][0]); // Close read end

            totalPassengerWeight += luggageWeight + bodyWeight;
        }

        plane.totalWeight = totalPassengerWeight + avgCrewWeight * numPassPlaneCrew; 
    }
    else if (plane.planeType == 0) {
        while(1){

            printf("Enter Number of Cargo Items: ");
            scanf("%d", &numCargoItems);
            if(numCargoItems >=1 && numCargoItems <=100){
                break;
            }
        }
        while(1){

            printf("Enter Average Weight of Cargo Items: ");
            scanf("%d", &avgCargoWeight);
            if(avgCargoWeight >=1 && avgCargoWeight <=100){
                break;
            }
        }
        
        
        totalCargoWeight = avgCargoWeight * numCargoItems;
        plane.totalWeight = totalCargoWeight + avgCrewWeight * numCarPlaneCrew;
    }
    printf("Total Weight of the Plane: %d\n", plane.totalWeight);
    printf("Enter Airport Number for Departure: ");
    scanf("%d", &plane.departureAirport);
    plane.departureStatus = 1;
    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &plane.arrivalAirport);
    msgid = msgget(key, 0644|IPC_CREAT);    
    if (msgid == -1){
        printf("error in creating message queue\n");
        exit(1);
    }
    plane.mtype = 50;
    if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
    }
    
    plane.mtype = 206+plane.planeId;
    if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long),plane.mtype , 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
    }
    printf("Flying....\n");
    sleep(30);
    printf("Reached Airport %d....\n",plane.arrivalAirport);
    plane.mtype = 207+plane.planeId;
    if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
    }
    
    plane.mtype = 208+plane.planeId;
    if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long),plane.mtype , 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
    }
    
    printf("Plane %d has successfully traveled from Airport %d to Airport %d !\n",plane.planeId,plane.departureAirport,plane.arrivalAirport);

    return 0;
}