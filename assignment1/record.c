#include "record.h"
#include <string.h>

int fixed_len_sizeof(Record *record)
{
	int i, size = 0;
	for(i = 0; i < record->size; i++)
	{
		size += strlen(record[i]);
	}
	return size;
}
