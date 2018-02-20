#include "library.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

//record functions
/*
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record)
{
	int i, size = 0;
	for(i = 0; i < record->size(); i++)
	{
		size += lengthAttribute;	//strlen(record[i]);
	}
	return size;
}

/*
 * Serialize the record to a byte array to be stored in buf
 */
void fixed_len_write(Record *record, void *buf)
{
	int i;
	char *iter = (char *)buf;
	if(buf != NULL)	// && sizeof(buf) >= fixed_len_sizeof(record))
	{
		for(i = 0; i < record->size(); i++)
		{
			memcpy(iter, record->at(i), lengthAttribute);	//strcat(buf, *it);
			iter += lengthAttribute;
		}
	}	
}

/*
 * Deserialize 'size' bytes from the 'buf' buffer, and
 * stores the record in 'record'
 */
void fixed_len_read(void *buf, int size, Record *record)
{
	int i;
	char *iter = (char *)buf;
	//char buffer[lengthAttribute] = { 0 };
	//for(i = 0; i < record->size() && size > 0; i++)
	while(size > 0)
	{
		char *buffer = (char *)malloc(lengthAttribute);
		if(size >= lengthAttribute)
		{
			strncpy(buffer, iter, lengthAttribute);
			record->push_back(buffer);
			iter += lengthAttribute;
			size -= lengthAttribute;
		}
		else
		{
			strncpy(buffer, iter, size);
			record->push_back(buffer);
			iter += size;
			size = 0;
		}
	}
}

//page functions
void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
	//if(page == NULL)
	//{
	//	page = (Page *)malloc(sizeof(Page));
	//}
	if(page->data == NULL)
		page->data = malloc(page_size);
	else
		memset(page->data, 0, page_size);
	page->page_size = page_size;
	page->slot_size = slot_size;
	//page->used_slots = 0;
	int capacity = fixed_len_page_capacity(page);
	//page->bitmapLength = 1 + ((capacity-1) / BYTE_SIZE);
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	//memset(page->data, capacity, sizeof(int));
	((int *)page->data)[0] = capacity;
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
	int i = 0, count = 0;
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	//unsigned char bitmap[bitmapLength];
	unsigned char *bitmap = (unsigned char *)malloc(bitmapLength);
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
	int remaining = capacity;
	//for(i = 0; i < capacity; i++)
	while(remaining > 0)
	{
		uint8_t byte = bitmap[i];
		if(remaining >= BYTE_SIZE)
			count += ((byte & 0x1)+((byte>>1) & 0x1)+((byte>>2) & 0x1)+((byte>>3) & 0x1)+((byte>>4) & 0x1)+((byte>>5) & 0x1)+((byte>>6) & 0x1)+((byte>>7) & 0x1));
		else
		{
			byte = byte >> (BYTE_SIZE - remaining);
			do
			{
				count += byte & 0x1;
				byte = byte >> 1;
				remaining--;
			}
			while(remaining > 0);
		}
		i++;
		remaining = remaining - BYTE_SIZE;
	}
	free(bitmap);
	return capacity-count;
}

int add_fixed_len_page(Page *page, Record *r)
{
	if(page == NULL || fixed_len_page_freeslots(page) <= 0)
		return -1;
	// get the metadata of the page
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	unsigned char *bitmap = (unsigned char *)malloc(bitmapLength);
	//unsigned char bitmap[bitmapLength];
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
	// find a free slot marked by a bit 0 in the corresponding directory
	int i, j, slot;
	uint8_t flag = 0x80;
	for(i = 0; i < bitmapLength; i++)
	{
		if(bitmap[i] != 0xff)
		{
			
			unsigned char value = bitmap[i]; 
			for(j = 0; j < BYTE_SIZE && ((value & 0x80) != 0); j++)
				value = value << 1;
			bitmap[i] |= (flag >> j);		//toggle the bit
			slot = BYTE_SIZE * i + j;	//record the slot index of the first free slot
			break;
		}
	}
	if(i >= bitmapLength)
	{
		free(bitmap);
		return -1;
	}
	memcpy(((unsigned char *)page->data + sizeof(int)), bitmap, bitmapLength);		//write back the changed directory
	//printf("writing a record to page\n");
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
	free(bitmap);
	return slot;
}

