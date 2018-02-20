#include "library.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		return -1;
	}
	int pagesize = atoi(argv[3]);
	if(access(argv[1], W_OK ) == -1)
	{
		return -1;
	}
	FILE *stream = fopen(argv[1], "w+");
	if(stream == NULL)
	{
		return -1;
	}
	
	// read in the record id
	int pid, i = 0;
	while(argv[2][i] != '_')
		i++;
	char arr1[i] = { 0 };
	memcpy(arr1, argv[2], i);
	char *arr2[i] = { 0 };
	pid = atoi(arr1);
	int slot = atoi(argv[2] + i+1);
	
	Heapfile *file = (Heapfile *)malloc(sizeof(Heapfile));
	file->file_ptr = stream;
	file->page_size = pagesize;
	
	Page *page = (Page *)malloc(sizeof(Page));
	init_fixed_len_page(page, pagesize, numAttribute*lengthAttribute);
	int result = read_page(file, pid, page);
	if(result < 0)
	{
		fprintf(stderr, "Error: page %d does not exist\n", pid);
		return -1;
	}
	write_fixed_len_page(page, slot, NULL);
	
	/*
	
	PageEntry entry = {-1, 0};
	result = readHeapfileDirectory(file, pid, &entry);
	if(result < 0)
	{
		fprintf(stderr, "Error: page %d does not exist\n", pid);
		return -1;
	}
	char *buffer = (char *)malloc(pagesize);
	
	free(buffer);*/
	return 0;
}
