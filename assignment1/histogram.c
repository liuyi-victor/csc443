#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/timeb.h>

#define histogram_length 26

int get_position_in_alphabet(char c) 
{
    return c - 'A';
}
/*
 * file_ptr: the file pointer, ready to be read from.
 * hist: an array to hold 26 long integers. hist[0] is the
 *	 number of 'A', and hist[1] is the number of 'B', etc. 
 * block_size: the buffer size to be used
 * miliseconds: time it took to complete the file scan
 * total_bytes_read: the amount data in bytes read
 *
 * returns: -1 if there is an error
 */
int get_histogram(FILE *file_ptr,
                  long hist[],
                  long block_size,
                  long *milliseconds,
                  long *total_bytes_read) 
{

	char *buffer = (char *)malloc(block_size);
	//char buffer[block_size] = { 0 };
	//initialize histogram values to zero
	int i;
	
	for (i = 0; i < histogram_length; i++) 
		hist[i] = 0;
	
	//initialize the time recording to zero
	*milliseconds = 0;
	*total_bytes_read = 0;

	//fseek(file_ptr, 0, SEEK_END);
	//long file_size = ftell(file_ptr);
	//fseek(file_ptr, 0, SEEK_SET);

	long start_time_in_ms;
	long time_diff;
	struct timeb t;

	size_t size;

	while(!feof(file_ptr)) 
	{
		bzero(buffer, block_size * sizeof(char));

		ftime(&t);
		start_time_in_ms = t.time * 1000 + t.millitm;

		size = fread(buffer, block_size, 1, file_ptr);
		if(size == 0)
			break;
		ftime(&t);
		time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
		*milliseconds += time_diff;

		int i;
		for (i = 0; i < size; i++) 
		{
			char c = buffer[i];
			hist[get_position_in_alphabet(c)]++;
		}

		*total_bytes_read += size;
	}
	free(buffer);
	return 0;
}

int main(int argc, char* argv[]) 
{

	if (argc != 3) 
	{
		printf("Invalid amount of arguments, please input the format:\n");
		printf("./get_histogram <filename> <block_size>\n");
		exit(1);
	}

	char* file_name = argv[1];

	char *other = NULL;

	long block_size = strtol(argv[2], &other, 10);
	if (*other) {
		printf("Invalid block size.\n");
		exit(1);
	}

	FILE *fp = fopen(file_name, "r");
	long hist[histogram_length];
	long milliseconds = 0;
	long filelen = 0;

	get_histogram(fp, hist, block_size, &milliseconds, &filelen);

	int i;
	for (i = 0; i < histogram_length; i++) {
		printf("%c: %ld\n", i + 'A', hist[i]);
	}

	printf("BLOCK SIZE: %ld\n    TOTAL BYTES: %ld\n    TIME IN MS: %ld\n", block_size, filelen, milliseconds);

	fclose(fp);
	return 0;
}
