#pragma once
#include <string>
#include <vector>
#include <assert.h>

#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS
#define DEBUG false
#define DEBUG_FILETYPES false

#define LOG_PRINTF(...) if (DEBUG) printf(__VA_ARGS__)

#if DEBUG_FILETYPES
std::vector<unsigned int> FileTypes;
#endif

namespace DDArchive
{
	const unsigned char MAGIC[7] = { ':', 'h', 'x', ':', 'r', 'g', ':' };
	const char VERSION = 1;

	struct FileEntry_s
	{
		unsigned short FileType;
		std::string Name;
		unsigned int Offset;
		unsigned int Length;
		unsigned int Unknown;
	};

	struct FileArchiveHeader_s
	{
		char Magic[7];
		char Version;
		unsigned int DataOffset;
	};

	struct FileArchive_s
	{
		FILE *FileHandle;
		FileArchiveHeader_s Header;
		std::vector<FileEntry_s> FileEntrys;
		int NumFileEntrys;
	};

	std::string ReadNullTermString(FILE *file)
	{
		std::string str;
		while (true)
		{
			int currChar = fgetc(file);

			if (currChar != 0)
			{
				str += currChar;
			}
			else
			{
				break;
			}
		}

		return str;
	}

	FileArchive_s* LoadArchive(std::string filePath)
	{
		FILE *file = fopen(filePath.c_str(), "rb");
		if (!file)
		{
			printf("Couldn't open file: %s\n", filePath.c_str());
			return nullptr;
		}

		FileArchiveHeader_s Header;
		fread(&Header, sizeof(FileArchiveHeader_s), 1, file);

		if (memcmp(&Header.Magic, &MAGIC, sizeof(MAGIC)) == 0)
		{
			if (Header.Version == VERSION)
			{
				LOG_PRINTF("Magic: %s Version: %i, %u\n", Header.Magic, Header.Version, Header.DataOffset);

				// Read all the file entrys
				FileArchive_s *Archive = new FileArchive_s();
				Archive->FileHandle = file;
				Archive->Header = Header;
				while (ftell(file) < Header.DataOffset)
				{
					FileEntry_s FileEntry;
					fread(&FileEntry.FileType, sizeof(FileEntry.FileType), 1, file);
					FileEntry.Name = ReadNullTermString(file);
					fread(&FileEntry.Offset, sizeof(FileEntry.Offset), 1, file);
					fread(&FileEntry.Length, sizeof(FileEntry.Length), 1, file);
					fread(&FileEntry.Unknown, sizeof(FileEntry.Unknown), 1, file);
					Archive->FileEntrys.push_back(FileEntry);
					Archive->NumFileEntrys++;

					if (FileEntry.FileType == 17)
					{
						assert(FileEntry.Length == 0 && "File type 17 and length is not 0! :/");
					}
					else if (FileEntry.FileType == 16)
					{
						assert(FileEntry.Unknown == 0 && "File type 16 and uknown is not 0! :/");
					}

#if DEBUG_FILETYPES
					if (std::find(FileTypes.begin(), FileTypes.end(), FileEntry.FileType) == FileTypes.end())
					{
						FileTypes.push_back(FileEntry.FileType);
					}
#endif

					LOG_PRINTF("Name: %s, Flags: %u, Offset: %u, Length: %u\n", FileEntry.Name.c_str(), FileEntry.FileType, FileEntry.Offset, FileEntry.Length);
				}

#if DEBUG_FILETYPES
				for (auto &fileType : FileTypes)
				{
					printf("FileType: %u\n", fileType);
				}
#endif

				return Archive;
			}
			else
			{
				printf("Error: Unsuported version ;^; I can only handle version %i, this archive is version %i\n", VERSION, Header.Version);
				return nullptr;
			}
		}
		else
		{
			printf("Error: This does not appear to be a Devil Daggers archive? :<\n");
			return nullptr;
		}

		return nullptr;
	}

	void CloseArchive(const FileArchive_s &archive)
	{
		fclose(archive.FileHandle);
	}

	std::string FileTypeToExt(unsigned short FileType)
	{
		switch (FileType)
		{
			case 32: // Audio
				return ".wav";
			default:
				return "." + std::to_string(FileType);
		}
	}

	void ExtractFile(const FileArchive_s &archive, const FileEntry_s &entry, const std::string outFolder)
	{
		FILE *outFile;
		std::string filePath = outFolder + "//" + entry.Name + FileTypeToExt(entry.FileType);
		outFile = fopen(filePath.c_str(), "wb");

		if (!outFile)
		{
			printf("Couldn't create file: %s\n", filePath.c_str());
		}
		else
		{
			printf("Extracting file: %s (%u)\n", entry.Name.c_str(), entry.FileType);

			unsigned char *buffer = (unsigned char*)malloc(entry.Length);
			fseek(archive.FileHandle, entry.Offset, SEEK_SET);
			fread(buffer, sizeof(*buffer), entry.Length, archive.FileHandle);
			fwrite(buffer, sizeof(*buffer), entry.Length, outFile);
			free(buffer);
			fclose(outFile);
		}
	}

	void ExtractAllFiles(const FileArchive_s &archive, const std::string outFolder)
	{
		for (auto &entry : archive.FileEntrys)
		{
			ExtractFile(archive, entry, outFolder);
		}
	}
}
