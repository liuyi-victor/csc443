#include "library.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
/*
int open_heapfile(Heapfile *heap, char *path, int page_size)//, int slot_size) 
{
	FILE* heap_file = fopen(path, "wb");
	if(!heap_file)
	{
		fprintf(stderr, "Failed to open heap file: %s\n", path);
		free(heap);
		fclose(heap_file);
		return 2;
	}
	heap->page_size = page_size;
	//heap->slot_size = slot_size;
	heap->file_ptr = heap_file;
	return 0;
}
*/
/**
 * Insert records from a csv into an existing heap file.
 */
int main(int argc, char** argv)
{
	if(argc != 4)
	{
		fprintf(stderr, "Usage: %s <heapfile> <csv_file> <page_size>\n", argv[0]);
		return 1;
	}

	//Load records from csv.
	std::vector<Record*> records;
	FILE* csv = fopen(argv[2], "r");
	if (csv == NULL)
	{
		return -1;
	}
	//string delimiter = ",";
	rewind(csv);
	int count = 0;

	//Record tuple;// = new Record();
	//char field[lengthAttribute];
	while(!feof(csv))
	{
		//fread(field, 1100, 1, file)
		Record *tuple = new Record();
		int i;
		for(i = 0; i < numAttribute; i++)
		{
			//read a tuple in the table
			char *field = (char *)malloc(lengthAttribute);
			if(fread(field, lengthAttribute, 1, csv) == 0)
				break;

			//skip comma and newline
			fgetc(csv);
			tuple->push_back(field);
		}
		if(feof(csv))
			break;
		records.push_back(tuple);
		//tuple.clear();
		count++;
	}
	
	
	
	
	assert(records.size() == count);
	/*
	int error = read_records(argv[2], &records);
	if (error) {
		fprintf(stderr, "Could not read records from file: %s\n", argv[2]);
		return 2;
	}
	if(records.size() == 0){
		fprintf(stderr, "No records in file: %s\n", argv[2]);
		return 3;
	}
	*/
	int pagesize = atoi(argv[3]);
	Heapfile* file = (Heapfile*)malloc(sizeof(Heapfile));
	file->file_ptr = fopen(argv[1], "r+");
	if(file->file_ptr == NULL)
		return 4;
	file->page_size = pagesize;
	int slotsize = numAttribute*lengthAttribute;

	Page *page = (Page *)malloc(sizeof(Page));
	page->page_size = pagesize;
	page->slot_size = slotsize;
	init_fixed_len_page(page, pagesize, slotsize);
	int i, result, slots, total = records.size();
	PageID pid = 1;
	PageEntry entry = {-1, -1};
	for(i = 0; i < total; i++)
	{
		while((i < total) &&(result = readHeapfileDirectory(file, pid, &entry) > 0))
		{
			if(entry.freeslots > 0)
			{
				slots = entry.freeslots;
				init_fixed_len_page(page, pagesize, slotsize);
				read_page(file, pid, page);
				while(slots > 0 && i < total)
				{
					result = add_fixed_len_page(page, records.at(i));
					assert(result >= 0);
					slots = slots - 1;
					i++;
				}
				write_page(page, file, pid);
			}
			pid = pid +1;
		}
		if(i >= total)
			break;
		while(i < total)
		{
			pid = alloc_page(file);
			init_fixed_len_page(page, pagesize, slotsize);
			result = readHeapfileDirectory(file, pid, &entry);
			assert(result >= 0);
			slots = entry.freeslots;
			read_page(file, pid, page);
			while(slots > 0 && i < total)
			{
				result = add_fixed_len_page(page, records.at(i));
				assert(result >= 0);
				slots = slots - 1;
				i++;
			}
			write_page(page, file, pid);
		}
	}
	free(page->data);
	free(page);
	fclose(file->file_ptr);
	free(file);





/*





	Page* page = (Page*)malloc(sizeof(Page));
	init_fixed_len_page(page, pagesize, slotsize);
	
	unsigned char buffer[pagesize] = { 0 };
	long currdir = 0, nextdir = 0;
	int total = table.size();
	int index = 0;		//index to the tuple number of the table to be inserted
	int free;
	int i, entryPerPage = pagesize/sizeof(PageEntry) - 1;
	bool reachedEnd = false;
	FILE *stream = heap->file_ptr;
	PageEntry *ptr;
	while(!reachedEnd && index < total)
	{
		fseek(stream, currdir, SEEK_SET);
		fread(buffer, pagesize, 1, stream);
		ptr = (PageEntry *)buffer;
		nextdir = ptr[0].offset;
		for(i = 1; i <= entryPerPage; i++)
		{
			if(ptr[i].offset > 0 && ptr[i].freeslots != 0)
			{
				fseek(stream, ptr[i].offset, SEEK_SET);
				add_fixed_len_page(page, );
			}
		}
		free = ptr
		if(nextdir < 0)
			reachedEnd = true;
		buffer
		
	}
	Page* directory_page = (Page*)malloc(sizeof(Page*));

	PageID current_id = 0; // increments at the end of the loop and from seek
	int records_exhausted = 0;
	while (records_exhausted < records.size()) 
	{
		current_id = seek_page(page, directory_page, current_id, heap, false);

		// if no free page exists, we need to create a new one to insert into.
		if (current_id == -1) 
		{
			current_id = alloc_page(heap);
			read_page(heap, current_id, page);
		}

		// insert a record into each free slot, short-circuit if we run out
		std::vector<int> freeslots = fixed_len_page_freeslot_indices(page);
		for (int i = 0; i < freeslots.size(); i++) 
		{
			write_fixed_len_page(page, freeslots[i], records[records_exhausted]);
			records_exhausted++;

			if (records_exhausted >= records.size())
				break;
		}

		// only write the page if we would have emptied some records into it,
		// which occurs when there are some free record slots.
		if (freeslots.size() > 0) 
		{
			write_page(page, heap, current_id);
		}

		current_id++;
	}

	free(page);
	free(directory_page);

	fclose(heap->file_ptr);
	free(heap);
*/
	return 0;
}
