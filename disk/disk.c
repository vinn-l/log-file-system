/**
 * Name: Waltvin Lee
 * ID: V00894034
 * Modified Date: 11/4/2019
 * Filename: disk.c
 * Details: CSC360 Assignment <3>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "disk.h"

typedef unsigned char byte;
#define BIT (8*sizeof(byte))

static bool get  (byte,   byte);
static void set  (byte *, byte);
static void reset(byte *, byte);

/* CAREFUL WITH pos AND BITMAP SIZE! */

bool bitmapGet(byte *bitmap, int pos) {
/* gets the value of the bit at pos */
	pos = 7 - pos;
    return get(bitmap[pos/BIT], pos%BIT);
}

void bitmapSet(byte *bitmap, int pos) {
/* sets bit at pos to 1 */
	pos = 7 - pos;
    set(&bitmap[pos/BIT], pos%BIT);
}

void bitmapReset(byte *bitmap, int pos) {
/* sets bit at pos to 0 */
	pos = 7 - pos;
    reset(&bitmap[pos/BIT], pos%BIT);
}

int bitmapSearch(byte *bitmap, bool n, int start) {
/* Finds the first n value in bitmap after start */
/* size is the Bitmap size in bytes */
    int i;
    /* size is now the Bitmap size in bits */
    for(i = start; i < 8; i++)
        if(bitmapGet(bitmap,i) == n)
            return i;
    return -1;
}

static bool get(byte a, byte pos) {
/* pos is something from 0 to 7*/
    return (a >> pos) & 1;
}

static void set(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 1 */
    *a |= 1 << pos;
}

static void reset(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 0 */
    *a &= ~(1 << pos);
}

uint16_t* getindirectblocksid(FILE* file, int blockid){
	uint16_t* blockarr = (uint16_t*) calloc(256, sizeof(uint16_t));
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, blockid, buffer2);
	memcpy(blockarr, buffer2, 256*sizeof(uint16_t));
	free(buffer2);
	return blockarr;
}

// Write an inode when given a free inodeid, and its properties 
void writeinode(FILE*file, int inodeid, uint32_t size, uint32_t flags, uint16_t* blockarr, int numblocks){
	char* buffer2 = (char*) calloc(512, sizeof(char));
	char* buffer3 = (char*) calloc(512, sizeof(char));
	readblock(file, 10 + (inodeid/16), buffer2);
	
	//block 10 represent inode 0 to 15
	//block 11 represent inode 16 to 31
	//block 12 represent inode 32 to 63
	//...
	//block 25 represent inode 240 to 255
	
	//4 bytes size of file
	//4 bytes flags
	//2 bytes multiplied by 10, block numbers
	//2 bytes single indirect block
	//2 bytes double indirect block
	
	//write size of file
	memcpy(&buffer2[32*inodeid], &size, sizeof(uint32_t));
	int j = 0;
	uint16_t* newIndirectBlock;
	//write unique block id on on block numbers
	for(int i = 0; i < numblocks; i++){
		if (i == 11){
			
			//We have to create single indirect block at byte 12!
			newIndirectBlock = findandusenblocks(file, 1);
			memcpy(&buffer2[32*(inodeid%16) + (8+i*2)], &newIndirectBlock[0], sizeof(uint16_t));
			
			//We then add the blockid to the indirect block
			memcpy(&buffer3[0 + j*2], &blockarr[i], sizeof(uint16_t));
			j++;
			
			/*
			if(k == 11){ // all 11 bytes are full, we need to use the 12th byte to point to single indirect block
				if(oldblocks[k] == 0){ //single indirect block not initialized, thus we initialize it and write the newblock into the single indirect block
					uint16_t* newIndirectBlock = findandusenblocks(file, 1);
					oldblocks[11] = newIndirectBlock[0];
					writeindirectblock(file, oldblocks[11], newblock, 1)
				}
				else{ //single indirect block is initialized, we copy the contents of the single indirect block and add entry and write it back.
					uint16_t* indirectoldblocks = getindirectblocksid(file, oldblocks[11])
					for (int k = 0; k < 256; k++){
						if(indirectoldblocks[k] == 0){
							indirectoldblocks[k] = newblock[0];
						}
						break;
					}
					writeindirectblock(file, oldblocks[11], indirectoldblocks, 256);					
					free(indirectoldblocks);
					free(oldblocks);
					free(newblock);
					i = 0; //set i back to 0
					CurrDirBlockId = getindirectblocksid(file, oldblocks[11]);
				}
			}
			*/
		}
		else if(i >11){
			//We write into single indirect block
			memcpy(&buffer3[0+j*2], &blockarr[i], sizeof(uint16_t));
			j++;
		}
		else{
			memcpy(&buffer2[32*(inodeid%16) + (8+i*2)], &blockarr[i], sizeof(uint16_t)); //little endian
		}
	}
	writeblock(file, 10 + (inodeid/16), buffer2);
	if (j!= 0){ //If indirect block initalized
		writeblock(file, newIndirectBlock[0], buffer3);
	}
	free(buffer2);
}

