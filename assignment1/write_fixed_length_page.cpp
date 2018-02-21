#include "library.h"
#include <sys/timeb.h>
#include <assert.h>

//g++ -g write_fixed_length_page.cpp library.cpp -o write_page

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
void print_record(Record *record)
{
	//Iterate all records and print the 10 bytes as characters.
	for(int i = 0; i < record->size() -1; i++)
	{
		printf("%s, ", record->at(i));
	}
	//Print the last variable with no trailing comma.
	printf("%s\n", record->at(record->size()-1));
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
	FILE* stream = fopen(argv[2], "w+");
	if(stream == NULL)
	{
		printf("Failed to open page file: %s\n", argv[2]);
		return 2;
	}

	int page_size = atoi(argv[3]);

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

	//record the time that the insertion started
	struct timeb t;
	ftime(&t);
	long start_time = t.time * 1000 + t.millitm;

	//Create the page
	Page *page = (Page *)malloc(sizeof(Page));;
	init_fixed_len_page(page, page_size, numAttribute*lengthAttribute);
	int pagecount = 0;
	//printf("reached here!\n");
	//Add records to pages
	for(int i = 0; i < table.size(); i++)
	{
		//print_record(table.at(i));
		if(add_fixed_len_page(page, table.at(i)) == -1)
		{
			//The page is full, write page to file.
			fwrite(page->data, page->page_size, 1, stream);
			fflush(stream);

			//Create new page.
			//free_fixed_len_page(page);
			init_fixed_len_page(page, page_size, numAttribute*lengthAttribute);
			// rewrite the record to the newly initialize page
			add_fixed_len_page(page, table.at(i));
			pagecount++;
		}
	}

	//write last page to file.
	fwrite(page->data, page->page_size, 1, stream);
	pagecount++;
	fflush(stream);
	fclose(stream);

	//Calculate program end time.
	ftime(&t);
	long end_time = t.time * 1000 + t.millitm;

	//Release page memory.
	free(page->data);
	free(page);

	//Print metrics.
	printf("NUMBER OF RECORDS: %d\n", table.size());
	printf("NUMBER OF PAGES: %i\n", pagecount);
	printf("TIME: %lu milliseconds\n", end_time - start_time);
}