void write_fixed_len_page(Page *page, int slot, Record *r)
{
	int j;
	int tuple_size = page->slot_size;	//fixed_len_sizeof(r);
	int capacity = fixed_len_page_capacity(page);
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	if(r == NULL)
	{
		//delete
		// NO NEED: unsigned char bitmap[bitmapLength] = { 0 };
		//index is the byte number in the directory
		int index = slot/BYTE_SIZE;
		unsigned char *bitmap = (unsigned char *)page->data + sizeof(int);
		//index2 is the bit number in the directory byte
		int index2 = slot%BYTE_SIZE;
		//uint8_t byte = bitmap[index];
		uint8_t flag = 0x80;
		if(((bitmap[index])&(flag>>index2)) == 0)	//WATCH OUT: the precedence of comparison and bitwise operators
		{
			return;		//the slot is empty (no record)
		}
		else
		{
			bitmap[index] &= ~(flag>>index2);
		}
	}
	//get the pointer to the memory that directly points to slot for the tuple
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + bitmapLength + slot * tuple_size;
	for(j = 0; j < r->size(); j++)
	{
		memcpy(tuple, r->at(j), lengthAttribute);
		tuple = tuple + lengthAttribute;
	}
}

void read_fixed_len_page(Page *page, int slot, Record *r)
{
	int j;
	int tuple_size = page->slot_size;
	int capacity = fixed_len_page_capacity(page);
	//int bitmapLength = ceil(capacity/BYTE_SIZE);
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	//check that the specified record at slot has content
	unsigned char *bitmap = (unsigned char *)malloc(bitmapLength);
	//unsigned char bitmap[bitmapLength];
	
	//index is the byte number in the directory
	int index = slot/BYTE_SIZE;
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
	//index2 is the bit number in the directory byte
	int index2 = slot%BYTE_SIZE;
	uint8_t byte = bitmap[index];
	uint8_t flag = 0x80;
	if(((byte)&(flag>>index2)) == 0)	//WATCH OUT: the precedence of comparison and bitwise operators
	{
		free(bitmap);
		return;		//the slot is empty (not record)
	}
		
	//get the pointer to the memory that directly points to slot for the tuple
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + bitmapLength + slot * tuple_size;
	//char *ptr = tuple;
	//char buffer[lengthAttribute]= { 0 };
	for(j = 0; j < numAttribute; j++)
	{
		char *buffer = (char *)malloc(lengthAttribute);
		memcpy(buffer, tuple, lengthAttribute);
		r->push_back(buffer);
		tuple = tuple + lengthAttribute;
	}
	free(bitmap);
	//free(tuple);
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
	//unsigned char buffer[page_size] = { 0 };
	unsigned char *buffer = (unsigned char *)malloc(page_size);
	
	//long pointer = -1;
	//fwrite(&pointer, sizeof(long), 1, file);
	int entryPerPage = page_size/sizeof(PageEntry);
	PageEntry *directoryEntry = (PageEntry *)buffer;
	int i;
	for(i = 0; i < entryPerPage; i++)
	{
		// initialize all page pointers to -1 (NULL)
		directoryEntry[i].offset = -1;
		directoryEntry[i].freeslots = -1;
	}
	fwrite(buffer, page_size, 1, heapfile->file_ptr);
	fflush(heapfile->file_ptr);
	rewind(heapfile->file_ptr);
	free(buffer);
}