// Assigns output as contents of the block in blockid
void readblock(FILE* file, int blockid, char* output){
	fseek(file, 512*blockid, SEEK_SET);
	fread(output, 1, sizeof(char)*512, file);
	//return true;
}

// Writes input into blockid
void writeblock(FILE* file, int blockid, char* input){
	fseek(file, 512*blockid, SEEK_SET);
	int ret = fwrite (input, 1, sizeof(char)*512, file);
	if (ret != sizeof(char)*512) {
        fprintf(stderr, "Write failed!\n");
        fclose(file);
        //return false;
    }
	//return true;
}

// Writes input into blockarr (multiple blocks)
void writeNblocks(FILE* file, uint16_t* blockarr, int numblocks, char* input){
	for(int i = 0; i < numblocks; i++){
		//if (i>12){
		//	printf("Error! can't fit into 12 blocks! single and double indirect block not implemented!\n");
		//}
		//else{
		writeblock(file, blockarr[i], &input[512*i]);
		//}
	}
}

// Sets the inodeid in the Inode Block Vector to 0 (as unavailable)
void usefreeinode(FILE* file, int inodeid){
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, 2, buffer2); //copy to buffer
	bitmapReset(&buffer2[inodeid/8], inodeid%8);
	writeblock(file, 2, buffer2);
	free(buffer2);
}

// Sets the inodeid in the Inode Block Vector to 1 (as available)
void delfreeinode(FILE* file, int inodeid){
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, 2, buffer2); //copy to buffer
	bitmapSet(&buffer2[inodeid/8], inodeid%8);
	writeblock(file, 2, buffer2);
	free(buffer2);
}

// Sets the blockid in the Free Block Vector to 0 (as unavailable)
void usefreeblock(FILE* file, int blockid){ //set 0 for unavailable
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, 1, buffer2); //copy to buffer
	bitmapReset(&buffer2[blockid/8], blockid%8);
	writeblock(file, 1, buffer2);
	//byte 0 represent block 0 to 7
	//byte 1 represent block 8 to 15
	//byte 2 represent block 16 to 23
	//...
	//byte 63 represent block 504 to 511
	free(buffer2);
}

// Sets the blockid in the Free Block Vector to 1 (as available)
void delfreeblock(FILE* file, int blockid){ //set 1 for available
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, 1, buffer2); //copy to buffer
	bitmapSet(&buffer2[blockid/8], blockid%8);
	writeblock(file, 1, buffer2);
	free(buffer2);
}

