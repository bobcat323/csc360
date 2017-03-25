exec: \
compile
	./PMan.out

compile: \
PMan.c
	gcc PMan.c -o PMan.out
