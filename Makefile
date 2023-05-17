CC = gcc
CFLAGS = -g
CLINK = -pthread

SRCDIR=src
INCLDIR=include
LIBDIR=lib

LOCALADDR=127.0.0.1
PORT=9986

launcher: $(SRCDIR)/launcher.c $(LIBDIR)/utils.o client server
	$(CC) $(CLINK) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/launcher.c -o launcher

client: $(SRCDIR)/client.c $(LIBDIR)/utils.o
	$(CC) $(CLINK) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/client.c -o client
	
server: $(SRCDIR)/server.c $(LIBDIR)/utils.o
	$(CC) $(CLINK) $(CFLAGS) -I$(INCLDIR) $(LIBDIR)/utils.o $(SRCDIR)/server.c -o server

$(LIBDIR)/utils.o: $(SRCDIR)/utils.c
	$(CC) $(CLINK) $(CFLAGS) -I$(INCLDIR) -c $(SRCDIR)/utils.c -o $(LIBDIR)/utils.o

run1: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input1.csv

run2: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input2_0.csv input2_1.csv input2_2.csv input2_3.csv input2_4.csv input2_5.csv

run3: launcher
	./launcher $(LOCALADDR) $(PORT) 8 input2_0.csv input2_1.csv input2_2.csv input2_3.csv input2_4.csv input2_5.csv

run4: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input4.csv

t1: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input1.csv
	diff expected/balances1.csv output/balances.csv
	@echo passed

t2: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input2_0.csv input2_1.csv input2_2.csv input2_3.csv input2_4.csv input2_5.csv
	diff expected/balances2.csv output/balances.csv
	@echo passed

t3: launcher
	./launcher $(LOCALADDR) $(PORT) 8 input2_0.csv input2_1.csv input2_2.csv input2_3.csv input2_4.csv input2_5.csv
	diff expected/balances2.csv output/balances.csv
	@echo passed

t4: launcher
	./launcher $(LOCALADDR) $(PORT) 1 input4.csv
	diff expected/balances4.csv output/balances.csv
	diff expected/account0.csv output/account0.csv
	diff expected/account1.csv output/account1.csv
	diff expected/account2.csv output/account2.csv
	@echo passed

.PHONY: clean run1 run2 run3 run4 t1 t2 t3 t4

clean: 
	rm server client launcher $(LIBDIR)/utils.o