// Returns a free block id
uint16_t findfreeblock(FILE* file){
	char* buffer2 = (char*) calloc(512, sizeof(char)); 
	//find next freeblock in freeblock vector, block 1
	readblock(file, 1, buffer2);
	//find which earliest bit is 1
	int i = 0;
	while(buffer2[i] == 0x00){ // if bytes all unavailable, increment i till available.
		//printf("Byte unavailable at byte number: %d\n", i);
		i++;
	}
	int bitpos = bitmapSearch(&buffer2[i], 1, 0);
	//printf("byte number: %d, bit number: %d is 1(available!)\n", i, bitpos);
	//byte 0 represent block 0 to 7
	//byte 1 represent block 8 to 15
	uint16_t freeid = 8*i + bitpos;
	//printf("Block ID: %d is available!\n", freeid);
	free(buffer2);
	return freeid;
}

// Returns a free inode unique id
uint8_t findfreeinode(FILE* file){
	char* buffer3 = (char*) calloc(512, sizeof(char));
	readblock(file, 2, buffer3);
	int i = 0;
	while(buffer3[i] == 0x00){
		//printf("Byte unavailable at byte number: %d\n", i);
		i++;
	}
	int bitpos = bitmapSearch(&buffer3[i], 1, 0);
	//printf("byte number: %d, bit number: %d is 1(available!)\n", i, bitpos);
	//byte 0 represent inode number 0 to 7
	//byte 1 represent inode  number 8 to 15
	
	uint8_t freeid = 8*i + bitpos;
	//printf("Inode ID: %d is available!\n", freeid);
	free(buffer3);
	return freeid;
}

// Finds numblocks amount of free blocks, set them to unavailable in the Free Block Vector, and returns an array of the blockids of the free blocks.
uint16_t* findandusenblocks(FILE* file, int numblocks){ //return block number in array
	uint16_t* blockarr = (uint16_t*) calloc(numblocks, sizeof(uint16_t));
	int i;
	for(i = 0; i < numblocks; i++){
		blockarr[i] = findfreeblock(file);
		usefreeblock(file, blockarr[i]);
	}
	return blockarr;
}

// Returns an array of blockids of the inode id (maximum 12 blocks)
uint16_t* getblocksid(FILE* file, int inodeid){
	uint16_t* blockarr = (uint16_t*) calloc(12, sizeof(uint16_t));
	char* buffer2 = (char*) calloc(512, sizeof(char));
	readblock(file, 10 + (inodeid/16), buffer2);
	memcpy(blockarr, &buffer2[32*(inodeid%16) + 8], 12*sizeof(uint16_t));
	
	//block 10 represent inode 0 to 15
	//block 11 represent inode 16 to 31
	//block 12 represent inode 32 to 63
	//...
	//block 25 represent inode 240 to 255
	free(buffer2);
	return blockarr;
}

// Returns an array of file names in directory with the inode id of currdirinode
char** getallfilenames(FILE* file, int currdirinode){
	char** namearray = (char**) calloc(192, sizeof(char*)); //192 maximum files, 12 blocks *16 entries
	
	for(int i = 0; i< 192; i++){
		namearray[i] = calloc(32, sizeof(char));
	}
	uint16_t* CurrDirBlockId = getblocksid(file, currdirinode);
	char* buffer2 = (char*) calloc(512, sizeof(char));
	int i = 0;
	int arraycounter = 0;
	
	for(int i = 0; i < 12; i++){
		if (CurrDirBlockId[i] != 0){
			readblock(file, CurrDirBlockId[i], buffer2);
			for(int j = 0; j*32 < 512; j++){
				if(buffer2[j*32] != 0x00){ //if inode number is not 0, copy name into namearray
					memcpy(namearray[arraycounter], &buffer2[j*32], 32);	
					arraycounter++;
				}
			}
		}
	}
	free(buffer2);
	return namearray;
}

