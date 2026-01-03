TARGET = server
IMAGE_NAME = high_load_c_server
CONTAINER_NAME = my_server
PORT = 3490

CC = gcc
CFLAGS = -fsanitize=address -Wall -Wextra -std=c99 -g -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS = -fsanitize=address

LIBS = -lsqlite3

SRC_DIR = src
INC_DIR = include
OBJ_DIR = build
BIN_DIR = bin
DATA_DIR = data

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: directories $(BIN_DIR)/$(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR)

$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

re: clean all

docker-build:
	docker build -t $(IMAGE_NAME) .

docker-run:
	docker run -p $(PORT):$(PORT) --rm \
	   --privileged \
	   --ulimit nofile=65535:65535 \
	   -v $(PWD)/$(DATA_DIR):/app/$(DATA_DIR) \
	   --name $(CONTAINER_NAME) $(IMAGE_NAME)

docker-build-run:
	make docker-build
	make docker-run

docker-stop:
	docker stop $(CONTAINER_NAME)

.PHONY: all clean re directories docker-build docker-run docker-stop