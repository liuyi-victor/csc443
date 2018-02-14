#include "library.h"
#include <string.h>
#include <math.h>

//record functions
/*
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record)
{
	int i, size = 0;
	for(i = 0; i < record->size(); i++)
	{
		size += 10;	//strlen(record[i]);
	}
	return size;
}

/*
 * Serialize the record to a byte array to be stored in buf
 */
void fixed_len_write(Record *record, void *buf)
{
	int i;
	void *iter = buf;
	if(buf != NULL && sizeof(buf) >= fixed_len_sizeof(record))
	{
		for(i = 0; i < record->size(); i++)
		{
			memcpy(iter, record[i], 10);	//strcat(buf, *it);
			iter += 10;
		}
	}	
}

void fixed_len_read(void *buf, int size, Record *record)
{
	int i;
	void *iter = buf;
	for(i = 0; i < record->size() && size > 0; i++)
	{
		if(size >= 10)
		{
			strncpy(record[i], iter, 10);
			iter += 10;
			size -= 10;
		}
		else
		{
			strncpy(record[i], iter, size);
			iter += size;
			//size = 0 
		}
	}
}

//page functions
void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
	if(page == NULL)
	{
		page = (Page *)malloc(sizeof(Page));
	}
	page->data = malloc(sizeof(page_size));
	page->page_size = page_size;
	page->slot_size = slot_size;
	//page->used_slots = 0;
	int capacity = fixed_len_page_capacity(page);
	//page->bitmapLength = 1 + ((capacity-1) / BYTE_SIZE);
	int bitmapLength = ceil(capacity/BYTE_SIZE);	// round up (capacity / BYTE_SIZE)
	memset(page->data, capacity, sizeof(int));
	memset(((unsigned char *)page->data + sizeof(int)), '\0', bitmapLength);	//page->bitmapLength);
}

int fixed_len_page_capacity(Page *page)
{
	int numer = BYTE_SIZE*(page->page_size - sizeof(int));
	int denom = BYTE_SIZE * page->slot_size + 1;
	if(numer % denom == 0)
		return numer/denom;
	else
		return (numer - 8)/ denom;
}

int fixed_len_page_freeslots(Page *page)
{
	//return fixed_len_page_capacity(page) - page->used_slots;
	int bitmapLength = 
}

int add_fixed_len_page(Page *page, Record *r)
{
	if(page == NULL || fixed_len_page_freeslots(page) == 0)
		return -1;
	// get the metadata of the page
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = ceil(capacity/BYTE_SIZE);	// round up (capacity / BYTE_SIZE)
	(unsigned char *)bitmap = (unsigned char *)malloc(bitmapLength);
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
	// find a free slot marked by a bit 0 in the corresponding directory
	int i, j, slot;
	for(i = 0; i < bitmapLength; i++)
	{
		if(bitmap[i] != 0xff)
		{
			unsigned char value = bitmap[i]; 
			for(j = 0; j < BYTE_SIZE && value & 0b1 != 0; j++)
				value = value >> 1;
			bitmap[i] ^= 0b1 << j;		//toggle the bit
			slot = BYTE_SIZE * i + j;	//record the slot index of the first free slot
			break;
		}
	}
	if(i >= bitmapLength)
		return -1;
	/*
	int tuple_size = fixed_len_sizeof(r);
	//get the pointer to the memory that directly points to slot for the tuple
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + bitmapLength + slot * tuple_size;
	for(j = 0; j < r->size(); j++)
	{
		strncpy(tuple, r[j], 100);
		tuple = tuple + 100;
	}
	*/
	write_fixed_len_page(page, slot, r);
	return slot;
}

void write_fixed_len_page(Page *page, int slot, Record *r)
{
	int tuple_size = fixed_len_sizeof(r);
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = ceil(capacity/BYTE_SIZE);
	//get the pointer to the memory that directly points to slot for the tuple
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + bitmapLength + slot * tuple_size;
	for(j = 0; j < r->size(); j++)
	{
		strncpy(tuple, r[j], 100);
		tuple = tuple + 100;
	}
}

void read_fixed_len_page(Page *page, int slot, Record *r)
{
	if(r == NULL)
		r = std::vector<V>();
	int tuple_size = fixed_len_sizeof(r);
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = ceil(capacity/BYTE_SIZE);
	//get the pointer to the memory that directly points to slot for the tuple
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + bitmapLength + slot * tuple_size;
	unsigned char *buffer = malloc(lengthAttribute);
	for(j = 0; j < numAttribute; j++)
	{
		strncpy(buffer, tuple, lengthAttribute);
		r->pushback(buffer);
		tuple = tuple + lengthAttribute;
	}
}

