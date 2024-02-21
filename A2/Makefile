all: build

build:
	gcc -ggdb -o s-talk list.o main.c udpReceive.c screenOutput.c keyboardInput.c  udpSender.c -lpthread
clean:
	rm s-talk
