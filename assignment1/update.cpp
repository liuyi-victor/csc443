#include "library.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void free_record_memory(Record *tuple)
{
	int arrity = tuple->size();
	for(int i=0; i < arrity; i++)
		free((char *)tuple->at(i));
	tuple->clear();
}
//g++ -g update.cpp library.cpp -o update
//./update file 1_0 0 AAAAAAAAAA 1048576
int main(int argc, char** argv) 
{
	if (argc != 6) 
	{
		fprintf(stderr, "Usage: %s <heapfile> <record_id> <attribute_id> <new_value> <page_size>\n", argv[0]);
		return 1;
	}
	if(access(argv[1], W_OK ) == -1)
	{
		fprintf(stderr, "File: %s does not exist\n", argv[0]);
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
	/*
	memcpy(arr1, argv[2], i);
	std::string rid(argv[2]);
	std::string delimiter = "_";
	pid = atoi(rid.substr(0, rid.find(delimiter)));
	rid.erase(0, rid.find(delimiter) + delimiter.length());
	int slot = atoi(rid);	//parse_record_id(argv[2], &pid);
	*/
	if (slot == -1) 
	{
		fprintf(stderr, "Invalid record id: %s\n", argv[2]);
		return 3;
	}
	int column = atoi(argv[3]);
	int pagesize = atoi(argv[5]);
	FILE *stream = fopen(argv[1], "r+");
	if(stream == NULL)
	{
		return -1;
	}
	Heapfile* heap = (Heapfile*)malloc(sizeof(Heapfile));
	heap->file_ptr = stream;
	heap->page_size = pagesize;
	/*
	if (open_heapfile(heap, argv[1], page_size, record_size) != 0) {
		return 2;
	}
	*/
 
	char *field = (char *)malloc(lengthAttribute);
	memcpy(field, argv[4], lengthAttribute);

	// read in the page to update
	Page* page = (Page*)malloc(sizeof(Page));
	init_fixed_len_page(page, pagesize, numAttribute*lengthAttribute);

	// bound checking? just read the page
	PageEntry entry = {-1, 0};
	int result = readHeapfileDirectory(heap, pid, &entry);
	if(result < 0)
	{return -1;}
	
	read_page(heap, pid, page);
	Record *record = new Record;
	read_fixed_len_page(page, slot, record);
	if(slot >= fixed_len_page_capacity(page) || record->size() <= 0)
		return -1;
	free((char *)record->at(slot));
	record->at(slot) = field;
	write_fixed_len_page(page, slot, record);
	/*
	//unsigned char *tuple = 
	
	if (try_read_page(heap, pid, page) == -1) 
	{
		fprintf(stderr, "Page id out of bounds: %d\n", pid);
		fclose(heap->file_ptr);
		free(heap);
		free(page);
		return 4;
	}

	// make sure the record exists at the given slot
	if (is_freeslot(page, slot)) 
	{
		fprintf(stderr, "Record with id %s does not exist\n", argv[2]);
		fclose(heap->file_ptr);
		free(heap);
		free(page);
		return 5;
	}

	// read in the record's contents, swap in the new attribute
	// value and write it back out to the page
	Record *record = new Record;
	read_fixed_len_page(page, slot, record);
	(*record)[attr_index] = new_value;
	write_fixed_len_page(page, slot, record);
*/
	// write page back to the file
	write_page(page, heap, pid);

	// and free all our stuff
	fclose(heap->file_ptr);
	free_record_memory(record);
	free(heap);

	return 0;
}

