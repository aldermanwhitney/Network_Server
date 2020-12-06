all: KKJserver

KKJserver: Asst3.c
	gcc -Wall -Werror -pthread -fsanitize=address Asst3.c -o KKJserver

clean: 
	rm -f KKJserver *.o
