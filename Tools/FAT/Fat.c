#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// FAT12 BootSector informations
// Note: __attribute__((packed)) is a gcc compiler instruction to tell it not to pad the structure with empty spaces
// (it does so to improve some operation performances, but in our case it would read too much bytes of our FAT image file)
typedef struct {
	uint8_t BootJumpInstruction[3];
	uint8_t OemIdentifier[8];
	uint16_t BytesPerSector;
	uint8_t SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t FatCount;
	uint16_t DirEntryCount;
	uint16_t TotalSectors;
	uint8_t MediaDescriptorType;
	uint16_t SectorsPerFat;
	uint16_t SectorsPerTrack;
	uint16_t Heads;
	uint32_t HiddenSectors;
	uint32_t LargeSectorCount;

	// extended boot record
	uint8_t DriveNumber;
	uint8_t _Reserved;
	uint8_t Signature;
	uint32_t VolumeId; // serial number, value doesn't matter
	uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
	uint8_t SystemId[8];

	// ...and we don't care about code

} __attribute__((packed)) BootSector;

// FAT12 directory entry
typedef struct {
	uint8_t Name[11];
	uint8_t Attributes;
	uint8_t _Reserved;
	uint8_t CreatedTimeTenths;
	uint16_t CreatedTime;
	uint16_t CreatedDate;
	uint16_t AccessedDate;
	uint16_t FirstClusterHigh;
	uint16_t ModifiedTime;
	uint16_t ModifiedDate;
	uint16_t FirstClusterLow;
	uint32_t Size;
} __attribute__((packed)) DirectoryEntry;

bool readBootSector(FILE* disk, BootSector* output){
	size_t sizeRead = fread(output, sizeof(BootSector), 1, disk);
	return (sizeRead > 0);
}

/** Reads "count" (n) sectors sectors from the disk (with the associated BootSector bs), starting from lba, into buffer_out */
bool readSectors(FILE* disk, BootSector bs, uint32_t lba, uint32_t count, void* buffer_out){
	
	// Set cursor position to where we have to read
	int fseek_res = fseek(disk, lba*bs.BytesPerSector, SEEK_SET);
	if (fseek_res != 0) return false;

	// Read
	size_t fread_res = fread(buffer_out, bs.BytesPerSector, count, disk);
	return (fread_res == count);
}

/** 
 * Reads the FAT from disk (with the associated BootSector bs) into fat_out
 * *fat_out will be reallocated, so its memory needs to be freed or saved elsewhere
*/
bool readFat(FILE* disk, BootSector bs, uint8_t** fat_out){
	int fatSize = bs.SectorsPerFat * bs.BytesPerSector;
	*fat_out = (uint8_t*) malloc(fatSize);
	bool couldRead = readSectors(disk, bs, bs.ReservedSectors, bs.SectorsPerFat, *fat_out); // bs.ReservedSectors: the FAT starts just after the reserved sectors
	return couldRead;
}

void printBootSectorInformations(BootSector bs){
	printf("Read bootsector informations:\n");
	printf("BootJumpInstruction\t %o %o %o \n", bs.BootJumpInstruction[0], bs.BootJumpInstruction[1], bs.BootJumpInstruction[2]);
	printf("OemIdentifier\t\t %s \n", 			bs.OemIdentifier);
	printf("BytesPerSector\t\t %u \n", 			bs.BytesPerSector);
	printf("SectorsPerCluster\t %u \n", 		bs.SectorsPerCluster);
	printf("ReservedSectors\t\t %u \n", 		bs.ReservedSectors);
	printf("FatCount\t\t %o \n", 				bs.FatCount);
	printf("DirEntryCount\t\t %u \n", 			bs.DirEntryCount);
	printf("TotalSectors\t\t %u \n", 			bs.TotalSectors);
	printf("MediaDescriptorType\t %u \n", 		bs.MediaDescriptorType);
	printf("SectorsPerFat\t\t %u \n", 			bs.SectorsPerFat);
	printf("SectorsPerTrack\t\t %u \n", 		bs.SectorsPerTrack);
	printf("Heads\t\t\t %u \n", 				bs.Heads);
	printf("HiddenSectors\t\t %u \n", 			bs.HiddenSectors);
	printf("LargeSectorCount\t %u \n", 			bs.LargeSectorCount);

	printf("DriveNumber\t\t %u \n", 			bs.DriveNumber);
	printf("_Reserved\t\t %u \n", 				bs._Reserved);
	printf("Signature\t\t %u \n", 				bs.Signature);
	printf("VolumeId\t\t %u \n", 				bs.VolumeId);
	printf("VolumeLabel\t\t %*.*s %s", 11, 11, 	bs.VolumeLabel, "\n"); // print only 11 char from string (which is not null-terminated)
	printf("SystemId\t\t %s \n", 				bs.SystemId);
	
	printf("\n");
}

/** 
 * Reads the disk's root directory (with the associated BootSector bs) into dir_out
 * *dir_out will be reallocated, so its memory needs to be freed or saved elsewhere
 * Note: It will be allocated to the number of directories on the disk !
*/
bool readRootDirectory(FILE* disk, BootSector bs, DirectoryEntry** dir_out){
	// Where we start reading
	uint32_t lba = bs.ReservedSectors + bs.FatCount*bs.SectorsPerFat;
	
	// Total size of the entries
	uint32_t size = sizeof(DirectoryEntry)*bs.DirEntryCount; // in bytes
	// Translate in sectors count for the read
	uint32_t sectors = size / bs.BytesPerSector;
	if (size % bs.BytesPerSector > 0) sectors++; // round up number of sectors

	// Allocate enough memory for all directories entries
	// Note: We do not allocate size bytes because we rounded up the number of sectors for the read
	// The read wouldn't read the last bytes but rather the entire sector, so it would be an overflow of our dir_out (seg fault)
	*dir_out = (DirectoryEntry*) malloc(sectors * bs.BytesPerSector);
	
	bool res = readSectors(disk, bs, lba, sectors, *dir_out);
	return res;
}

