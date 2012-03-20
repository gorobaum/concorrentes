PROG_NAME = ep1
FLAGS=-lpthread

SRC_DIR = src
OBJ_DIR = .temp
#OUTPUT_DIR = bin

CC = gcc
CFLAGS =  $(FLAGS)

#OUTPUT = bin/$(PROG_NAME)
OUTPUT = $(PROG_NAME)

include objs.makefile

#$(PROG_NAME): $(OBJ_DIR) $(OUTPUT_DIR) $(OBJS)
$(PROG_NAME): $(OBJ_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTPUT)

$(OBJ_DIR):
	mkdir $@

#$(OUTPUT_DIR):
#	mkdir $@

.temp/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

include deps.makefile

.PHONY: debug
debug: CFLAGS += -g
debug: $(PROG_NAME)

.PHONY: clean
clean:
	rm -rf $(OUTPUT)
	rm -rf $(OBJ_DIR)


