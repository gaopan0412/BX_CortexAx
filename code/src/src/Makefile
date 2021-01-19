#cc = arm-linux-gnueabihf-g++
#cc = g++
cc = gcc
#cc = arm-linux-gnueabihf-gcc
#CFLAGS = -Wall


all:main
main:  main.c  init.c client.c
	$(cc)  -g -o $@ $^ -lpthread
clean: 
	rm -rf main  *.o
