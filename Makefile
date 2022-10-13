PORT=4000

default: build

help:
	@echo "Truco Online"
	@echo
	@echo "Target rules:"
	@echo "    all      - Compiles and generates binary file"
	@echo "    server    - Starts a server"
	@echo "    client 	- Runs a client"
	@echo "    clean    - Clean the project by removing binaries"
	@echo "    help     - Prints a help message with target rules"

build:
	gcc -Wall src/client.c -o bin/client
	gcc -Wall src/server.c -o bin/server

server:
	./bin/server 5004

client:
	./bin/client localhost 5004

clean:
	@rm ./bin/*