// Allocate another page in the heapfile. This grows the file by a page.
PageID alloc_page(Heapfile *heapfile)
{
	// 1. seek to the end of file
	// 2. initialize the a page to all zeros by writing past the eof pagesize bytes
	// 3. update the directory by appending a new page entry
	FILE *stream = heapfile->file_ptr;
	Page *page = (Page *)malloc(sizeof(Page));
	page->page_size = heapfile->page_size;
	page->slot_size = numAttribute*lengthAttribute;
	int capacity = fixed_len_page_capacity(page);
	int freeslots = capacity;
	
	// 1. goto the end of the file and extend the file size by 1 page by writing past the end-of-file
	fseek(stream, 0, SEEK_END);
	if(!feof(stream))
		fgetc(stream);
	assert(feof(stream));
	long newpage = ftell(stream);
	//printf("the offset of the new page is %d\n", newpage);
	//unsigned char array[heapfile->page_size] = { 0x0 };
	unsigned char *buffer = (unsigned char *)malloc(heapfile->page_size);
	memset(buffer, '\0', heapfile->page_size);
	size_t size = fwrite(buffer, heapfile->page_size, 1, stream);
	fflush(stream);
	// making sure that the maximum file size of the filesystem is not exceeded
	assert(feof(stream));
	//assert(size == heapfile->page_size);
	//printf("written %d bytes to heapfile\n", size);
	if(size == 0)
		printf("Heapfile alloc_page: error has occurred\n");

	//save the offset pointer of the page that was just added
	//fseek(stream, size, SEEK_END);
	//long newpage = ftell(stream);
	//printf("the temp and newpage index are: %d and %d", temp, newpage);
	rewind(heapfile->file_ptr);
	
	// navigate to the last directory and check if a new page record can be appended to the end or a new directory needs to be created
	FILE *stream1 = heapfile->file_ptr;
	//unsigned char buffer[heapfile->page_size] = { 0 };
	//unsigned char *buffer = (unsigned char *)malloc(heapfile->page_size);
	
	long nextdir = 0, currentdir = 0;
	int count = -1, index =0;
	PageEntry *ptr;
	do		//navigate to the last directory
	{
		count++;
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
	int entryPerPage = heapfile->page_size/sizeof(PageEntry) - 1;
	int i;
	for(i = 1; i <= entryPerPage; i++)
	{
		if(entry[i].offset < 0)		//found an empty slot in directory
			break;
	}
	if(i <= entryPerPage)
	{
		// can just append a new page entry to the end of the directory (ie. the directory is not full)
		entry[i].freeslots = freeslots;
		entry[i].offset = newpage;
		//DO NOT FORGET TO WRITE BACK THE UPDATED DIRECTORY!
		fseek(stream1, currentdir, SEEK_SET);
		fwrite(buffer, heapfile->page_size, 1, stream1);
		fflush(stream1);
	}
	else
	{
		// the last directory is full and need to create another directory and having that directory point to newly created directory

		//first set the stream1 to the eof in order to create a new directory
		fseek(stream1, 0, SEEK_END);
		if(!feof(stream1))
			fgetc(stream);
		assert(feof(stream1));
		//PageEntry directoryEntry[entryPerPage];
		//PageEntry *directoryEntry = (PageEntry *)((unsigned char *)buffer + sizeof(long));
		PageEntry *directoryEntry = (PageEntry *)buffer;
		
		// initialize the directory pointer to -1 as this will be the last directory
		directoryEntry[0].offset = -1;
		directoryEntry[0].freeslots = -1;
		
		directoryEntry[1].offset = newpage;
		directoryEntry[1].freeslots = freeslots;
		for(i = 2; i <= entryPerPage; i++)
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
		fpos_t position;
		fgetpos (stream1, &position);
	//assert(feof(stream1));
		fwrite(buffer, heapfile->page_size, 1, stream1);
		fflush(stream1);
		assert(feof(stream1));

		fsetpos(stream1, &position);
		//fseek(stream1, heapfile->page_size, SEEK_END);
		long newdir = ftell(stream1);		//save the newpage's offset
		
		// now update the pointer of the previous directory to point to the newly created last directory
		fseek(stream1, currentdir, SEEK_SET);
		fread(buffer, heapfile->page_size, 1, stream1);
		ptr = (PageEntry *)buffer;
		ptr[0].offset = newdir;
		fseek(stream1, currentdir, SEEK_SET);
		fwrite(buffer, heapfile->page_size, 1, stream1);
		fflush(stream1);
	}
	fseek(stream1, 0, SEEK_END);
	long filesize = ftell(stream1);
	rewind(heapfile->file_ptr);
	assert(newpage%(heapfile->page_size) == 0);
	free(buffer);
	//free(array);
	return (count*entryPerPage)+i;
}

// Read a page into memory.
int read_page(Heapfile *heapfile, PageID pid, Page *page)
{
	assert(heapfile->page_size == page->page_size);
	FILE *stream = heapfile->file_ptr;
	//if(page == NULL)
	//	page = (Page *)malloc(sizeof(Page));
	PageEntry entry = { -1, 0};
	int result = readHeapfileDirectory(heapfile, pid, &entry);		//(long)pid * heapfile->page_size;
	if(result < 0)
		return result;
	fseek(stream, entry.offset, SEEK_SET);
	size_t size = fread(page->data, page->page_size, 1, stream);
	rewind(heapfile->file_ptr);
	return result;
}

int readHeapfileDirectory(Heapfile *heapfile, PageID pid, PageEntry *entry)
{
	// the first PageEntry (with index 0) is the pointer to the next directory
	rewind(heapfile->file_ptr);
	FILE *stream = heapfile->file_ptr;
	
	int pagesize = heapfile->page_size;
	//unsigned char *buffer[pagesize];
	unsigned char *buffer = (unsigned char *)malloc(pagesize);
	
	//long *ptr;
	long nextdir;
	PageEntry *ptr;
	PageID index = pid - 1;
	int entryPerPage = (pagesize/sizeof(PageEntry)) - 1;	//(pagesize - sizeof(long))/sizeof(PageEntry);
	bool error = false;
	while(entryPerPage <= index)
	{
		fread(buffer, pagesize, 1, stream);
		ptr = (PageEntry *)buffer;
		nextdir = ptr[0].offset;
		if(nextdir < 0)
		{
			free(buffer);
			return -1;
		}
		fseek(stream, nextdir, SEEK_SET);
		index -= entryPerPage;
	}
	fread(buffer, heapfile->page_size, 1, stream);
	if((((PageEntry *)buffer)[1+index]).offset < 0)
	{
		free(buffer);
		return -1;
	}
	entry->offset = (((PageEntry *)buffer)[1+index]).offset;
	entry->freeslots = (((PageEntry *)buffer)[1+index]).freeslots;
	rewind(heapfile->file_ptr);
	free(buffer);
	return 1;
}

void writeHeapfileDirectory(Heapfile *heapfile, PageID pid, PageEntry *entry)
{
	rewind(heapfile->file_ptr);
	FILE *stream = heapfile->file_ptr;
	int pagesize = heapfile->page_size;
	//unsigned char *buffer[pagesize];
	unsigned char *buffer = (unsigned char *)malloc(pagesize);
	
	//long *ptr;
	long nextdir;
	PageEntry *ptr;
	PageID index = pid - 1;
	int entryPerPage = (pagesize/sizeof(PageEntry)) - 1;	//(pagesize - sizeof(long))/sizeof(PageEntry);
	//rewind(stream);
	while(entryPerPage <= index)
	{
		fread(buffer, pagesize, 1, stream);
		ptr = (PageEntry *)buffer;
		nextdir = ptr[0].offset;
		fseek(stream, nextdir, SEEK_SET);
		index -= entryPerPage;
	}
	//save the current stream position
	fpos_t position;
	fgetpos (stream, &position);
	
	//read the current directory page
	fread(buffer, heapfile->page_size, 1, stream);
	
	//set the directory entry for the page
	ptr = (PageEntry *)buffer;
	if(ptr[1+index].offset < 0)
	{
		free(buffer);
		return;
	}
	ptr[1+index].freeslots = entry->freeslots;
	
	//write back the updated directory entry
	fsetpos(stream, &position);
	fwrite(buffer, heapfile->page_size, 1, stream);
	fflush(stream);
	free(buffer);
}
// Write a page from memory to disk.
void write_page(Page *page, Heapfile *heapfile, PageID pid)
{
	assert(heapfile->page_size == page->page_size);
	FILE *stream = heapfile->file_ptr;
	PageEntry entry = {-1, 0};
	int result = readHeapfileDirectory(heapfile, pid, &entry);
	if(result < 0)
		return;
	long offset = entry.offset;
	fseek(stream, offset, SEEK_SET);
	
	fwrite(page->data, page->page_size, 1, stream);
	fflush(stream);
	rewind(heapfile->file_ptr);
	entry.freeslots = fixed_len_page_freeslots(page);
	writeHeapfileDirectory(heapfile, pid, &entry);
	rewind(heapfile->file_ptr);
}

RecordIterator::RecordIterator(Heapfile *heapfile)	//completed
{
	int pagesize = heapfile->page_size;
	this->file = heapfile;
	//this->stream = heapfile->file_ptr;
	rewind(this->file->file_ptr);
	//unsigned char buffer[pagesize];
	unsigned char *buffer = (unsigned char *)malloc(pagesize);
	
	fread(buffer, pagesize, 1, heapfile->file_ptr);
	PageEntry *iter = (PageEntry *)buffer;
	//this->current = iter[1].offset;
	this->slot = 0;
	this->currdir = 0;
	this->pid = 1;
	this->index = 1;
	this->end = false;
	free(buffer);
}
int RecordIterator::searchPageNext(Page *page, int start, int capacity)
{
	int tempnewslot = start;
	int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
	//unsigned char bitmap[bitmapLength];
	unsigned char *bitmap = (unsigned char *)malloc(bitmapLength);
	
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
	// find an allocated slot marked by a bit 1 in the corresponding directory
	int i, slot, j = 0;
	uint8_t flag = 0x80;
	for(i = tempnewslot/BYTE_SIZE; i < bitmapLength; i++)
	{
		if(bitmap[i] != 0x00)
		{
		
			uint8_t value = bitmap[i]; 
			value = value << (tempnewslot%BYTE_SIZE);
			j = tempnewslot%BYTE_SIZE;
			while((j < BYTE_SIZE) && ((value & flag) == 0))
			{
				value = value << 1;
				tempnewslot++;
				j++;
			}
			//if(tempnewslot%BYTE_SIZE != 0)
			if(j < BYTE_SIZE)
			{
				this->slot = tempnewslot;
				free(bitmap);
				return 0;
			}
		}
	}
	free(bitmap);
	return -1;	//did not found the next allocated record in current page
}
int RecordIterator::updatenext(Page *page)
{
	int slotsize = numAttribute*lengthAttribute;
	int capacity = fixed_len_page_capacity(page);
	int maxslots = capacity;
	int tempnewslot = this->slot + 1;
	int tempnewpage = this->pid;
	int result, searchResult;
	//bool found = false;
	PageEntry entry = {-1, 0};
	if(tempnewslot >= capacity)
	{
		//if reached the end of the current page, try look for next record in the subsequent pages
		tempnewpage = this->pid + 1;
		tempnewslot = 0;
	}
	while((result = readHeapfileDirectory(this->file, tempnewpage, &entry)) >= 0)
	{
		//result = readHeapfileDirectory(this->file, this->pid, &entry);
		if(entry.freeslots > 0 && (entry.freeslots < maxslots))
		{
			init_fixed_len_page(page, page->page_size, slotsize);
			read_page(this->file, tempnewpage, page);
			searchResult = this->searchPageNext(page, tempnewslot, capacity);
			
			if(searchResult >= 0)
			{
				this->pid = tempnewpage;
				//this->slot = tempnewslot;
				return 0;
			}
		}
		tempnewslot = 0;
		tempnewpage += 1;
	}
	this->end = true;
	return -1;
}
Record RecordIterator::next()	//incomplete
{
	//Record *record = new Record();
	Record record;
	
	FILE *stream = this->file->file_ptr;
	int j, pagesize = this->file->page_size;
	Page *page = (Page *)malloc(sizeof(Page));
	page->data = NULL;
	init_fixed_len_page(page, pagesize, numAttribute*lengthAttribute);
	
	//get the offset pointer to the current page
	PageEntry entry = {-1, 0};
	//unsigned char buffer[pagesize] = { 0 };
	//unsigned char *buffer = (unsigned char *)malloc(pagesize);
	
	int result = readHeapfileDirectory(this->file, this->pid, &entry);
	assert(result >= 0);
	
	//read the current page
	//fseek(stream, entry.offset, SEEK_SET);
	//fread(buffer, pagesize, 1, stream);
	read_page(this->file, this->pid, page);
	read_fixed_len_page(page, this->slot, &record);
	
	updatenext(page);
	//free(buffer);
	free(page->data);
	free(page);
	return record;
	/*
	//try to find the next record in the current page
	int capacity = fixed_len_page_capacity(page);
	int tempnewslot = this->slot + 1;
	bool found = false;
	if(tempnewslot < capacity)
	{
		int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
		unsigned char bitmap[bitmapLength];
		memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
		// find an allocated slot marked by a bit 1 in the corresponding directory
		int i, slot;
		uint8_t flag = 0x80;
		for(i = tempnewslot/BYTE_SIZE; i < bitmapLength; i++)
		{
			if(bitmap[i] != 0x00)
			{
			
				uint8_t value = bitmap[i]; 
				value = value << (tempnewslot%BYTE_SIZE);
				while((tempnewslot%BYTE_SIZE != 0) && ((value | 0x00) == 0))
				{
					value = value << 1;
					tempnewslot++;
				}
				if(tempnewslot%BYTE_SIZE != 0)
				{
					this->slot = tempnewslot
					return record;
				}
			}
		}
	}
	
	read_fixed_len_page(this->page, this->slot, &record);
	if(!this->hasNext())
		return record;
	record = new Record();
	int entryPerPage = (this->file)->page_size/sizeof(PageEntry) - 1;
	int pagesize = (this->file)->page_size;
	//read the current directory page and get pointer to the current record
	unsigned char buffer[pagesize];
	fseek(stream, this->currdir, SEEK_SET);
	fread(buffer, pagesize, 1, stream);
	
	
	
	
	
	
	
	
	//goto the next record
	//if(entry.offset < 0)
	//	this->page = this->page + 1;
	for(j = 0; j < numAttribute; j++)
	{
		char *buffer = (char *)malloc(lengthAttribute);
		memcpy(buffer, tuple, lengthAttribute);
		record->push_back(buffer);
		tuple = tuple + lengthAttribute;
	}
	return record;*/
}
bool RecordIterator::hasNext()	//incompleted
{
	if(this->end)
		return false;
	else
		return true;
	/*
	int entryPerPage = (this->file)->page_size/sizeof(PageEntry) - 1;
	if(this->index < entryPerPage)
		return true;
	int pagesize = (this->file)->page_size;

	//read the current directory and see if this is the last directory
	unsigned char buffer[pagesize];
	fseek(this->file->file_ptr, this->currdir, SEEK_SET);
	fread(buffer, pagesize, 1, file->file_ptr);
	PageEntry *entry = (PageEntry *)buffer;
	long nextdir;
	rewind(this->file->file_ptr);
	//if the iterator is not even at the last directory, then must not be the last record
	if((entry[0].offset) > 0)
		return true;
	else
		return false;

	*/
	
	
	
	/*
	int i, entryPerPage = (this->file)->page_size/sizeof(PageEntry) - 1;
	for(i = 1; i < entryPerPage; i++)
	{
		if(entry[i].offset == current)
			break;
	}
	if(i < entryPerPage)
		return false;
	else
		return true;
	*/
	//fseek(stream, this->current, SEEK_SET);
	//fread(buffer, pagesize, 1, stream);
}

/*
		int bitmapLength = (capacity/BYTE_SIZE) + (capacity%BYTE_SIZE == 0 ? 0 : 1);
		unsigned char bitmap[bitmapLength];
		memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), bitmapLength);
		// find an allocated slot marked by a bit 1 in the corresponding directory
		int i, slot;
		uint8_t flag = 0x80;
		for(i = tempnewslot/BYTE_SIZE; i < bitmapLength; i++)
		{
			if(bitmap[i] != 0x00)
			{
			
				uint8_t value = bitmap[i]; 
				value = value << (tempnewslot%BYTE_SIZE);
				while((tempnewslot%BYTE_SIZE != 0) && ((value | 0x00) == 0))
				{
					value = value << 1;
					tempnewslot++;
				}
				if(tempnewslot%BYTE_SIZE != 0)
				{
					this->slot = tempnewslot
					return record;
				}
			}
		}
*/
