#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


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
int r =0;
// pointer to the shared memory
struct memory *sharedMemPtr;

void handlersend(int signum) {
 	if(signum == SIGUSR1) {
      if(sharedMemPtr->status == RECEIVED) {
        r = 1;
      }
 	}
}

 /**
  * Sets up the shared memory segment
  * @param shmid - the id of the allocated shared memory
  */
void init(int& shmid, struct memory*& sharedMemPtr)
{
    	/* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
     		   so manually or from the code).
    	  2. Use ftok("keyfile.txt", 'a') in order to generate the key.
    		3. Use the key in the TODO's below. Use the same key for the
           shared memory segment. This also serves to illustrate the difference
    		   between the key and the id used in shared memory. The id
    		   or any System V objest (i.e. message queues, shared memory, and sempahores)
    		   is unique system-wide among all SYstem V objects. Two objects, on the other hand,
    		   may have the same key.
    	 */

          //generating key for keyfilesignal.txt for further use and error check
          key_t key = ftok("keyfilesignal.txt",'a');
    			if(key == -1) {
    	 	    perror("ERROR:: generating key");
    	 			exit(1);
    	 	  }


      	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
      	/* TODO: Attach to the shared memory */
      	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */

         // get id of shared memory segment and error check
      	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE,0666);
      	if(shmid == -1) {
      		perror("ERROR:: shared segment ");

      		exit(1);
      	}

      	//attach to shared memory and error check
        sharedMemPtr = (struct memory*)shmat(shmid,NULL,0);
      	if(sharedMemPtr == (memory*)-1) {
      		perror("ERROR:: not attached with segment");
      		exit(1);
      	}

        //getting sender id
         sharedMemPtr->pids = getpid();
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 */

void cleanUp(const int& shmid, struct memory* sharedMemPtr)
{
  	/* TODO: Detach from shared memory */
  	// program detaches from shared memory segment and error check
  	if (shmdt(sharedMemPtr) == -1)
    {
  		perror("shmdt");
  		exit(-1);
  	}
}


 /**
  * The main send function
  * @param fileName - the name of the file
  */
void send(const char* fileName)
{

	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");

	/* Was the file open? */
	if(!fp)
	{
		perror("fopen");
		exit(-1);
	}

	/* Read the whole file */
	while(!feof(fp))
	{

    		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.
     		 * fread will return how many bytes it has actually read (since the last chunk may be less
     		 * than SHARED_MEMORY_CHUNK_SIZE).
     		 */
    		if((sharedMemPtr->msgsize = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
    		{
    			perror("fread");
    			exit(-1);
    		}

        // changes status to ready
        sharedMemPtr->status = READY;
        // sending signal to RECEIVER -SIGUSR2
        kill(sharedMemPtr->pidr,SIGUSR2);
  }

  while(r!=1) {
    continue;
  }

  if(r==1) {
   //change the status to DONE
   sharedMemPtr->status = DONE;
   //set messagesize 0
   sharedMemPtr->msgsize =0;
   //send the signal to receiver
   kill(sharedMemPtr->pidr,SIGUSR2);
  }

  /* Close the file */
  fclose(fp);
}

int main(int argc, char** argv)
{

  signal(SIGUSR1,handlersend);
	/* Check the command line arguments */
	 if(argc < 2)
	 {
    fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
	 	exit(-1);
	 }

	/* Connect to shared memory and the message queue */
	init(shmid,sharedMemPtr);

	/* Send the file */
	send(argv[1]);

	/* Cleanup */
  cleanUp(shmid,sharedMemPtr);

	return 0;
}
