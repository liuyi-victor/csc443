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
                  long *miliseconds,
                  long *total_bytes_read) 
{

	//char *buffer = (char *)malloc(block_size);
	char buffer[block_size] = { 0 };
	//initialize histogram values to zero
	int i;
	/*
	for (i = 0; i < histogram_length; i++) 
		hist[j] = 0;
	*/
	//initialize the time recording to zero
	*miliseconds = 0;
	*total_bytes_read = 0;

	fseek(file_ptr, 0, SEEK_END);
	long file_size = ftell(file_ptr);
	fseek(file_ptr, 0, SEEK_SET);

	long start_time_in_ms;
	long time_diff;
	struct timeb t;

	size_t chunk_read;

	while(*total_bytes_read < file_size) 
	{
		bzero(buffer, block_size * sizeof(char));

		ftime(&t);
		start_time_in_ms = t.time * 1000 + t.millitm;

		chunk_read = fread(buffer, sizeof(char), block_size, file_ptr);

		ftime(&t);
		time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
		*miliseconds += time_diff;

		int i;
		for (i = 0; i < chunk_read; i++) 
		{
			char c = buffer[i];
			hist[get_position_in_alphabet(c)]++;
		}

		*total_bytes_read += chunk_read;
	}

	return 0;
}

int main(int argc, char* argv[]) 
{
	int print_histogram = 1;
	if (argc == 4 && strcmp(argv[3], "--no-histogram") == 0) {
		print_histogram = 0;
	} 
	else if (argc != 3) {
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
	long miliseconds;
	long filelen;

	get_histogram(fp, hist, block_size, &miliseconds, &filelen);

	if (print_histogram) 
	{
		int i;
		for (i = 0; i < histogram_length; i++) {
		    printf("%c %ld\n", i + 'A', hist[i]);
		}
	}

	printf("BLOCK SIZE: %ld    RATE IN BYTES/S: %ld    TIME IN MS: %ld\n", block_size, filelen / miliseconds / 1000, miliseconds);

	fclose(fp);
	return 0;
}
