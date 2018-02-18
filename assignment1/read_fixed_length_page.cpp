#include "library.h"
//#include "csvhelper.h"
#include <assert.h>
#include <sys/timeb.h>

void print_record(Record* record)
{
	//Iterate all records and print the 10 bytes as characters.
	for(int i = 0; i < record->size() -1; i++)
	{
		printf("%s, ", record->at(i));
	}
	//Print the last variable with no trailing comma.
	printf("%s\n", record->at(record->size()-1));
}
void free_record_memory(Record *tuple)
{
	int arrity = tuple->size();
	for(int i=0; i < arrity; i++)
		free((char *)tuple->at(i));
	tuple->clear();
}
/**
 * Reads page_file that was generated using write_fixed_len_pages.
 * page_size should match the page_size that was used with write_fixed_len_pages.
 * Once a page is loaded, it prints all records in the page to stdout.
 */
int main(int argc, char** argv)
{
	if(argc != 3)
	{
		printf("Usage: read_fixed_len_page <page_file> <page_size>\n");
		return 1;
	}

	//Parse arguments.
	FILE* page_file = fopen(argv[1], "rb");
	if(!page_file)
	{
		printf("Failed to open page file: %s\n", argv[1]);
		return 2;
	}
	int pagesize = atoi(argv[2]);
	Page* page = (Page *)malloc(sizeof(Page));

	int capacity;
	Record tuple;
	//record the time that program started
	struct timeb t;
	ftime(&t);
	long start_ms = t.time * 1000 + t.millitm;
	long total;

	//Read all pages in file.
	while(!feof(page_file))
	{

		//initialize a page to read into.
		init_fixed_len_page(page, pagesize, numAttribute*lengthAttribute);
		capacity = fixed_len_page_capacity(page);

		//if a page could not be read were done.
		if(fread(page->data, pagesize, 1, page_file) == 0)
		{
			assert(feof(page_file));
			break;
		}
		else
		{
			//read all records of contained in this page
			int i;
			for(i = 0; i < capacity; i++)
			{
				read_fixed_len_page(page, i, &tuple);
				if(tuple.size() != 0)
				{
					printf("record %d is: \n", i);
					print_record(&tuple);
				}
				free_record_memory(&tuple);
				//record.clear();
			}
		}
		init_fixed_len_page(page, pagesize, numAttribute*lengthAttribute);
	}
	free(page->data);
	free(page);

	//Calculate program end time.
	ftime(&t);
	long end_ms = t.time * 1000 + t.millitm;
	printf("TIME: %lu\n", end_ms - start_ms);
}
