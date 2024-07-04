#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct clean {
    long mtype;
    int terminationStatus;
};

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
    struct clean cl;
    struct Plane plane;
    plane.terminationStat = 0;
    cl.terminationStatus = 0;
    cl.mtype = 51;
    plane.mtype = 2;
    int numAirports;
    int msgid;
    
    while(1){
        printf("Enter the number of airports to be handled/managed: ");
        scanf("%d", &numAirports);
        if(numAirports>=2 && numAirports<=10){
            break;
        }
    }
    
    key_t key = ftok("airtrafficcontrol.c", 'A');
    if (key == -1) { 
        printf("Error: In creating unique key\n");
        exit(1);
    }
    
    msgid = msgget(key, 0644);
    if (msgid != -1) {
        if (msgctl(msgid, IPC_RMID, NULL) == -1) {
            perror("Error in deleting preexisting message queue");
            exit(EXIT_FAILURE);
        }
    
    }
    
    // Create a new message queue
    msgid = msgget(key, 0644 | IPC_CREAT);
    if (msgid == -1) {
        printf("Error in creating message queue\n");
        exit(EXIT_FAILURE);
    }

    
    while (1) {
        
        
        cl.mtype = 51;
        if (msgrcv(msgid, (void *)&cl, sizeof(cl)-sizeof(long), cl.mtype, IPC_NOWAIT) != -1) {
                if (cl.terminationStatus == 1) {
                    printf("Termination signal received\n");
                    
                    plane.terminationStat = 1;

                    break;
                }
        }
        
        plane.mtype = 50;
       
        if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long), plane.mtype, 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
        } 
        
        
        
        
        plane.mtype = plane.departureAirport;
        if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
        }

    	  
        
        if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long), 100+plane.mtype, 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
        }
        FILE *filePointer;

        // Open the file in append mode
        filePointer = fopen("AirTrafficContr**oller.txt", "a");

        // Check if file opened successfully
        if (filePointer == NULL) {
            printf("Unable to open file.\n");
            return 1;
        }

        // Write a newline to the file
        fprintf(filePointer, "Plane %d has departed from Airport %d and will land at Airport %d.\n",plane.planeId,plane.departureAirport,plane.arrivalAirport);
    
        // Close the file
        fclose(filePointer);
        
        plane.mtype = 206+plane.planeId;
        if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
        }
        plane.mtype = 207+plane.planeId;
        
        if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long),plane.mtype , 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
        }
        


        plane.mtype = plane.arrivalAirport;
        
        if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
        }
        
        if (msgrcv(msgid, (void *)&plane, sizeof(plane) - sizeof(long), 300+plane.mtype, 0) == -1) {
            perror("Error: msgrcv for struct Plane");
            exit(EXIT_FAILURE);
        }

        

    
        
        plane.mtype = 208+plane.planeId;
        if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
    	   printf("error in sending to message queue");
           exit(1);
        }
        
        
    }
    for(int i=1;i<=numAirports;i++){
        plane.mtype = i;
        
        if(msgsnd(msgid,(void *)&plane,sizeof(plane)-sizeof(long),0) == -1){
           printf("error in sending to message queue");
           exit(1);
        }
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Error in deleting message queue");
        exit(EXIT_FAILURE);
    }



    return 0;
}
