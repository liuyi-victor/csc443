#include "library.h"

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
int main(int argc, char** argv)
{
	if(argc != 3){
		fprintf(stderr, "Usage: %s <heap_file> <page_size>\n", argv[0]);
		return 1;
	}

	Heapfile* heap = (Heapfile*)malloc(sizeof(Heapfile));
	int pagesize = atoi(argv[2]);
	FILE *stream = fopen(argv[1], "r+b");
	if(stream == NULL)
	{
		fprintf(stderr,"Failed to open page file %s\n", argv[2]);
		//perror("Failed to open page file %s: ", argv[2]);
		return 2;
	}
	heap->file_ptr = stream;
	heap->page_size = pagesize;

	RecordIterator* recordi = new RecordIterator(heap);
	int count = 0;
	while (recordi->hasNext())
	{
		Record next_record = recordi->next();
		printf("Record %d:\n", count);
		print_record(&next_record);
		free_record_memory(&next_record);
		count++;
	} 

	fclose(heap->file_ptr);
	free(heap);
	free(recordi);

	return 0;
}
/*
//g++ -g scan.cpp library.cpp -o scan
int main(int argc, char** argv) 
{
	if (argc < 3) 
	{
		printf("Usage: ./scan <heapfile> <page_size>\n");
		return 1;
	}
	
	Heapfile *file = (Heapfile*)malloc(sizeof(Heapfile));
	int pagesize = atoi(argv[2]);
	FILE *stream = fopen(argv[1], "r+b");
	if(stream == NULL)
	{
		fprintf(stderr,"Failed to open page file %s\n", argv[2]);
		//perror("Failed to open page file %s: ", argv[2]);
		return 2;
	}
	file->file_ptr = stream;
	file->page_size = pagesize;
	//if (open_heapfile(heap, argv[1], atoi(argv[2]), record_size) != 0) 
	//{
	//	return 2;
	//}
	//Record *next_record;
	RecordIterator* iter = new RecordIterator(file);
	do
	{
		Record next_record = iter->next();
		print_record(&next_record);
	}
	while (iter->hasNext());

	fclose(file->file_ptr);
	free(file);
	free(iter);

	return 0;
}*/
