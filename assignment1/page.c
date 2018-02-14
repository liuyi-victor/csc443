#include "library.h"
#include <string.h>
#include <math.h>

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
	unsigned char *buffer = malloc();
	for(j = 0; j < numAttribute; j++)
	{
		strncpy(buffer, tuple, lengthAttribute);
		r->pushback(buffer);
		tuple = tuple + lengthAttribute;
	}
}