//Return inode number for given filename, dirinode is directory where filename is supposed to be in
int findinode(FILE* file, int dirinode, char* filename){
	char** filenames = getallfilenames(file, dirinode);
	char inode;
	int i = 0;
	for(i = 0; i < 192; i++){
		//printf("%s ",&filenames[i][1]);
		//printf("%s\n",filename);
		if(strcmp(&filenames[i][1], filename) == 0){
			memcpy(&inode, &filenames[i][0], 1);
			//printf("%x\n",inode);
			break;
		}
	}
	if(i == 192){ //Filename not Found
		for(int i = 0; i< 192; i++){
			free(filenames[i]);
		}
		free(filenames);
		return 0;
	}
	int inodenum = inode & 0xFFFF;
	//printf("%x\n", filenames[1][0]);
	//printf("%d\n", inodenum);
	for(int i = 0; i< 192; i++){
		free(filenames[i]);
	}
	free(filenames);
	return inodenum;
}

//Delete entry from directory block when given inode that is to be deleted
void delfilefromblock(FILE* file, int currdirinode, int delInode){
	uint16_t* CurrDirBlockId = getblocksid(file, currdirinode);
	int i = 0;
	int j = 0;
	char* buffer2 = (char*) calloc(512, sizeof(char));
	for(i = 0; i < 12; i++){ //Read blocks into buffer2 and find delInode
		if (CurrDirBlockId[i] != 0){ //if block id is not 0, means block exist
			readblock(file, CurrDirBlockId[i], buffer2);
			j = 0;
			while(buffer2[j*32] != delInode){ //Search for the delInode
				j++;
				if(j*32 == 512){
					break;
				}
			}
			if(j*32 == 512){//block full, proceed to next block
				//do nothing.
			}
			else{ //found it!
				memset(&buffer2[j*32], 0, 32); //set all to 0 to clear the entry
				writeblock(file, CurrDirBlockId[i], buffer2);

				free(CurrDirBlockId);
				free(buffer2);
				//printf("Cleared Entry number %d\n", j);
				return;
			}
		}
		else{ //block does not exist
			//do nothing, go to next block
		}
	}
	//printf("File not found to be deleted!\n");
	free(CurrDirBlockId);
	free(buffer2);
	return;
}

// Add entry to directory block when given inode and name for the new entry
void addfiletoblock(FILE* file, int currdirinode, int newinode, char* newname){
	uint16_t* CurrDirBlockId = getblocksid(file, currdirinode);
	int i = 0;
	int j = 0;
	char* buffer2 = (char*) calloc(512, sizeof(char));
	while(i < 12){ //12 blocks
		if (CurrDirBlockId[i] != 0){ // if blockid is not 0, means block exist
			readblock(file, CurrDirBlockId[i], buffer2); //read block content into buffer2
	
			//Each directory block contains 16 entries.
			//Each entry is 32 bytes long
			
			//Find an entry where first byte is 0 (no entry)
			//Each first byte of entries are at position 0, 32, 64, 96, ..., 480 
			//Search for a 0 byte in these positions
			j = 0;
			//printf("%d\n", CurrDirBlockId[0]);
			while(buffer2[j*32] != 0x00){ //Search for available entry
				//printf("Entry unavailable at entry number: %d\n", j);
				j++;
				if (j*32 == 512){
					printf("Whole block is full!\n");
					break;
				}
				//printf("%d\n", j*32);
			}
			if (j*32 == 512){ //block full, proceed to next block
				i++;
				//do nothing and proceed to next block.
			}
			else{ //found it!
				//printf("Entry available at entry number: %d\n", j);
				//Occupy the entry and put in newname
				memcpy(&buffer2[j*32], &newinode, sizeof(uint8_t));
				
				char* buffer3 = (char*) calloc(31, sizeof(char));
				memset(buffer3, 0, 31);
				if(strlen(newname) <= 30){
					memcpy(buffer3, newname, strlen(newname));
				}
				else{
					memcpy(buffer3, newname, 30);
				}
				
				memcpy(&buffer2[j*32 + 1], buffer3, 31*sizeof(byte));
				writeblock(file, CurrDirBlockId[i], buffer2);
				free(buffer2);
				return;
			}
		}
		else{ //block does not exist, initalize new block
			uint16_t* oldblocks = getblocksid(file, currdirinode);
			uint16_t* newblock = findandusenblocks(file, 1);
			
			//update newblock in oldblocks
			for (int k = 0; k < 12; k++){
				if(oldblocks[k] == 0){
					oldblocks[k] = newblock[0];
					break;
				}
			}
			
			//Update Inode for current directory
			writeinode(file, currdirinode, 0, 1, oldblocks, 12);
			free(CurrDirBlockId);
			CurrDirBlockId = getblocksid(file, currdirinode);
			free(oldblocks);
			free(newblock);
			//printf("%d\n",i);
			//printf("%d\n", CurrDirBlockId[1]);
		}
	}
	printf("More than 12 blocks, unable to create more!\n");
	free(CurrDirBlockId);
	free(buffer2);
	return;
}

