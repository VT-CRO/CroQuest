import tkinter as tk
from PIL import Image, ImageTk

class Snake(tk.Canvas):
    def __init__(self):
        super().__init__(width = 20, height = 20, background = "black", highlightthickness=0)

        self.load_assets()
    
    def load_assets(self):
        try:
            self.snake_body_png = Image.open("./assets/snake.png")
            self.snake_body = ImageTk.PhotoImage(self.snake_body_png)

            self.apple_image_png = Image.open("./assets/apple.png")
            self.apple = ImageTk.PhotoImage(self.apple_image_png)
        except IOError as error:
            print(error)
            root.destroy()

root = tk.Tk()
root.title("CroQuest Snake")
root.resizable(False, False)

board = Snake()
board.pack()

root.mainloop()