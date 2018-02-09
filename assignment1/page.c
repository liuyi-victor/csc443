#include <library.h>
#include <string.h>

void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
	if(page == NULL)
	{
		page = (Page *)malloc(sizeof(Page));
	}
	page->data = malloc(sizeof(page_size));
	page->page_size = page_size;
	page->slot_size = slot_size;
	page->used_slots = 0;
	int capacity = fixed_len_page_capacity(page);
	memset(page->data, capacity, sizeof(int));
	memset(((unsigned char *)page->data + 1), '\0', sizeof(unsigned char)*capacity);
}

int fixed_len_page_capacity(Page *page)
{
	return BYTE_SIZE*(page->page_size - sizeof(int)) / (page->slot_size + 1);
}

int fixed_len_page_freeslots(Page *page)
{
	return fixed_len_page_capacity(page) - page->used_slots;
}

int add_fixed_len_page(Page *page, Record *r)
{
	if(page == NULL || fixed_len_page_freeslots(page) == 0)
		return -1;
	// get the metadata of the page
	int capacity = fixed_len_page_capacity(page);
	(unsigned char *)bitmap = (unsigned char *)malloc(capacity);
	memcpy(bitmap, ((unsigned char *)page->data + sizeof(int)), capacity);
	int i;
	for(i = 0; i < capacity; i++)
	{
		if(bitmap[i] == '\0')
		{
			bitmap[i] = 0b1;
			break;
		}
	}
	int tuple_size = fixed_len_sizeof(r);
	unsigned char *tuplelist = (unsigned char *)page->data + sizeof(int) + capacity + i * tuple_size;
	
}
