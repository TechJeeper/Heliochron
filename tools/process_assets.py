from PIL import Image
from pathlib import Path

root = Path(r"C:\Projects\Heliochron\resources\images")
planets = root / "planets"


def trim_transparent_or_near_black(im, thr=18):
    im = im.convert("RGBA")
    px = im.load()
    w, h = im.size
    minx, miny, maxx, maxy = w, h, 0, 0
    found = False
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 16:
                continue
            if a > 200 and r <= thr and g <= thr and b <= thr:
                px[x, y] = (0, 0, 0, 0)
                continue
            found = True
            if x < minx:
                minx = x
            if y < miny:
                miny = y
            if x > maxx:
                maxx = x
            if y > maxy:
                maxy = y
    if not found:
        return im
    return im.crop((minx, miny, maxx + 1, maxy + 1))


def fit_square(im, size):
    im = trim_transparent_or_near_black(im)
    w, h = im.size
    side = max(w, h)
    canvas = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    canvas.paste(im, ((side - w) // 2, (side - h) // 2), im)
    return canvas.resize((size, size), Image.NEAREST)


sizes = {
    "sun": 36,
    "mercury": 12,
    "earth": 18,
    "mars": 22,
    "saturn": 24,
    "jupiter": 22,
}

for name, size in sizes.items():
    src = planets / f"{name}_raw.png"
    out = planets / f"{name}.png"
    im = Image.open(src)
    fitted = fit_square(im, size)
    fitted.save(out)
    print(name, im.size, "->", fitted.size)

sf = Image.open(root / "starfield_raw.png").convert("RGBA")
print("starfield raw", sf.size)
px = sf.load()
w, h = sf.size


def is_border(x, y):
    r, g, b, a = px[x, y]
    return r > 230 and g > 230 and b > 230


minx = 0
while minx < w - 1 and all(is_border(minx, y) for y in range(h)):
    minx += 1
maxx = w - 1
while maxx > minx and all(is_border(maxx, y) for y in range(h)):
    maxx -= 1
miny = 0
while miny < h - 1 and all(is_border(x, miny) for x in range(w)):
    miny += 1
maxy = h - 1
while maxy > miny and all(is_border(x, maxy) for x in range(w)):
    maxy -= 1

sf2 = sf.crop((minx, miny, maxx + 1, maxy + 1))
print("starfield cropped", sf2.size, "from", (minx, miny, maxx, maxy))
sf2 = sf2.convert("RGB").resize((200, 228), Image.NEAREST)
sf2.save(root / "starfield.png")
print("starfield saved", sf2.size)

menu = Image.new("RGBA", (25, 25), (0, 0, 0, 255))
sun = Image.open(planets / "sun.png").convert("RGBA").resize((17, 17), Image.NEAREST)
earth = Image.open(planets / "earth.png").convert("RGBA").resize((7, 7), Image.NEAREST)
mars = Image.open(planets / "mars.png").convert("RGBA").resize((6, 6), Image.NEAREST)
menu.paste(sun, (4, 4), sun)
menu.paste(earth, (17, 3), earth)
menu.paste(mars, (2, 17), mars)
menu.save(root / "menu_icon.png")
print("menu_icon saved", menu.size)
