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
	rewind(file);		//make sure that the file stream is at the head of the file
	heapfile->page_size = page_size;
	heapfile->file_ptr = file;
	unsigned char buffer[page_size] = { 0 };
	//long pointer = -1;
	//fwrite(&pointer, sizeof(long), 1, file);
	int entryPerPage = page_size/sizeof(PageEntry);
	PageEntry directoryEntry = (PageEntry *)buffer;
	for(i = 0; i < entryPerPage; i++)
	{
		// initialize all page pointers to -1 (NULL)
		directoryEntry[i].offset = -1;
	}
	fwrite(buffer, page_size, 1, heapfile->file_ptr);
	rewind(heapfile->file_ptr);
}

// Allocate another page in the heapfile. This grows the file by a page.
PageID alloc_page(Heapfile *heapfile)
{
	// 1. seek to the end of file
	// 2. initialize the a page to all zeros by writing past the eof pagesize bytes
	// 3. update the directory by appending a new page entry
	FILE *stream = heapfile->file_ptr;

	// 1. goto the end of the file and extend the file size by 1 page by writing past the end-of-file
	fseek(stream, 0, SEEK_END);
	assert(feof(stream));
	unsigned char array[heapfile->page_size] = { 0x0 };
	size_t size = fwrite(array, heapfile->page_size, 1, stream);
	// making sure that the maximum file size of the filesystem is not exceeded
	assert(feof(stream));
	assert(size == heapfile->page_size);
	if(size != heapfile->page_size)
		printf("error has occurred");

	//save the offset pointer of the page that was just added
	fseek(stream, size, SEEK_END);
	long newpage = ftell(stream);
	rewind(heapfile->file_ptr);
	
	// navigate to the last directory and check if a new page record can be appended to the end or a new directory needs to be created
	FILE *stream1 = heapfile->file_ptr;
	unsigned char buffer[heapfile->page_size] = { 0 };
	long nextdir = 0, currentdir = 0;
	PageEntry *ptr;
	do		//navigate to the last directory
	{
		currentdir = nextdir;
		fseek(stream1, nextdir, SEEK_SET);
		fread(buffer, heapfile->page_size, 1, stream1);
		ptr = (PageEntry *)buffer;
		nextdir = ptr[0].offset;
	}
	while(nextdir > 0);
	
	//PageEntry *entry = (PageEntry *)((unsigned char *)buffer + sizeof(long));
	//int entryPerPage = (pagesize - sizeof(long))/sizeof(PageEntry);
	PageEntry *entry = (PageEntry *)buffer;
	int entryPerPage = pagesize/sizeof(PageEntry) - 1;
	int i;
	for(i = 0; i < entryPerPage; i++)
	{
		if(entry[i].offset < 0)
			break;
	}
	if(i < entryPerPage)
	{
		// can just append a new page entry to the end of the directory (ie. the directory is not full)
		entry[i].freeslots = pagesize;
		entry[i].offset = newpage;
	}
	else
	{
		// the last directory is full and need to create another directory and having that directory point to newly created directory

		//first set the stream1 to the eof in order to create a new directory
		fseek(stream1, 0, SEEK_END);
		//PageEntry directoryEntry[entryPerPage];
		//PageEntry *directoryEntry = (PageEntry *)((unsigned char *)buffer + sizeof(long));
		PageEntry *directoryEntry = (PageEntry *)buffer;
		
		// initialize the directory pointer to -1 as this will be the last directory
		directoryEntry[0].offset = -1;
		directoryEntry[0].freeslots = 0;
		
		directoryEntry[1].offset = newpage;
		directoryEntry[1].freeslots = pagesize;
		for(i = 2; i < entryPerPage; i++)
		{
			// initialize all remaining page pointers to -1
			directoryEntry[i].offset = -1;
		}
		/*
		//unsigned char *newpage[heapfile->page_size];
		long tailPointer = -1;
		fwrite(&tailPointer, sizeof(long), 1, stream1);
		fwrite(directoryEntry, heapfile->page_size - sizeof(long), 1, stream1);
		*/

		fwrite(buffer, heapfile->page_size, 1, stream1);
		assert(feof(stream1));
		fseek(stream1, heapfile->page_size, SEEK_END);
		long newdir = ftell(stream1);		//save the newpage's offset
		
		// now update the pointer of the previous directory to point to the newly created last directory
		fseek(stream1, currentdir, SEEK_SET);
		fread(buffer, heapfile->page_size, 1, stream1);
		ptr = (PageEntry *)buffer;
		ptr[0].offset = newdir;
		fseek(stream1, currentdir, SEEK_SET);
		fwrite(buffer, heapfile->page_size, 1, stream1);
	}
	rewind(heapfile->file_ptr);
}

