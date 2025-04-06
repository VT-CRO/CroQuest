from random import randint
import tkinter as tk
import os
from PIL import Image, ImageTk

MOVE_INCREMENT = 16
MOVES_PER_SECOND = 10
GAME_SPEED = 1000 // MOVES_PER_SECOND

# Grid constants
GRID_WIDTH = 20
GRID_HEIGHT = 15
TILE_SIZE = 16

# Window Sizes (320, 240)
WINDOW_WIDTH = GRID_WIDTH * TILE_SIZE
WINDOW_HEIGHT = GRID_HEIGHT * TILE_SIZE

# Creating the Snake + Canvas
class Snake(tk.Canvas):
    def __init__(self, parent):
        super().__init__(width = WINDOW_WIDTH, height = WINDOW_HEIGHT, background="black", highlightthickness=0)

        # Setting the snake starting position
        self.snake_positions = [(5 * TILE_SIZE, 5 * TILE_SIZE), (4 * TILE_SIZE, 5 * TILE_SIZE), (3 * TILE_SIZE, 5 * TILE_SIZE)]
        self.food_position = self.set_new_food_position()
        self.score = 0

        # Starting direction
        self.direction = "Right"

        self.bind_all("<Key>", self.on_key_press)

        # Loading all the assets
        self.load_assets()
        self.create_objects()

        self.after(5000000, self.perform_actions)

    # ----------------------------------------------------------------------------------------------------------

    # Loading in the assets for the snake game
    def load_assets(self):
        try:
            self.snake_head_png = Image.open("./assets/closed.png")
            self.snake_head = ImageTk.PhotoImage(self.snake_head_png)

            self.snake_body_png = Image.open("./assets/body_segment.png").resize((TILE_SIZE, TILE_SIZE), Image.NEAREST)
            self.snake_body = ImageTk.PhotoImage(self.snake_body_png)

            self.food_image_png = Image.open("./assets/Apple.png").resize((TILE_SIZE, TILE_SIZE), Image.NEAREST)
            self.food = ImageTk.PhotoImage(self.food_image_png)

            self.background_image_png = Image.open("./assets/bg.png").resize((WINDOW_WIDTH, WINDOW_HEIGHT), Image.NEAREST)
            self.background_image = ImageTk.PhotoImage(self.background_image_png)

        except IOError as error:
            print(error)
            root.destroy()

    # ----------------------------------------------------------------------------------------------------------
    
    # Creating the snake images
    def create_objects(self):
        # No more background png
        #self.create_image(0, 0, image=self.background_image, anchor="nw", tag="background")
        self.create_text(45, 12, text=f"Score {self.score}", tag="score", fill="#fff", font=("TkDefaultFont", 14))
        self.draw_debug_grid()

        # Setting the position
        for x_position, y_position in self.snake_positions:
            self.create_image (x_position, y_position, image=self.snake_body, tag="snake", anchor="nw")

        # Creating the food location
        self.create_image(*self.food_position, image=self.food, tag="food", anchor="nw")

        # Creating the border
        self.create_rectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, outline="black", width=4)

    # ----------------------------------------------------------------------------------------------------------

    # Debug offset
    def draw_debug_grid(self):
        for x in range(0, GRID_WIDTH):
            for y in range(0, GRID_HEIGHT):
                color = "#42B032" if (x + y) % 2 == 0 else "#6FBD63"
                self.create_rectangle(x * TILE_SIZE, y * TILE_SIZE, (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE, fill=color, outline=color, tags="debug_grid")

        # Drawing grid lines
        #for x in range(0, WINDOW_WIDTH, TILE_SIZE):
            #self.create_line(x, 0, x, WINDOW_HEIGHT, fill="black", tags="debug_grid")

        #for y in range(0, WINDOW_HEIGHT, TILE_SIZE):
            #self.create_line(0, y, WINDOW_WIDTH, y, fill="black", tags="debug_grid")   

    # ----------------------------------------------------------------------------------------------------------

    # Method to move the snake
    def move_snake(self):
        head_x_position, head_y_position = self.snake_positions[0]

        # Defining direction with each keystroke
        if self.direction == "Left":
            new_head_position = (head_x_position - MOVE_INCREMENT, head_y_position)
        if self.direction == "Right":
            new_head_position = (head_x_position + MOVE_INCREMENT, head_y_position)
        if self.direction == "Down":
            new_head_position = (head_x_position, head_y_position + MOVE_INCREMENT)
        if self.direction == "Up":
            new_head_position = (head_x_position, head_y_position - MOVE_INCREMENT)

        self.snake_positions = [new_head_position] + self.snake_positions[:-1]

        for segment, position in zip(self.find_withtag("snake"), self.snake_positions):
            self.coords(segment, position)

    # ----------------------------------------------------------------------------------------------------------
    
    # Actions that happen during the game
    def perform_actions(self):
        if self.check_collisions():
            self.end_game()
            return
        
        # Checking for collision and moving the snake
        self.check_food_collision()
        self.move_snake()
        self.after(GAME_SPEED, self.perform_actions)

    # ----------------------------------------------------------------------------------------------------------
    
    def check_collisions(self):
        # Making sure the snake doesn't collide with itself
        head_x_position, head_y_position = self.snake_positions[0]

        if (head_x_position < 0 or head_x_position >= WINDOW_WIDTH or
            head_y_position < TILE_SIZE or head_y_position >= WINDOW_HEIGHT):
            return True

        # Check if the snake hits itself
        if (head_x_position, head_y_position) in self.snake_positions[1:]:
            return True

        return False

        # Old logic
        #return(head_x_position in (0, 600) or head_y_position in (20, 620) or (head_x_position, head_y_position) in self.snake_positions[1:])

    # ----------------------------------------------------------------------------------------------------------
    
    # No weird directional turns
    def on_key_press(self, e):
        new_direction = e.keysym
        all_directions = ("Up", "Down", "Left", "Right")
        opposites = ({"Up", "Down"}, {"Left", "Right"})

        if new_direction in all_directions and {new_direction, self.direction} not in opposites:
            self.direction = new_direction

    # ----------------------------------------------------------------------------------------------------------
    
    # Checking for the food collisions
    def check_food_collision(self):
        if self.snake_positions[0] == self.food_position:
            self.score += 1
            self.snake_positions.append(self.snake_positions[-1])

            self.create_image(*self.snake_positions[-1], image=self.snake_body, tag="snake", anchor="nw")

            self.food_position = self.set_new_food_position()
            self.coords(self.find_withtag("food"), self.food_position)

            score = self.find_withtag("score")
            self.itemconfigure(score, text=f"Score: {self.score}", tag="score")

    # ----------------------------------------------------------------------------------------------------------
    
    # Setting a new position for the food
    def set_new_food_position(self):
        while True:
            x_tile = randint(0, GRID_WIDTH - 1)
            y_tile = randint(1, GRID_HEIGHT - 1)

            x_position = x_tile * TILE_SIZE
            y_position = y_tile * TILE_SIZE

            food_position = (x_position, y_position)

            if food_position not in self.snake_positions:
                return food_position
            
    # ----------------------------------------------------------------------------------------------------------

    # Ending the game
    def end_game(self):
        self.delete(tk.ALL)
        self.create_text(self.winfo_width()/2, self.winfo_height()/2, text=f"Game over! You scored {self.score}!", fill = "#fff", font=("TkDefaultFont", 24))


root = tk.Tk()
root.title("CroQuest Snake")
root.resizable(True, True)

board = Snake(root)
board.pack()

root.mainloop()