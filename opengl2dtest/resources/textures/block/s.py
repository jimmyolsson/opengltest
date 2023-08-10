
from PIL import Image

def add_alpha_channel(img_path):
    img = Image.open(img_path).convert("RGBA")
    img.save(img_path)

add_alpha_channel("dirt_grass_side.png")
add_alpha_channel("white_concrete.png")