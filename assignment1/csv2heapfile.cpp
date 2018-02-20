#include "library.h"
#include <assert.h>
#include <sys/timeb.h>
#include <stdio.h>

int read_records(char *csvfile, std::vector<Record> *table)
{
	FILE* file = fopen(csvfile, "r");
	if (file == NULL)
	{
		return -1;
	}
	//string delimiter = ",";
	rewind(file);
	int count = 0;

	Record tuple;// = new Record();
	char field[lengthAttribute];
	while(!feof(file))
	{
		//fread(field, 1100, 1, file)
		
		int i;
		for(i = 0; i < numAttribute; i++)
		{
			//read a tuple in the table
			if(fread(field, lengthAttribute, 1, file) == 0)
				break;

			//skip comma and newline
			fgetc(file);
			tuple.push_back(field);
		}

		table->push_back(tuple);
		tuple.clear();
		count++;
		//if (count == 1000)
			//put("adsd");
	}
	return count;
}

/**
 * Takes a csv file and converts it to a heap file with given page sizes.
 */
 
 
//g++ -g csv2heapfile.cpp library.cpp -o heap
int main(int argc, char** argv)
{
	//Make sure all args are provided.
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s <csv_file> <heapfile> <page_size>\n", argv[0]);
		return 1;
	}

	//load records from csv.
	
	
	
	//Get records
	std::vector<Record*> table;	//this is basically a table in memory
	//int count = read_records(argv[1], &table);
	
	
	
	
	
	FILE* file = fopen(argv[1], "r");
	if (file == NULL)
	{
		return -1;
	}
	//string delimiter = ",";
	rewind(file);
	int count = 0;

	//Record tuple;// = new Record();
	//char field[lengthAttribute];
	while(!feof(file))
	{
		//fread(field, 1100, 1, file)
		Record *tuple = new Record();
		int i;
		for(i = 0; i < numAttribute; i++)
		{
			//read a tuple in the table
			char *field = (char *)malloc(lengthAttribute);
			if(fread(field, lengthAttribute, 1, file) == 0)
				break;

			//skip comma and newline
			fgetc(file);
			tuple->push_back(field);
		}
		if(feof(file))
			break;
		table.push_back(tuple);
		//tuple.clear();
		count++;
	}
	
	
	
	
	assert(table.size() == count);

	
	
	
	/*
	if (error) 
	{
		fprintf(stderr, "Could not read records from file: %s\n", argv[1]);
		return 2;
	}
*/
	if(table.size() == 0)
	{
		fprintf(stderr, "No records in file: %s\n", argv[1]);
		return 3;
	}
	int slotsize = fixed_len_sizeof(table.at(0));
	int pagesize = atoi(argv[3]);
	//Record start time of program.
	//We do not include parsing of the csv because that is irrelevant to our metrics.
	struct timeb t;
	ftime(&t);
	long start_ms = t.time * 1000 + t.millitm;

	Heapfile* heap = (Heapfile*)malloc(sizeof(Heapfile));
	//open heap file where heap is stored.
	FILE* heap_file = fopen(argv[2], "w+b");
	if(!heap_file)
	{
		printf("Failed to open heap file to write to: %s\n", argv[2]);
		fclose(heap_file);
		free(heap);
		return 4;
	}
	init_heapfile(heap, pagesize, heap_file);

	Page* page = (Page*)malloc(sizeof(Page));
	page->data = NULL;
	//read_page(heap, page_id, page);
	init_fixed_len_page(page, pagesize, slotsize);

	//Page* dir_page = (Page*)malloc(sizeof(Page));
	//read_directory_page(heap, heap_id_of_page(page_id, heap->page_size), dir_page);
	int i, result, cardinality = table.size();
	PageID pid = alloc_page(heap);		//first initialize a data page, PageID 0 is for the directory of the heapfile
	for(i = 0; i < cardinality; i++)
	{
		result = add_fixed_len_page(page, table.at(i));
		if(result == -1)
		{
			//printf("reached here\n");
			//page is full, need to write this page to heapfile and re-initialize page
			write_page(page, heap, pid);
			pid = alloc_page(heap);
			init_fixed_len_page(page, pagesize, slotsize);
			add_fixed_len_page(page, table.at(i));	//try again to insert into the new page
		}
	}
	//write the final page to heap.
	write_page(page, heap, pid);
	/*
	//Loop all records and add them to heap.
	for(int i = 0; i < records.size(); i++)
	{
		printf("Record %d: ", i);
		print_record(records.at(i));

		//If page is full, create new page in heap.
		if(add_fixed_len_page(page, records.at(i)) == -1)
		{

			//Write page back to heap.
			write_page(page, heap, page_id);

			// don't read over the zeroth page again
			// because until we write to disk it will still seem like it has
			// free space
			if (page_id == 0) 
			{
				page_id = 1;
			}

			//Alloc new page and add record to it.
			page_id = alloc_page(heap, dir_page, page_id);
			//            page_id = alloc_page(heap);
			read_page(heap, page_id, page);
			add_fixed_len_page(page, records.at(i));
		}
	}

	//Write our final page to heap.
	write_page(page, heap, page_id);
*/
	//Calculate program end time.
	ftime(&t);
	long end_ms = t.time * 1000 + t.millitm;
	printf("TIME: %lu\n", end_ms - start_ms);

	return 0;
}

