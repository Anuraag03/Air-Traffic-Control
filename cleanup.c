#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct clean {
    long mtype;
    int terminationStatus;
};

int main() {
    struct clean cl;
    cl.mtype = 51;
    cl.terminationStatus = 0;
    char userInput;
    
    key_t key = ftok("airtrafficcontrol.c",'A');
    if (key == -1) { 
        printf("Error: In creating unique key\n");
        exit(1);
    }
    
    int msgid = msgget(key, 0644|IPC_CREAT);    
    if (msgid == -1) {
        printf("Error in creating message queue\n");
        exit(EXIT_FAILURE);
    }

    printf("Key: %d, Message Queue ID: %d\n", key, msgid); // Debug print
    
    while (1) {
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No): ");
        scanf(" %c", &userInput);  // Notice the space before %c to consume newline
        
        if (userInput == 'Y' || userInput == 'y') {
            while (msgrcv(msgid, (void *)&cl, sizeof(cl)-sizeof(long), cl.mtype, IPC_NOWAIT) != -1) {
                printf("Removed message from queue\n");
            }
            cl.terminationStatus = 1;
            // Send message to message queue
            printf("Sending termination status: %d\n", cl.terminationStatus); // Debug print
            if (msgsnd(msgid, (void *)&cl, sizeof(cl)-sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        } else if (userInput == 'N' || userInput == 'n') {
            cl.terminationStatus = 0;
        } else {
            printf("Incorrect Input: Enter Y for Yes and N for No.\n");
            continue; // Restart the loop
        }

        

        if (userInput == 'Y' || userInput == 'y') {
            break;  // Exit the loop
        }
    }
    
    printf("Cleanup Completed\n");
    return 0;
}
