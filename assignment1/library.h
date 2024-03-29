#ifndef LIBRARY_H
#define LIBRARY_H

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BYTE_SIZE 8
#define numAttribute 100
#define lengthAttribute 10

typedef const char* V;
typedef std::vector<V> Record;
typedef int PageID;

typedef struct {
	void *data;
	int page_size;
	int slot_size;
	//int bitmapLength;
	//int used_slots;
} Page;

typedef struct {
	long offset;
	int freeslots;
} PageEntry;

typedef struct {
	FILE *file_ptr;
	int page_size;
} Heapfile;

typedef struct {
	int page_id;
	int slot;
} RecordID;

class RecordIterator 
{
	Heapfile *file;
	//FILE *stream;
	int pid;
	int currdir;
	int slot;
	int index;
	bool end;
	int updatenext(Page *page);
	int searchPageNext(Page *page, int start, int capacity);
    public:
        RecordIterator(Heapfile *heapfile);
        Record next();
        bool hasNext();
};
// implement serialization of fixed length records.


// Compute the number of bytes required to serialize record
int fixed_len_sizeof(Record *record);


// Serialize the record to a byte array to be stored in buf

void fixed_len_write(Record *record, void *buf);

/*
 * Deserialize 'size' bytes from the 'buf' buffer, and
 * stores the record in 'record'
 */
void fixed_len_read(void *buf, int size, Record *record);

// page operations
// Initializes a page using the given slot size
void init_fixed_len_page(Page *page, int page_size, int slot_size);

// Calculates the maximum number of records that fit in a page
int fixed_len_page_capacity(Page *page);

// Calculate the free space (number of free slots) in the page
int fixed_len_page_freeslots(Page *page);

/**
* Add a record to the page
* Returns:
*   record slot offset if successful,
*   -1 if unsuccessful (page full)
*/
int add_fixed_len_page(Page *page, Record *r);

// Write a record into a given slot.
void write_fixed_len_page(Page *page, int slot, Record *r);

// Read a record from the page from a given slot.
void read_fixed_len_page(Page *page, int slot, Record *r);

/* Heapfile functions*/
// Initialize a heapfile to use the file and page size given.
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

// Allocate another page in the heapfile. This grows the file by a page.
PageID alloc_page(Heapfile *heapfile);

// Read a page into memory.
int read_page(Heapfile *heapfile, PageID pid, Page *page);

// Write a page from memory to disk.
void write_page(Page *page, Heapfile *heapfile, PageID pid);

// fetch a page entry from the directory of the heapfile
int readHeapfileDirectory(Heapfile *heapfile, PageID pid, PageEntry *entry);

// write a page entry to the directory of the heapfile
void writeHeapfileDirectory(Heapfile *heapfile, PageID pid, PageEntry *entry);
#endif

