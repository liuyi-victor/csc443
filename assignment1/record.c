#include "library.h"
#include <string.h>

/*
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record)
{
	int i, size = 0;
	for(i = 0; i < record->size; i++)
	{
		size += 10//strlen(record[i]);
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
		for(i = 0; i < record->size; i++)
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
	for(i = 0; i < record->size && size > 0; i++)
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
