/**
 * Name: Waltvin Lee
 * ID: V00894034
 * Modified Date: 11/4/2019
 * Filename: disk.h
 * Details: CSC360 Assignment <3>
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef __disk_h__
#define __disk_h__

void writeinode(FILE*file, int inodeid, uint32_t size, uint32_t flags, uint16_t* blockarr, int numblocks);

void readblock(FILE* file, int blockid, char* output);
void writeblock(FILE* file, int blockid, char* input);

void writeNblocks(FILE* file, uint16_t* blockarr, int numblocks, char* input);

uint8_t findfreeinode(FILE* file);
void usefreeinode(FILE* file, int inodeid);

void delfreeinode(FILE* file, int inodeid);

uint16_t findfreeblock(FILE* file);
void usefreeblock(FILE* file, int blockid);
uint16_t* findandusenblocks(FILE* file, int numblocks);

void delfreeblock(FILE* file, int blockid);
void InitLLFS();

uint16_t* getblocksid(FILE* file, int inodeid);

void addfiletoblock(FILE* file, int currdirinode, int newinode, char* newname);

char** getallfilenames(FILE* file, int currdirinode);

int findinode(FILE* file, int dirinode, char* filename);

void delfilefromblock(FILE* file, int currdirinode, int delInode);

uint16_t* getindirectblocksid(FILE* file, int blockid);

#endif
