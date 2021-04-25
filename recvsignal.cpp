#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>


// shared memory size
#define SHARED_MEMORY_CHUNK_SIZE 1000
// shared memory struct
#define READY 1
#define RECEIVED 2
#define DONE -1
struct memory{
  char buff[SHARED_MEMORY_CHUNK_SIZE];
  int pids,pidr,msgsize,status;
};

int shmid;
int set=0;
// pointer to shared memory
struct memory *sharedMemPtr;

const char recvFileName[] = "recvfile";

void handler(int signum) {
 if(signum == SIGUSR2) {
   // if it is ready, set is assigned 1
    if(sharedMemPtr->status == READY) {
        set = 1;
    }
    // if it is done, set is assigned 2
    if(sharedMemPtr->status == DONE) {
        set = 2;
    }
 }


}

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, struct memory*& sharedMemPtr)
{

    	/* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
       		 so manually or from the code).
      	2. Use ftok("keyfile.txt", 'a') in order to generate the key.
        3. Use the key in the TODO's below. Use the same key for the
           shared memory segment. This also serves to illustrate the difference
      		 between the key and the id used in message queues and shared memory. The id
      		 for any System V object (i.e. message queues, shared memory, and sempahores)
      		 is unique system-wide among all System V objects. Two objects, on the other hand,
      		 may have the same key.
    	 */

      //generating key for keyfilesignal.txt for further use and error check
    	 key_t key = ftok("keyfilesignal.txt",'a');
    	 if(key == -1) {
    	    perror("ERROR:: generating key");
    			exit(1);
    	 }

    	/* TODO: Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
    	/* TODO: Attach to the shared memory */
    	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */

       // allocate piece of shared memory and error check
    	 shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT|0666);
    	 if(shmid == -1) {
    		 perror("ERROR:: shared segment already exist for this key");
    		 shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE,0666);
    		 exit(1);
    	 }

    	 // attach to shared memory and error check
    	sharedMemPtr = (struct memory*)shmat(shmid, NULL, 0);
      if(sharedMemPtr == (memory*)-1) {
        perror("ERROR:: not attached with segment");
        exit(1);
      }

      // getting id for receiver
      sharedMemPtr->pidr = getpid();

}


/**
 * The main loop
 */
  void mainLoop()
 {
       	std::cout<< "sending signal..."<<std::endl;

        // setting to 1
        sharedMemPtr->msgsize =1;

     	  /* Open the file for writing */
      	FILE* fp = fopen(recvFileName, "w");

    	  /* Error checks */
      	if(!fp)
    	{
    		perror("fopen");
    		exit(-1);
    	}


       /* TODO: Receive the signal and get the message size. The message will
        * contain regular information.  If the size field
        * of the message is not 0, then we copy that many bytes from the shared
        * memory region to the file. Otherwise, if 0, then we close the file and
        * exit.
        *
        * NOTE: the received file will always be saved into the file called
        * "recvfile"
        */

     	 /* Keep receiving until the sender set the size to 0, indicating that
      	* there is no more data to send
      	*/

      // loop while message has not been received
    	while(set != 2 && sharedMemPtr->msgsize !=0) {
        // waiting for sender's signal
        while(set==0) {
          continue;
        }
        sleep(1);

        // set is assigned 1 after getting signal
    		if(set == 1) {
      			// error check
      			if(fwrite(sharedMemPtr->buff, sizeof(char), sharedMemPtr->msgsize, fp) < 0) {
      				perror("fwrite");
      			}

      			/* TODO: Tell the sender that we are ready for the next file chunk.*/
            sharedMemPtr->status = RECEIVED;
             // signaling that message has been received
              kill(sharedMemPtr->pids,SIGUSR1);
              // assigning 0 to set
              set = 0;
    		}
    	}
      std::cout << "Message received!\n";
    	/* We are done */
      if(set == 2) {
  		  	/* Close the file */
  			  fclose(fp);
  		}

 }

/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 */

void cleanUp(const int& shmid, struct memory* sharedMemPtr)
{
	/* TODO: Detach from shared memory*/
  // and error check if failed to detach
	if (shmdt(sharedMemPtr) == -1) {
 	 perror("shmdt");
 	 exit(-1);
  }
	/* TODO: Deallocate the shared memory chunk */
  // and error check
	if (shmctl(shmid, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		exit(-1);
	}

}

  /**
  * Handles the exit signal
  * @param signal - the signal type
  */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid,sharedMemPtr);
}

int main(int argc, char** argv)
{

	/* TODO: Install a singnal handler (see signaldemo.cpp sample file).
 	 * In a case user presses Ctrl-c your program should delete message
 	 * queues and shared memory before exiting. You may add the cleaning functionality
 	 * in ctrlCSignal().
 	 */
  signal(SIGINT, ctrlCSignal);

  // install signal handler
  signal(SIGUSR2, handler);
	/* Initialize */
	init(shmid,sharedMemPtr);

	/* Go to the main loop */
	mainLoop();

	/** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
	ctrlCSignal(0);

	return 0;
}
