#include "library.h"
//#include "csvhelper.h"
#include <sys/timeb.h>

void read_records(char *csvfile, std::vector<Record*> table)
{
	FILE* file = fopen(csvfile, "r");
	if (!csvfile)
	{
		return -1;
	}
	while(!feof(file))
	{
		// initialize new record.
		Record* tuple = new Record();
		int i = 0;

		//Read all attributes into record.
		for(i = 0; i < numAttribute; i++)
		{
			char* attribute = (char*)malloc(attribute_len);
			if(fread(attribute, attribute_len, 1, file) == 0)
				break;

			//Skip comma and newline
			fgetc(csvfile);
			record->push_back(attribute);
		}

		//If we read the correct number of attributes, add record to records list.
		if(i == num_attributes)
		{
		    records->push_back(record);
		}
	}
	return 0;
}
/**
 * Takes a csv file, reads all the values into records.
 * The records are all added to pages of size page_size.
 * Pages are written to page_file as they fill up.
 */
int main(int argc, char** argv)
{
	//check if the arguments are correct
	if(argc != 4)
	{
		printf("Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>\n");
		return 1;
	}

	//Open page file.
	FILE* stream = fopen(argv[2], "w+");	//"w+b");
	if(stream = NULL)
	{
		//printf("Failed to open page file %s: %s\n", argv[2], strerror(errno));
		perror("Failed to open page file %s: ", argv[2]);
		return 2;
	}

	int page_size = atoi(argv[3]);

	//Get records
	std::vector<Record*> records;	//this is basically a table in memory
	read_records(argv[1], &records);

	//record the time that the insertion started
	struct timeb t;
	ftime(&t);
	long start_time = t.time * 1000 + t.millitm;

	//Create the page
	Page *page = (Page *)malloc(sizeof(Page));;
	init_fixed_len_page(page, page_size, record_size);
	int pagecount = 1;

	//Add records to pages
	for(int i = 0; i < records.size(); i++)
	{
		if(add_fixed_len_page(page, records.at(i)) == -1)
		{
			//The page is full, write page to file.
			fwrite(page->data, page->page_size, 1, stream);
			fflush(stream);

			//Create new page.
			//free_fixed_len_page(page);
			init_fixed_len_page(page, page_size, record_size);
			// rewrite the record to the newly initialize page
			add_fixed_len_page(page, records.at(i));
			pagecount++;
		}
	}

	//write last page to file.
	fwrite(page->data, page->page_size, 1, stream);
	fflush(stream);
	fclose(stream);

	//Release page memory.
	free(page->data);
	free(page);

	//Calculate program end time.
	ftime(&t);
	long end_time = t.time * 1000 + t.millitm;

	//Print metrics.
	printf("NUMBER OF RECORDS: %lu\n", records.size());
	printf("NUMBER OF PAGES: %i\n", pagecount);
	printf("TIME: %lu milliseconds\n", end_time - start_time);
}

