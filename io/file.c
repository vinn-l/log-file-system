/**
 * Name: Waltvin Lee
 * ID: V00894034
 * Modified Date: 11/4/2019
 * Filename: file.c
 * Details: CSC360 Assignment <3>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../disk/disk.h"

int currdirinode = 0; //set default inode directory as 0
int currentpath[20];
int pathdepth = 0;

void writeFile(char* fname, char* inputstring) {
	//write fname to directory inode
	//find current directory block
	//root directory is at block 26
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
    }
	
	//check if fname is taken
	char** filearray = getallfilenames(file, currdirinode);
	for(int i = 0; i < 192; i++){
		if(strcmp("", filearray[i]) == 0){
			//do nothing
		}
		else{
			if(strcmp(&filearray[i][1], fname) == 0){
				printf("Filename is taken!\n");
				for(int j = 0; j < 192; j++){
					free(filearray[j]);
				}
				free(filearray);
				fclose(file);
				return;
			}
		}
	}
	
	//Get a free inode
	int inodeid = findfreeinode(file);
	usefreeinode(file, inodeid);
	
	//Determine number of blocks required for size of file
	int numblocks = (strlen(inputstring)+1)/512;
	//Add 1 to numblock to round up the above equation
	if (((strlen(inputstring)+1)%512) != 0){
		numblocks++;
	}
	
	//Create inputstring2 as a array[] and memset 0 to set all bytes to 0 initially.
	char inputstring2[numblocks*512];
	memset(inputstring2, 0, numblocks*512);
	
	//Copy contents of inputstring to inputstring2
	memcpy(inputstring2, inputstring, strlen(inputstring)+1);
	
	//Find and use and write nblocks
	uint16_t* blockarr = findandusenblocks(file, numblocks);

	writeNblocks(file, blockarr, numblocks, inputstring2);
	
	//Write contents to Inode as well
	writeinode(file, inodeid, strlen(inputstring)+1, 0, blockarr, numblocks);
	
	//Update block of current directory to save filename
	addfiletoblock(file, currdirinode, inodeid, fname);
	
	printf("File Written!\n");
    // Close file
	free(blockarr);
	for(int j = 0; j < 192; j++){
		free(filearray[j]);
	}
	free(filearray);
    fclose(file);
	return;
}

void readFile(char* fname){
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
    }
	int fileinode = findinode(file, currdirinode, fname);
	if(fileinode == 0){
		printf("File not found!\n");
		return;
	}
	
	uint16_t* blockidarray = getblocksid(file, fileinode);
	
	char* buffer = (char*) calloc(512, sizeof(char));
	char* buffer2 = (char*) calloc(512, sizeof(char));
	//Read all block id array
	int i = 0;
	for(i = 0; i < 12; i++){
		//printf("%d\n", blockidarray[i]);
		if(i == 11){ //Reached single indirect block
			if(blockidarray[i] != 0){ //If single indirect block exists, read it
				uint16_t* indirectblockidarray = getindirectblocksid(file, blockidarray[i]);
				//printf("Reading indirectblock!\n");
				for(int j = 0; j < 256; j++){
					if(indirectblockidarray[j] != 0){
						buffer2 = (char*) realloc(buffer2, 512*(i+1 + j+1));
						readblock(file, indirectblockidarray[j], buffer);
						strncat(buffer2, buffer, 512);
					}
				}
				free(indirectblockidarray);
				//printf("Im done!\n"); 
			}
		}
		else if(blockidarray[i] != 0){
			buffer2 = (char*) realloc(buffer2, 512*(i+1));
			readblock(file, blockidarray[i], buffer);
			strncat(buffer2, buffer, 512);
		}
	}
	
	
	
	free(buffer);
	printf("Contents of %s : %s\n",fname ,buffer2);
	free(buffer2);
	free(blockidarray);
	fclose(file);
	return;
	//return buffer2;
}

void makeDir(char* newdirname){
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
        //return false;
    }
	
	//check if newdirname is taken
	char** filearray = getallfilenames(file, currdirinode);
	for(int i = 0; i < 192; i++){
		if(strcmp("", filearray[i]) == 0){
			//do nothing
		}
		else{
			if(strcmp(&filearray[i][1], newdirname) == 0){
				printf("Directory name is taken!\n");
				for(int j = 0; j < 192; j++){
					free(filearray[j]);
				}
				free(filearray);
				fclose(file);
				return;
			}
		}
	}
	
	//Get a free inode
	int newdirinode = findfreeinode(file);
	usefreeinode(file, newdirinode);
	
	//Determine number of blocks required for size of file, directory only need 1 block to startoff
	int numblocks = 1;
	
	//Create inputstring2 as a array[] and memset 0 to set all bytes to 0 initially, because directory is empty when made.
	char inputstring2[numblocks*512];
	memset(inputstring2, 0, numblocks*512);
	
	//Find and use and write nblocks
	uint16_t* blockarr = findandusenblocks(file, numblocks);
	
	writeNblocks(file, blockarr, numblocks, inputstring2);

	//Write contents to Inode as well
	writeinode(file, newdirinode, 0, 1, blockarr, numblocks); //flag set to 1 to indicate directory
	
	//Update block of current directory to save dirname
	uint16_t* CurrDirBlockId = getblocksid(file, currdirinode); //Array of block ids size 12.
	addfiletoblock(file, currdirinode, newdirinode, newdirname);
	printf("Directory Created!\n");

	free(blockarr);
	free(CurrDirBlockId);
    fclose(file);
	for(int j = 0; j < 192; j++){
		free(filearray[j]);
	}
	free(filearray);
	return;
}

void rmFile(char* filename){
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
        //return false;
    }
	int fileinode = findinode(file, currdirinode, filename);
	if(fileinode == 0){
		printf("File not found!\n");
		fclose(file);
		return;
	}
	uint16_t* FileBlockId = getblocksid(file, fileinode);
	
	//Delete all blocks, basically unset them in free block vector.
	for(int i = 0; i< 12; i++){
		if(i == 11){ // if indirect block (only for files) (if it was a directory, it would be 0 because of the assumption where directories can be deleted only when it is empty.
				if(FileBlockId[11] != 0){ // if exists
					uint16_t* indirectblocksid = getindirectblocksid(file, FileBlockId[11]);
					for(int k = 0; k < 256; k++){
						if(indirectblocksid[k] == 0){
							//do nothing
						}
						else{
							delfreeblock(file, indirectblocksid[k]);
						}
					}
					free(indirectblocksid);
				}
		}
			
		if(FileBlockId[i] == 0){
			//do nothing
		}
		else{
			delfreeblock(file, FileBlockId[i]);
		}
	}
	
	//Delete blocks if indirect exist as well
	
	
	//Delete the Inode for the file
	delfreeinode(file, fileinode);
	
	//Remove file entry in currdirinode
	uint16_t* CurrDirBlockId = getblocksid(file, currdirinode);
	
	//Set the first byte of the entry to 0
	delfilefromblock(file, currdirinode, fileinode);
	printf("File/Directory removed!\n");
	free(FileBlockId);
	free(CurrDirBlockId);
	fclose(file);
	return;
}

void openDir(char* dirname){ //start at root 
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
        //return false;
    }
	int dirinode = findinode(file, currdirinode, dirname);
	if(dirinode == 0){
		printf("Directory not found!\n");
		fclose(file);
		return;
	}
	currdirinode = dirinode;
	pathdepth++;
	currentpath[pathdepth] = currdirinode;
	fclose(file);
	printf("Currently in %s!\n", dirname);
	return;
	//return buffer2;
}

void exitDir(){
	pathdepth--;
	currdirinode = currentpath[pathdepth];
	printf("Exitted Directory!\n");
	return;
}

//Lists all files in current directory
void listallfiles(){ 
	int filesExist = 0;
	FILE * file = fopen("../disk/vdisk", "rb+");
    if (!file) {
        fprintf(stderr, "Unable to open disk\n");
    }
	char** filearray = getallfilenames(file, currdirinode);
	for(int i = 0; i < 192; i++){
		if(strcmp("", filearray[i]) == 0){
			//do nothing
		}
		else{
			filesExist = 1;
			printf("%02d: %s\n", i+1, &filearray[i][1]);
		}
	}
	if(filesExist == 0){
		printf("No files in Directory!\n");
	}
	for(int i = 0; i < 192; i++){
		free(filearray[i]);
	}
	free(filearray);
	fclose(file);
	return;
}

void file(char* command){
	
	char buffer[1024];
	char* token;
	char* token2;
	strcpy(buffer, command);
	
	if(strcmp(buffer, "help") == 0){
		printf("File v1 Usage:\n");
		printf("	Help: help\n");
		printf("	Initialize disk: InitDisk\n");
		printf("	List files: list\n");
		printf("	Open directory: openDir dirname\n");
		printf("	Exit directory: exitDir\n");
		printf("	Make directory: makeDir dirname\n");
		printf("	Write File: writeFile filename filetext\n");
		printf("	Read File: readFile filename\n");
		printf("	Remove File/Directory: rmFile filename/dirname\n");
		printf("	Exit Program: exit\n");
	}
	else if(strcmp(buffer, "list") == 0){
		printf("Listing Files\n");
		listallfiles();
	}
	else if(strcmp(buffer, "exit") == 0){
		printf("Exiting\n");
		return;
	}
	else if(strcmp(buffer, "InitDisk") == 0){
		printf("Initializing Disk\n");
		InitLLFS();
	}
	else if(strncmp(buffer, "makeDir ", 8) == 0){
		printf("Making Directory\n");
		token = strtok(buffer, " ");
		token = strtok(NULL, "");
		makeDir(token);
	}
	else if(strcmp(buffer, "exitDir") == 0){
		printf("Exit Directory\n");
		exitDir();
	}
	else if(strncmp(buffer, "openDir ", 8) == 0){
		printf("Opening Directory\n");
		token = strtok(buffer, " ");
		token = strtok(NULL, "");
		//printf("%s\n", token);
		openDir(token);
	}
	else if(strncmp(buffer, "writeFile ", 10) == 0){
		printf("Writing File\n");
		token = strtok(buffer, " ");
		token = strtok(NULL, " ");
		token2 = strtok(NULL, "");
		writeFile(token, token2);
		//printf("%s %s", token, token2);
	}
	else if(strncmp(buffer, "readFile ", 9) == 0){
		printf("Reading File\n");
		token = strtok(buffer," ");
		token = strtok(NULL,"");
		readFile(token);
	}
	else if(strncmp(buffer, "rmFile ", 7) == 0){
		printf("Removing File\n");
		token = strtok(buffer, " ");
		token = strtok(NULL, "");
		rmFile(token);
	}
	else{
		printf("Invalid command!\n");
	}
	printf("\n");
}

//Replace this with main() to have functionality by calling file.exe instead of going through app
int main(){
	printf("File v1 Usage:\n");
	printf("	Help: help\n");
	printf("	Initialize disk: InitDisk\n");
	printf("	List files: list\n");
	printf("	Open directory: openDir <dirname>\n");
	printf("	Exit directory: exitDir\n");
	printf("	Make directory: makeDir <dirname>\n");
	printf("	Write File: writeFile <filename> <filetext>\n");
	printf("	Read File: readFile <filename>\n");
	printf("	Remove File/Directory: rmFile <filename/dirname>\n");
	printf("	Exit Program: exit\n\n");
	char buffer[136704];
	char* token;
	char* token2;
	while(1){
		fgets(buffer, 136704, stdin);
		if(strcmp(buffer, "\n") == 0){
			//do nothing
		}
		else if(strcmp(buffer, "help\n") == 0){
			printf("File v1 Usage:\n");
			printf("	Help: help\n");
			printf("	Initialize disk: InitDisk\n");
			printf("	List files: list\n");
			printf("	Open directory: openDir <dirname>\n");
			printf("	Exit directory: exitDir\n");
			printf("	Make directory: makeDir <dirname>\n");
			printf("	Write File: writeFile <filename> <filetext>\n");
			printf("	Read File: readFile <filename>\n");
			printf("	Remove File/Directory: rmFile <filename/dirname>\n");
			printf("	Exit Program: exit\n");
		}
		else if(strcmp(buffer, "list\n") == 0){
			printf("Listing Files\n");
			listallfiles();
		}
		else if(strcmp(buffer, "exit\n") == 0){
			printf("Exiting\n");
			return 0;
		}
		else if(strcmp(buffer, "InitDisk\n") == 0){
			printf("Initializing Disk\n");
			InitLLFS();
		}
		else if(strncmp(buffer, "makeDir ", 8) == 0){
			printf("Making Directory\n");
			token = strtok(buffer, " ");
			token = strtok(NULL, "\n");
			makeDir(token);
		}
		else if(strcmp(buffer, "exitDir\n") == 0){
			printf("Exit Directory\n");
			exitDir();
		}
		else if(strncmp(buffer, "openDir ", 8) == 0){
			printf("Opening Directory\n");
			token = strtok(buffer, " ");
			token = strtok(NULL, "\n");
			//printf("%s\n", token);
			openDir(token);
		}
		else if(strncmp(buffer, "writeFile ", 10) == 0){
			printf("Writing File\n");
			token = strtok(buffer, " ");
			token = strtok(NULL, " ");
			token2 = strtok(NULL, "\n");
			writeFile(token, token2);
			//printf("%s %s", token, token2);
		}
		else if(strncmp(buffer, "readFile ", 9) == 0){
			printf("Reading File\n");
			token = strtok(buffer," ");
			token = strtok(NULL,"\n");
			readFile(token);
		}
		else if(strncmp(buffer, "rmFile ", 7) == 0){
			printf("Removing File\n");
			token = strtok(buffer, " ");
			token = strtok(NULL, "\n");
			rmFile(token);
		}
		else{
			printf("Invalid command!\n");
		}
		printf("\n");
	}
	
	//INTERNAL TESTING
/*
	InitLLFS();
	currentpath[pathdepth] = currdirinode;
	
	char* test = "This is file1";
	writeFile("file1", test);
	
	test = "This is file2";
	
	makeDir("NewDirectory!");
	makeDir("NewDirectory!1");
	
	makeDir("NewDirectory!2");
	makeDir("NewDirectory!3");
	makeDir("NewDirectory!4");
	makeDir("NewDirectory!5");
	makeDir("NewDirectory!6");
	makeDir("NewDirectory!7");
	makeDir("NewDirectory!8");
	writeFile("file2", test);
	makeDir("NewDirectory!9");
	makeDir("NewDirectory!10");
	makeDir("NewDirectory!11");
	makeDir("NewDirectory!12");
	makeDir("NewDirectory!13");
	
	makeDir("NewDirectory!14");
	
	makeDir("NewDirectory!15");
	makeDir("NewDirectory!16");
	makeDir("NewDirectory!17");
	makeDir("NewDirectory!18");
	makeDir("NewDirectory!19");
	makeDir("NewDirectory!20");

	FILE * file = fopen("../disk/vdisk", "rb+");
	char** rootfiles = getallfilenames(file, currdirinode);
	for(int i = 0; i< 192; i++){
		free(rootfiles[i]);
	}
	free(rootfiles);
*/
	/*
	printf("%s\n", &rootfiles[0][1]);
	printf("%s\n", &rootfiles[1][1]);
	printf("%s\n", &rootfiles[2][1]);
	printf("%s\n", &rootfiles[3][1]);
	*/
/*
	findinode(file, currdirinode, "NewDirectory!");
	//printf("Hello!1\n");
	readFile("file2");
	readFile("file1");
	listallfiles();
	openDir("NewDirectory!");
	//printf("%d\n", currdirinode);
	makeDir("newnewDirectory");
	test = "thisisfile3";
	writeFile("file3", test);
	//printf("Hello?");
	listallfiles();
	rmFile("file3");
	//readFile("file3");
	listallfiles();
/*
	/*listallfiles();
	exitDir();
	listallfiles();
	openDir("NewDirectory!");
	listallfiles();
	rmFile("file3");
	*/
}