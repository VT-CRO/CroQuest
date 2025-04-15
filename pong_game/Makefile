# Compiler and linker
CC = gcc
LDFLAGS = -lSDL2 -lSDL2_image -lm -lSDL2_ttf

# Source files
SRC = pong.c utils.c draw.c
OBJ = pong.o utils.o draw.o

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