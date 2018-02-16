#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

/**
 * populate a random array (which is already
 * allocated with enough memory to hold n bytes
 */
void random_array(char *array, long bytes)
{
	long i;
	for(i = 0; i < bytes; i++)
		array[i] = 'A' + (rand() % 26);
}

/**
 * allocate a fixed amount of memory 
 * char buffer[block_size], and repeatedly generate random content into buffer,
 * and then write buffer to disk. 
 * A reasonable block_size is 1MB.
 */
int main(int argc, char* argv[]) 
{
	//int verbose = 1;
	if(argc < 4)
	{
		printf("Invalid arguments\n");
		printf("USAGE: ./create_random_file <filename> <total_bytes> <block_size>\n");
		exit(-1);
	}

	char* file_name = argv[1];

	long total_bytes = atol(argv[2]);

	long block_size = atol(argv[3]);

	if (block_size > total_bytes || block_size < 0 || total_bytes < 0) 
	{
		printf("Invalid arguments.\n");
		exit(-1);
	}

	char *buffer = (char*)malloc(block_size);

	FILE *stream = fopen(file_name, "w");

	long execution_time_in_ms = 0;
	long start_time_in_ms;
	long time_diff;
	struct timeb t;

	int num_pages = total_bytes / block_size;
	long remaining = total_bytes;
	int i;
	for(i = 0; i < num_pages; i++) 
	{
		random_array(buffer, block_size);

		//do not include the time to generate the random array
		ftime(&t);
		start_time_in_ms = t.time * 1000 + t.millitm;

		fwrite(buffer, 1, block_size, stream);
		fflush(stream);

		ftime(&t);
		time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
		execution_time_in_ms += time_diff;

		remaining -= block_size;
	}

	if (remaining > 0) 
	{
		random_array(buffer, remaining);

		ftime(&t);
		start_time_in_ms = t.time * 1000 + t.millitm;

		fwrite(buffer, 1, remaining, stream);
		fflush(stream);

		ftime(&t);
		time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
		execution_time_in_ms += time_diff;
	}

	fclose(stream);
	//if (remove_file_after_completion) 
	//{
	//	remove(file_name);
	//}

	//if(verbose) 
	//{
		printf("BLOCK_SIZE: %ld    RATE IN BYTES/S: %ld    TIME IN MS: %ld\n", block_size, total_bytes / execution_time_in_ms / 1000, execution_time_in_ms);
	//}
	free(buffer);
	return 0;
}
