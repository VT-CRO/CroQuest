# Compiler and linker
CC = gcc
LDFLAGS = -lSDL2 -lSDL2_image -lm

# Source files
SRC = pong.c utils.c
OBJ = pong.o utils.o

EXEC = pong_game

# Default
all: $(EXEC)

# Link the object files
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compile the source code into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(EXEC)

# Run the game
run: $(EXEC)
	./$(EXEC)