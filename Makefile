PORT=4000

default: build

help:
	@echo "Truco Online"
	@echo
	@echo "Target rules:"
	@echo "    build      - Compiles and generates binary file"
	@echo "    run_server    - Starts a server"
	@echo "    run_client 	- Runs a client"
	@echo "    clean    - Clean the project by removing binaries"
	@echo "    help     - Prints a help message with target rules"

build: cliente.o print.o truco.o server.o
	gcc -o bin/cliente bin/cliente.o bin/print.o bin/truco.o
	gcc -o bin/server bin/server.o bin/truco.o

server.o: src/server.c src/truco.h
	gcc -o bin/server.o src/server.c -c -Wall

cliente.o: src/cliente.c src/truco.h src/print.h
	gcc -o bin/cliente.o src/cliente.c -c -Wall

print.o: src/print.c src/print.h src/truco.h
	gcc -o bin/print.o src/print.c -c -Wall

truco.o: src/truco.c src/truco.h
	gcc -o bin/truco.o src/truco.c -c -Wall

run_server:
	./bin/server 5004

run_cliente:
	./bin/cliente localhost 5004

clean:
	@rm ./bin/*