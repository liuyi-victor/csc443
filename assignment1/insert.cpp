#include "library.h"
#include "csvhelper.h"
#include <stdbool.h>
#include <stdlib.h>

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
	int error = read_records(argv[2], &records);
	if (error) {
		fprintf(stderr, "Could not read records from file: %s\n", argv[2]);
		return 2;
	}
	if(records.size() == 0){
		fprintf(stderr, "No records in file: %s\n", argv[2]);
		return 3;
	}

	Heapfile* heap = (Heapfile*)malloc(sizeof(Heapfile));
	int pagesize = atoi(argv[3]);
	int slotsize = numAttribute*lengthAttribute;
	if (open_heapfile(heap, argv[1], pagesize, record_size) != 0) 
	{
		return 4;
	}
	
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

	return 0;
}
