#include "library.h"

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
	//if (open_heapfile(heap, argv[1], atoi(argv[2]), record_size) != 0) 
	//{
	//	return 2;
	//}

	RecordIterator* iter = new RecordIterator(file);
	while (iter->hasNext()) 
	{
		Record next_record = iter->next();
		iter->printRecords(&next_record);
	}

	fclose(file->file_ptr);
	free(file);
	free(iter);

	return 0;
}