// Clear and Initialize Disk
void InitLLFS(){
	FILE * file = fopen("../disk/vdisk", "wb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
    }
    char* buffer = (char*) calloc(512, sizeof(char)); //initalize everything to 0.
	memset(buffer, 0, sizeof(char)*512);
	
	for(int i = 0; i< 4095; i++){
		writeblock(file, i, buffer);
	}
	
		//SUPERBLOCK 512 bytes
	uint16_t intbuffer;
    memset(buffer, 0, sizeof(char)*512);
	intbuffer = 0;
	memcpy(&buffer[0], &intbuffer, sizeof(uint16_t)); //2 byte, root inode unique inode number is 0, 0-255
	intbuffer = 4096;
	memcpy(&buffer[2], &intbuffer, sizeof(uint16_t)); //2 bytes, 0-4096, 4096 blocks
	intbuffer = 256;
	memcpy(&buffer[4], &intbuffer, sizeof(uint16_t)); //2 bytes, 0-256
	writeblock(file, 0, buffer);

		//FREEBLOCK VECTOR block 0 to 26 is unavailable, 10-25 is inode, 26 is root directory, so set 0th bit to 26th bit to 0
    memset(buffer, 0b11111111, sizeof(char)*512);
	memset(buffer, 0b00000000, sizeof(char)*3); //0bit to 23bit
	memset(&buffer[3], 0b00111111, sizeof(char)*1);//24bit to 26bit
    writeblock(file, 1, buffer);
	
		//INODEMAPPING, 0-255 bits
	memset(buffer, 0b00000000, sizeof(char)*512); //clear all
	memset(buffer, 0b01111111, sizeof(char)*1); //set all available except inode number 0, taken by root by default.
	memset(&buffer[1], 0b11111111, sizeof(char)*31);
	writeblock(file, 2, buffer);
		//initialize ROOT directory block in block 26
	
	uint16_t* rootdirblock = findandusenblocks(file, 1);
	
	//since root directory starts with nothing in it, initialize all bits to 0
	memset(buffer, 0b00000000, sizeof(char)*512); //clear
	writeblock(file, rootdirblock[0], buffer);
	
		//ROOT Directory inode in block 10 (first 32 byte)
	//memset(buffer, 0b00000000, sizeof(char)*512); //clear
	//4 bytes size of file
	//4 bytes flags
	//2 bytes multiplied by 10, block numbers
	//2 bytes single indirect block
	//2 bytes double indirect block
	
	//write inode for root directory, at inode 0
	writeinode(file, 0, 0, 1, rootdirblock, 1); //root inode id is 0, size 0, flag 1 for directory, 1 numblock
	
	free(buffer);
	fclose(file);
}

//int main(){
	//Testing
	
	/*
	unsigned char a;
	byte b = 0b00000000;
	byte* d = &b;
	//printf("%u", *d);
	bitmapSet(d, 1);
	int c = bitmapGet(d,0);
	printf("%d\n", c);
	bitmapReset(d,1);
	printf("%u\n", *d);
	int e = bitmapSearch(d, 1, 0);
	printf("%d", e);
	*/
	
	/*
	unsigned int c = 0b00001111;
	a = inttochar(b);
	printf("%02X, %02X, %02X", a, b, c);
	*/
	
//	printf("Initdisk\n");
//	InitLLFS();
	
//}