// Finds a file named "name" in a directory array *dir, on a disk descried by a BootSector bs
DirectoryEntry* findFile(BootSector bs, DirectoryEntry* dir, const char* name){
	for(uint16_t i = 0 ; i<bs.DirEntryCount ; i++){
		int compare_res = memcmp(name, dir[i].Name, 11);
		if (compare_res == 0){
			return &(dir[i]);
		}
	}
	return NULL;
}

// Reads a file from an entry fileEntry, into the bytes buffer ouput_file. Does so from a disk disk described by a BootSector bs and with the FAT fat
bool readFile(FILE* disk, BootSector bs, uint8_t *fat, DirectoryEntry* fileEntry, uint8_t* output_file){
	// Calculate where the root directory ends
	uint32_t lba = bs.ReservedSectors + bs.FatCount*bs.SectorsPerFat;
	uint32_t size = sizeof(DirectoryEntry)*bs.DirEntryCount; // in bytes
	uint32_t sectors = size / bs.BytesPerSector;
	if (size % bs.BytesPerSector > 0) sectors++; // round up number of sectors
	uint32_t rootDirectoryEnd = lba + sectors;

	bool ok = true;
	uint16_t currentCluster = fileEntry->FirstClusterLow;
	do {
		// currentCluster-2 : the first two clusters are reserved
		uint32_t lba = rootDirectoryEnd + (currentCluster-2)*bs.SectorsPerCluster;
		ok = readSectors(disk, bs, lba, bs.SectorsPerCluster, output_file);
		output_file += bs.SectorsPerCluster; // update pointer to point at where the data ends

		// To determine where the next cluster is, we lookup in the FAT
		uint32_t fatIndex = currentCluster + currentCluster/2; // indexes are 12 bits (1 byte and a half)
		if (currentCluster % 2 == 0) currentCluster = (*(uint16_t*) (fat + fatIndex)) & 0x0fff; // select only bottom 12 bits
		else currentCluster = (*(uint16_t*) (fat + fatIndex)) >> 4; // select only upper 12 bits

	} while (ok && currentCluster < 0xff8); // A "next cluster" value of 0xff8 or bigger means that we reached the end of the file's cluster chain

	return ok;
}

int main(int argc, const char** argv){
	if (argc < 3){
		printf("Syntax error. Usage: %s <disk image> <file name>\n", argv[0]);
		return -1;
	}

	// Open the disk
	FILE* disk = fopen(argv[1], "rb");
	if (!disk){
		fprintf(stderr, "Could not open disk image %s !\n", argv[1]);
		return -2;
	}

	// Read the disk's bootsector
	BootSector fat_bs;
	bool couldRead = readBootSector(disk, &fat_bs);
	if (!couldRead){
		fprintf(stderr, "Could not read '%s' boot sector !\n", argv[1]);
		return -3;
	}
	
	printBootSectorInformations(fat_bs);

	// Read the disk's FAT
	uint8_t* fat = NULL;
	couldRead = readFat(disk, fat_bs, &fat);
	if(!couldRead){
		fprintf(stderr, "Could not read FAT (File Allocation Table) !\n");
		free(fat);
		return -4;
	}

	printf("Successfully read FAT !\n");
	
	// Read the disk's root directory
	DirectoryEntry* directories = NULL;
	couldRead = readRootDirectory(disk, fat_bs, &directories);
	if(!couldRead){
		fprintf(stderr, "Could not read root directory !\n");
		free(fat);
		if (directories != NULL) free(directories);
		return -5;
	}

	printf("Successfully read root directory !\n");

	// Find the file in the directories entries
	const char* filename = argv[2];
	DirectoryEntry* file_location = findFile(fat_bs, directories, filename);
	if (file_location == NULL){
		fprintf(stderr, "The file '%s' does not exist on the disk ! \n", filename);
		fprintf(stderr, "Maybe the filename is wrong ? It should be FAT12 compatible, aka 11 char long, padded with spaces.\nFor example: 'TEST    TXT'\n");
		free(fat);
		free(directories);
		return -6;
	}

	printf("Successfully found '%s' on disk !\n", filename);

	// Read the file contents from disk
	uint8_t* file_buffer = (uint8_t*) malloc(file_location->Size + fat_bs.BytesPerSector);
	bool res = readFile(disk, fat_bs, fat, file_location, file_buffer);
	if (!res){
		fprintf(stderr, "An error occured during the reading of the file '%s' !\n", filename);
		free(fat);
		free(directories);
		free(file_buffer);
		return -7;
	}

	printf("Successfully read file from disk !\n");
	
	// Print the file content
	printf("Here is the file contents:\n");
	for(size_t i=0 ; i<file_location->Size ; i++){
		if (isprint(file_buffer[i])) fputc(file_buffer[i], stdout); // if the char is printable, print it
		else printf("<%02x>", file_buffer[i]); // else print its hex value
	}
	printf("\n");

	free(fat);
	free(directories);
	free(file_buffer);
	return 0;
}
