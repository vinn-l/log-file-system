io = ../io/
disk = ../disk/

objects = $(io)file.c $(disk)disk.o

all : $(io)file.o $(disk)disk.o $(io)file

$(disk)disk.o: $(disk)disk.c
	gcc -std=c99 -c $(disk)disk.c -o $@

$(io)file.o: $(io)file.c
	gcc -std=c99 -c $(io)file.c -o $@

$(io)file: $(io)file.o $(disk)disk.o
	gcc -std=c99 $(io)file.o $(disk)disk.o -o $@