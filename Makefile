all: knocker.c backdoor.c
	gcc -o knocker knocker.c -std=gnu99
	gcc -o backdoor backdoor.c -lcurl -std=gnu99

clean:
	$(RM) knocker backdoor
