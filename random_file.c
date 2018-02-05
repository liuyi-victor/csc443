#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]) {
    int verbose = 1;
    if(argc < 4)
        printf("Invalid arguments\n");
        printf("USAGE: ./create_random_file <filename> <total_bytes> <block_size>\n");
        exit(-1);
    }

    char* file_name = argv[1];

    long total_bytes = atol(argv[2]);

    long block_size = atol(argv[3])

    if (*other || block_size > total_bytes) {
        printf("Invalid block size.\n");
        exit(1);
    }

    char *buffer = (char*)malloc(block_size * sizeof(char));

    FILE *fp = fopen(file_name, "w");

    long execution_time_in_ms = 0;
    long start_time_in_ms;
    long time_diff;
    struct timeb t;

    int pages = total_bytes / block_size;
    long written_bytes = total_bytes;
    int i;
    for (i = 0; i < pages; i++) {
        random_array(buffer, block_size);

        ftime(&t);
        start_time_in_ms = t.time * 1000 + t.millitm;

        fwrite(buffer, sizeof(char), block_size, fp);
        fflush(fp);

        ftime(&t);
        time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
        execution_time_in_ms += time_diff;

        written_bytes -= block_size;
    }

    if (written_bytes > 0) {
        random_array(buffer, written_bytes);

        ftime(&t);
        start_time_in_ms = t.time * 1000 + t.millitm;

        fwrite(buffer, sizeof(char), written_bytes, fp);
        fflush(fp);

        ftime(&t);
        time_diff = t.time * 1000 + t.millitm - start_time_in_ms;
        execution_time_in_ms += time_diff;
    }

    fclose(fp);
    if (remove_file_after_completion) {
        remove(file_name);
    }

    if (print_result) {
        printf("BLOCK_SIZE: %ld    RATE IN BYTES/S: %ld    TIME IN MS: %ld\n", block_size, total_bytes / execution_time_in_ms / 1000, execution_time_in_ms);
    }
    return 0;
}
