io = ./io/
disk = ./disk/

objects = $(io)file.c $(disk)disk.o

all : $(io)file.o $(disk)disk.o $(io)file

$(disk)disk.o: $(disk)disk.c
	gcc -c $(disk)disk.c -o $@

$(io)file.o: $(io)file.c
	gcc -c $(io)file.c -o $@

$(io)file: $(io)file.o $(disk)disk.o
	gcc $(io)file.o $(disk)disk.o -o $@