// Initialize a heapfile to use the file and page size given.
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file)
{
	if(heapfile == NULL)
	{
		heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	}
	heapfile->page_size = page_size;
	heapfile->file_ptr = file;
}

// Allocate another page in the heapfile. This grows the file by a page.
PageID alloc_page(Heapfile *heapfile)
{
	FILE *stream = heapfile->file_ptr;
	writeHeapfileDirectory
	fseek(stream, 0, SEEK_END);
	unsigned char array[heapfile->page_size] = { 0x0 };
	fwrite(array, heapfile->page_size, 1, stream);
	rewind(heapfile->page_size);
}

// Read a page into memory.
void read_page(Heapfile *heapfile, PageID pid, Page *page)
{
	assert(heapfile->page_size == page->page_size);
	FILE *stream = heapfile->file_ptr;
	//if(page == NULL)
	//	page = (Page *)malloc(sizeof(Page));
	long offset = (long)pid * heapfile->page_size;
	fseek(stream, offset, SEEK_SET);
	fread(page->data, page->page_size, 1, stream);
}

void readHeapfileDirectory(Heapfile *heapfile, PageID id, PageEntry *entry)
{
	FILE *stream = heapfile->file_ptr;
	int pagesize = heapfile->page_size
	unsigned char *buffer[pagesize];
	long *ptr;
	PageID index = pid;
	entryPerPage = (pagesize - sizeof(long))/sizeof(PageEntry);
	while(entryPerPage < index)
	{
		fread(buffer, heapfile->page_size, 1, stream);
		ptr = (long *)buffer;
		nextdir = ptr[0];
		fseek(stream, nextdir, SEEK_SET);
		index -= entryPerPage;
	}
	fread(buffer, 1, heapfile->page_size, stream);
	
	buffer = (PageEntry)buffer;
	page = 
}
void writeHeapfileDirectory(Heapfile *heapfile, PageID pid, unsigned char isAllocate)
{
	FILE *stream = heapfile->file_ptr;
	int pagesize = heapfile->page_size
	long *ptr;
	long nextdir;
	unsigned char *buffer[pagesize];
	PageEntry *entry;
	if(pid > 0)
	{
		PageID index = pid;
		entryPerPage = (pagesize - sizeof(long))/sizeof(PageEntry);
		while(entryPerPage < index)
		{
			// assuming page ID is reasonable
			fread(buffer, heapfile->page_size, 1, stream);
			ptr = (long *)buffer;
			nextdir = ptr[0];
			if(nextdir < 0)
				return;
			fseek(stream, nextdir, SEEK_SET);
			index -= entryPerPage;
		}
		fread(ptr, heapfile->page_size, 1, stream);
		entry[index].
	}
	else if(pid < 0)
	{
		
	}
	if(isAllocate > 0)
	{
		buffer = malloc(pagesize);
		fread(buffer, heapfile->page_size, 1, stream);
		ptr = (long *)buffer;
		entry = (PageEntry *)((unsigned char *)buffer + sizeof(long));
		long nextdir = buffer
		if((pagesize - sizeof(long))/sizeof(PageEntry) < pid)
	}
	else
	{
		// free the specified page
		PageID index = pid;
		entryPerPage = (pagesize - sizeof(long))/sizeof(PageEntry);
		while(entryPerPage < index)
		{
			fread(ptr, heapfile->page_size, 1, stream);
			nextdir = ptr[0];
			fseek(stream, nextdir, SEEK_SET);
			index -= entryPerPage;
		}
		fread(ptr, heapfile->page_size, 1, stream);
		entry[index].
	}
}

// Write a page from memory to disk.
void write_page(Page *page, Heapfile *heapfile, PageID pid)
{
	assert(heapfile->page_size == page->page_size);
	//FILE *stream = heapfile->file_ptr;
	//PageEntry entry = PageEntry(0, 0);
	//readHeapfileDirectory(heapfile, );
	long offset = pid * heapfile->page_size; 	//entry.offset;
	fseek(stream, offset, SEEK_SET);
	fwrite(page->data, page->page_size, 1, stream);
}

RecordIterator::RecordIterator(Heapfile *heapfile)
{
	
}
