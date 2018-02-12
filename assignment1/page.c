#include <library.h>
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
	int bitmapLength = ceil(capacity/BYTE_SIZE);	// round up (capacity / BYTE_SIZSE)
	memset(page->data, capacity, sizeof(int));
	memset(((unsigned char *)page->data + 1), '\0', bitmapLength);	//page->bitmapLength);
}

int fixed_len_page_capacity(Page *page)
{
	//int directoryLength = 1 + (capacity - 1)/8;
	return BYTE_SIZE*(page->page_size - sizeof(int)) / (page->slot_size + 1);
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
	//int capacity = fixed_len_page_capacity(page);
	(unsigned char *)bitmap = (unsigned char *)malloc(page->bitmapLength);
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), page->bitmapLength);
	// find a free slot marked by a bit 0 in the corresponding directory
	int i, j, slot;
	for(i = 0; i < page->bitmapLength; i++)
	{
		if(bitmap[i] != 0xff)
		{
			unsigned char value = bitmap[i]; 
			for(j = 0; j < BYTE_SIZE && value & 0b1 != 0; j++)
				value = value >> 1;
			bitmap[i] ^= 0b1 << j;		//toggle the bit
			slot = BYTE_SIZE * i + j;
			break;
		}
	}
	int tuple_size = fixed_len_sizeof(r);
	unsigned char *tuple = (unsigned char *)page->data + sizeof(int) + page->bitmapLength + slot * tuple_size;
	for(j = 0; j < r->size(); j++)
	{
		strncpy()
	}
}
