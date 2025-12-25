CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lsqlite3
TARGET = server

OBJ = server.o user.o network.o requests.o registration.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

server.o: server.c user.h network.h requests.h
	$(CC) $(CFLAGS) -c server.c

user.o: user.c user.h
	$(CC) $(CFLAGS) -c user.c

requests.o: requests.c requests.h user.h registration.h
	$(CC) $(CFLAGS) -c requests.c

network.o: network.c network.h
	$(CC) $(CFLAGS) -c network.c

registration.o: registration.c registration.h user.h
	$(CC) $(CFLAGS) -c registration.c

clean:
	rm -f $(OBJ) $(TARGET) server.db

re: clean all

docker-build:
	docker build -t high_load_c_server .

docker-run:
	docker run -p 3490:3490 --rm --name my_server high_load_c_server

docker-stop:
	docker stop my_server