GCC=gcc
FLAGS= -g -Wall


all: cclient server cleanup

test: main.c table.o table.h
	$(GCC) $(FLAGS) -o test main.c table.o -lm

table.o: table.c table.h
	$(GCC) $(FLAGS) -c table.c -lm

helper_client.o: helper_client.c helper_client.h
	$(GCC) $(FLAGS) -c helper_client.c

cclient: myClient.c networks.o gethostbyname6.o helper_client.o
	$(GCC) $(FLAGS) -o cclient myClient.c networks.o gethostbyname6.o helper_client.o

server: myServer.c networks.o gethostbyname6.o table.o helper_client.h
	$(GCC) $(FLAGS) -o server myServer.c networks.o gethostbyname6.o table.o helper_client.o -lm

.c.o:
		gcc -c $(FLAGS) $< -o $@ 

#networks.o: networks.c networks.o gethostbyname6.h
#	$(GCC) $(FLAGS) -c networks.c

#gethostbyname6.o: gethostbyname6.c gethostbyname6.h
#	$(GCC) $(FLAGS) -c gethostbyname6.c

clean: 
	rm -f cclient server *.o

cleanup:
	rm *.o
