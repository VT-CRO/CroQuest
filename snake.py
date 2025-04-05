import tkinter as tk
import os
from PIL import Image, ImageTk

class Snake(tk.Canvas):
    def __init__(self, parent):
        super().__init__(width = 600, height = 600, background="black", highlightthickness=0)

        self.snake_positions = [(100, 100), (80, 100), (60, 100)]

        self.load_assets()
        self.create_objects()
    
    def load_assets(self):
        try:
            self.snake_body_png = Image.open("./assets/body_segment.png")
            self.snake_body = ImageTk.PhotoImage(self.snake_body_png)

            self.apple_image_png = Image.open("./assets/Apple.png")
            self.apple = ImageTk.PhotoImage(self.apple_image_png)
        except IOError as error:
            print(error)
            root.destroy()

    def create_objects(self):
        for x_position, y_position in self.snake_positions:
            self.create_image (x_position, y_position, image=self.snake_body, tag="snake")

root = tk.Tk()
root.title("CroQuest Snake")
root.resizable(False, False)

board = Snake(root)
board.pack()

root.mainloop()