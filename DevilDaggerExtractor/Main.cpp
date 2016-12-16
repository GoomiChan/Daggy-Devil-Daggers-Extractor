#include <iostream>
#include <direct.h>
#include "DDArchive.h"

using namespace std;

int main(int argc, char* argv[])
{
	printf("=========================================================\n");
	printf("- Devil Daggers Extractor (Goomii)                      -\n");
	printf("=========================================================\n");

	if (argc != 3)
	{
		printf("\nUsage: \n"
			"\t DevilDaggersExtractor.exe [Archive] [Outputfolder]"
			);
	}
	else
	{
		// Try make the out folder
		int res = _mkdir(argv[2]);
		if (res == -1 && errno != EEXIST)
		{
			printf("Error creating folder %s\n :<", argv[2]);
		}
		else
		{
			const auto archive = DDArchive::LoadArchive(argv[1]);
			printf("%d files found in archive.\n", archive->NumFileEntrys);
			DDArchive::ExtractAllFiles(*archive, argv[2]);
			DDArchive::CloseArchive(*archive);
		}
	}

	return 0;
}