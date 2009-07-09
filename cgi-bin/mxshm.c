#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>


int main()
  {
  ///---segment initializing
	int segment_id;
	char *shared_memory;
	struct shmid_ds shmbuffer;
	int segment_size;
	const int shared_segment_size=0x6400;
	
//  key_t key1;
//  key1=ftok("/tmp/app", 'a');

	segment_id=shmget(ftok("/tmp/app", 'a'), shared_segment_size,
										IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if(segment_id==-1) {
		printf("error %d. segment exists?\n", errno);
		exit(1);
	  }
	
	///---segment using for write
	shared_memory=(char *) shmat(segment_id, 0, 0);
	printf("shared memory attached at address %p\n", shared_memory);
	
	shmctl(segment_id, IPC_STAT, &shmbuffer);
	segment_size=shmbuffer.shm_segsz;
	printf("segment size: %d\n", segment_size);
	sprintf(shared_memory, "Hello, World!");
	
	shmdt(shared_memory);
	
	///---segment using for reading
	shared_memory=(char *) shmat(segment_id, (void *) 0x5000000, 0);
	printf("Shared memory reattached at address %p\n", shared_memory);  
	
	printf("%s\n", shared_memory);
	
	shmdt(shared_memory);
	
	///---segment utilization
	shmctl(segment_id, IPC_RMID, 0);

  return 0;
  }
