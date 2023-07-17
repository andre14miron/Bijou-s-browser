# Declaratiile de variabile
CC = gcc
CFLAGS = -g -Wall -lm
 
# Regula de compilare task-uri
build: task1 task2 task3 task4

task1: task1.c
	$(CC) -o task1 task1.c $(CFLAGS)

task2: task2.c
	$(CC) -o task2 task2.c $(CFLAGS)

task3: task3.c
	$(CC) -o task3 task3.c $(CFLAGS)

task4: task4.c
	$(CC) -o task4 task4.c $(CFLAGS)

# Regula de compilare browser

build_browser: browser.c
	$(CC) -o browser browser.c -lmenu -lncurses


# Regulile de "curatenie" (se folosesc pentru stergerea fisierelor intermediare si/sau rezultate)
.PHONY : clean
clean :
	rm -f task1 task2 task3 task4 browser
	rm -f *.out
	rm my_diff