// Read a page into memory.
void read_page(Heapfile *heapfile, PageID pid, Page *page)
{
	assert(heapfile->page_size == page->page_size);
	FILE *stream = heapfile->file_ptr;
	//if(page == NULL)
	//	page = (Page *)malloc(sizeof(Page));
	PageEntry entry = { -1, 0};
	readHeapfileDirectory(heapfile, pid, &entry);		//(long)pid * heapfile->page_size;
	fseek(stream, entry.offset, SEEK_SET);
	fread(page->data, page->page_size, 1, stream);
	rewind(heapfile->file_ptr);
}

void readHeapfileDirectory(Heapfile *heapfile, PageID pid, PageEntry *entry)
{
	FILE *stream = heapfile->file_ptr;
	int pagesize = heapfile->page_size
	unsigned char *buffer[pagesize];
	//long *ptr;
	long nextdir;
	PageEntry *ptr;
	PageID index = pid;
	entryPerPage = pagesize/sizeof(PageEntry) - 1;	//(pagesize - sizeof(long))/sizeof(PageEntry);
	while(entryPerPage <= index)
	{
		fread(buffer, pagesize, 1, stream);
		ptr = (PageEntry *)buffer;
		nextdir = ptr[0].offset;
		fseek(stream, nextdir, SEEK_SET);
		index -= entryPerPage;
	}
	fread(buffer, 1, heapfile->page_size, stream);
	entry->offset = (((PageEntry *)buffer)[1 + index]).offset;
	entry->freeslots = (((PageEntry *)buffer)[1 + index]).freeslots;
	rewind(heapfile->file_ptr);
}

// Write a page from memory to disk.
void write_page(Page *page, Heapfile *heapfile, PageID pid)
{
	assert(heapfile->page_size == page->page_size);
	FILE *stream = heapfile->file_ptr;
	PageEntry entry = {-1, 0};
	readHeapfileDirectory(heapfile, pid, &entry);
	long offset = pid * heapfile->page_size; 	//entry.offset;
	fseek(stream, offset, SEEK_SET);
	
	fwrite(page->data, page->page_size, 1, stream);
	rewind(heapfile->file_ptr);
}

RecordIterator::RecordIterator(Heapfile *heapfile)
{
	int pagesize = heapfile->page_size;
	this->file = heapfile;
	this->stream = heapfile->file_ptr;
	rewind(this->stream);
	unsigned char buffer[pagesize];
	fread(buffer, pagesize, 1, this->stream);
	PageEntry *iter = (PageEntry *)buffer;
	this->current = buffer[1].offset;
	this->currdir = 0;
}
Record RecordIterator::next()
{
	file = heapfile;
}
bool RecordIterator::hasNext()
{
	int pagesize = heapfile->page_size;
	unsigned char buffer[heapfile->page_size];
	fseek(stream, this->currdir, SEEK_SET);
	fread(buffer, pagesize, 1, stream);
	PageEntry *entry = (PageEntry *)buffer;
	long nextdir;
	if((nextdir = entry[0].offset) < 0)
		return false;
	
}





/*
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
		while(entryPerPage < index)	//navigate to the correct directory
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
		fread(buffer, heapfile->page_size, 1, stream);
		entry = (PageEntry *)((unsigned char *)buffer + sizeof(long));
		entry[index].freeslots = pagesize;
		entry[index].offset = offset;
		fwrite();
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
*/
