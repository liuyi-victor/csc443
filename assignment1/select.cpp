#include "library.h"
#include <string.h>

void print_record(Record* record)
{
	//Iterate all records and print the 10 bytes as characters.
	for(int i = 0; i < record->size() -1; i++)
	{
		printf("%s, ", record->at(i));
	}
	//Print the last variable with no trailing comma.
	printf("%s\n", record->at(record->size()-1));
}
void free_record_memory(Record *tuple)
{
	int arrity = tuple->size();
	for(int i=0; i < arrity; i++)
		free((char *)tuple->at(i));
	tuple->clear();
}
int main (int argc, char *argv[])
{
	if(argc != 6)
	{
		fprintf(stderr, "Usage: %s <heapfile> <attribute_id> <start> <end> <page_size>\n", argv[0]);
		return -1;
	}
	int pagesize = atoi(argv[5]);
	int attribute = atoi(argv[2]);
	//char *field = (char *)malloc(lengthAttribute);
	char* start = argv[3];
	char* end = argv[4];
	
	Heapfile *file = (Heapfile *)malloc(sizeof(Heapfile));
	FILE *stream = fopen(argv[1], "r+");
	init_heapfile(file, pagesize, stream);
	
	RecordIterator* iter = new RecordIterator(file);
	int count = 0;
	while (iter->hasNext())
	{
		Record next_record = iter->next();
		if(strncmp(next_record.at(attribute), start, 5) >= 0 && strncmp(next_record.at(attribute), end, 5) <= 0)
		{
			printf("Record %d:\n", count);
			print_record(&next_record);
		}
		free_record_memory(&next_record);
		count++;
	} 
	fclose(stream);
	free(file);
}
