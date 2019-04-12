/**
 * Name: Waltvin Lee
 * ID: V00894034
 * Modified Date: 11/4/2019
 * Filename: Readme.txt
 * Details: CSC360 Assignment <3>
 */

Introduction:
	This simple file system can be run by first running the makefile in ./app/ directory or ./ directory, and then RUNNING THE EXECUTABLE "./io/file"
	The executable "./io/file" takes in commands from stdin.
	./app/ contains 2 text files called app1.txt and app2.txt, these text file serves as a test input file that is to be feeded into stdin.
	app1.txt tests for simple commands, and app2.txt tests for a 10000 character input to show that single indirect block works.
	The BASH script to run everything is "./runtest"
		- it runs the 2 test files	

Below shows 2 methods to test this Simple File System, one by feeding the included test input file "./app/app1.txt" or "./app/app2.txt" into stdin for "./io/file".
The other by just running the executable "./io/file" and inputting commands through the GUI as stdin.

Running the included test input file, app1.txt
	1. Run make in ./app/ directory or ./ directory to compile
	2. Navigate to ./io/ and run file, and feed ../app/app1.txt into stdin. 
		"./file < ../app/app1.txt"
	3. Currently, app1.txt consists of the following simple commands and file will take them as stdin and execute them top to bottom:
			help
			InitDisk
			writeFile file1 This is file1.
			writeFile file2 This is file2.
			writeFile file3 This is file3.
			writeFile file4 This is file4.
			writeFile file4 This is file4 but it should be taken!
			list
			makeDir dir1
			makeDir dir2
			openDir dir1
			writeFile file1indir1 This file is in dir1.
			exitDir
			readFile file1
			rmFile file1
			list
			openDir dir1
			list
			readFile file1indir1
			rmFile file1indir1
			list
			exitDir
			rmFile dir1
			list
			exit
	
	4. You can change or add commands in app1.txt by adding/editing/removing the commands.
		
		List of Commands are available here:
			Help: help
			Initialize disk: InitDisk
			List files: list
			Open directory: openDir <dirname>
			Exit directory: exitDir
			Make directory: makeDir <dirname>
			Write File: writeFile <filename> <filetext>
			Read File: readFile <filename>
			Remove File/Directory: rmFile <filename/dirname>
			Exit Program: exit
	
	5. ../app/app2.txt can be tested in a similar way
	6. app2.txt writes a file with 10000 characters to prove that single indirect block works, since 10000 characters is 10000 bytes and requires 20 blocks
	
Running without a test input file (taking input from stdin)
	1. Run make in ./app/ directory or ./ directory to compile all files
	2. Navigate to ./io/ and run file.exe
	3. Freely type in any command from the List of Commands above.

Notes:
	1. Double indirect block is not implemented, thus each file can only have maximum 11 blocks of data(from inode) + 256 blocks of data(from single indirect block)
		-Double indirect block seems unnecessary for such a simple file system
	2. Block 0 is the Superblock
	3. Block 1 is the Free Block Vector
	4. Block 2 is the Inode Block Vector
	5. Block 3 to 9 is reserved
	6. Block 10 to 25 is reserved for inodes
		- There is a maximum of 256 inodes, 16 blocks of inodes each sized 32 bytes.
		
		Inode format: 
		32 bytes
		4 bytes: size of file in bytes (directory size is set to 0)
		4 bytes: flags (0 for file, 1 for directory)
		2 bytes multiplied by 11: Data/Directory Block Number
			- if Data/Directory Block Number is 0, means there is no Data/Directory Block Number associated for that byte.
		2 bytes: Single Indirect Block Number
		
		Single Indirect Block Format:
		2 bytes multiplied by 256: Data/Directory Block Number
		
		Directory block format:
		Each directory block contains 16 entries.
		Each entry is 32 bytes long
		First byte indicates the inode unique number of the file (value of 0 means no entry)
		Next 31 bytes are for the filename, terminated with a “null” character