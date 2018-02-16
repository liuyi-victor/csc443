#include "library.h"
#include "csvhelper.h"
#include <sys/timeb.h>

void print_record(Record* record)
{
	//Iterate all records and print the 10 bytes as characters.
	for(int i = 0; i < record->size()-1; i++)
	{
		printf("%.10s,", record->at(i));
	}
	//Print the last variable with no trailing comma.
	printf("%.10s\n", record->at(record->size()-1));
}

/**
 * Reads page_file that was generated using write_fixed_len_pages.
 * page_size should match the page_size that was used with write_fixed_len_pages.
 * Once a page is loaded, it prints all records in the page to stdout.
 */
int main(int argc, char** argv){
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
	Record tuple = new Record();
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
		if(fread(page->data, pagesize, 1, page_file) == 0){
			assert(feof(page_file));
			break;
		}
		else
		{
			for(int i = 0; i < capacity; i++)
			{
				read_fixed_len_page(page, i, &record);
				if(record.size() != 0)
					print_record(&record);
				record.clear();
			}
			/*
			unsigned char* directory_offset = ((unsigned char*)page->data) + fixed_len_page_directory_offset(page);

			int record_count = 0;

			//Traverse all records in read page and print them.
			for(int i = 0; i < fixed_len_page_capacity(page); i++)
			{
				
				if(i > 0 && i%8 == 0){
					directory_offset += 1;
				}

				//Retrieve and print record from slot if its marked in the directory.
				unsigned char directory = (unsigned char)*directory_offset;
				if(directory >> (i%8) & 0x01)
				{
					Record record;
					read_fixed_len_page(page, i, &record);
					printf("Record %d at slot %d: ", record_count, i);
					print_record(&record);
					record_count++;
				}
			}
			*/
		}
		free_fixed_len_page(page);
	}
	free(page);

	//Calculate program end time.
	ftime(&t);
	long end_ms = t.time * 1000 + t.millitm;
	printf("TIME: %lu\n", end_ms - start_ms);
}
