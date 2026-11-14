#!/usr/bin/env python3
"""Render Rebble App Store screenshots at exact platform resolutions.

No watch bezel — store requirement. Composites PixelLab planets on the
starfield to match Heliochron's orbital watchface.
"""
from __future__ import annotations

import math
import os
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "store" / "screenshots"
IMG = ROOT / "resources" / "images"
PLANETS = IMG / "planets"

PLATFORMS = {
    "basalt": (144, 168, True),
    "emery": (200, 228, True),
    "gabbro": (260, 260, True),  # Pebble Round 2
}

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
DARK_GRAY = (85, 85, 85)

THEMES = {
    "classic": {
        "orbit": (0, 0, 85),          # Oxford Blue-ish
        "text": WHITE,
        "accent": (255, 170, 0),      # Chrome Yellow
        "battery": (0, 170, 0),
        "dim": DARK_GRAY,
    },
    "ember": {
        "orbit": (170, 0, 0),
        "text": (255, 170, 0),
        "accent": (255, 0, 0),
        "battery": (255, 85, 0),
        "dim": DARK_GRAY,
    },
    "ice": {
        "orbit": (85, 170, 255),
        "text": WHITE,
        "accent": (170, 255, 255),
        "battery": (0, 170, 255),
        "dim": DARK_GRAY,
    },
    "neon": {
        "orbit": (255, 0, 170),
        "text": (0, 255, 0),
        "accent": (0, 255, 255),
        "battery": (255, 0, 255),
        "dim": DARK_GRAY,
    },
}

# Five store shots — variety of times, themes, and hour bodies.
SCENES = [
    {
        "file": "01-classic-mars.png",
        "hour": 10,
        "minute": 42,
        "second": 18,
        "theme": "classic",
        "hour_body": "mars",
        "show_digital": True,
        "show_date": True,
        "date": "Sat 14 Nov",
        "show_seconds": True,
        "show_orbits": True,
        "show_starfield": True,
        "show_battery": True,
        "battery": 82,
        "elliptical": False,
    },
    {
        "file": "02-ember-saturn.png",
        "hour": 21,
        "minute": 17,
        "second": 45,
        "theme": "ember",
        "hour_body": "saturn",
        "show_digital": True,
        "show_date": False,
        "show_seconds": True,
        "show_orbits": True,
        "show_starfield": True,
        "show_battery": True,
        "battery": 54,
        "elliptical": False,
    },
    {
        "file": "03-ice-jupiter.png",
        "hour": 8,
        "minute": 5,
        "second": 30,
        "theme": "ice",
        "hour_body": "jupiter",
        "show_digital": True,
        "show_date": True,
        "date": "Sun 15 Nov",
        "show_seconds": True,
        "show_orbits": True,
        "show_starfield": True,
        "show_battery": False,
        "battery": 100,
        "elliptical": False,
    },
    {
        "file": "04-neon-ellipse.png",
        "hour": 14,
        "minute": 28,
        "second": 7,
        "theme": "neon",
        "hour_body": "mars",
        "show_digital": True,
        "show_date": False,
        "show_seconds": True,
        "show_orbits": True,
        "show_starfield": True,
        "show_battery": True,
        "battery": 33,
        "elliptical": True,
    },
    {
        "file": "05-minimal.png",
        "hour": 18,
        "minute": 0,
        "second": 0,
        "theme": "classic",
        "hour_body": "mars",
        "show_digital": False,
        "show_date": False,
        "show_seconds": False,
        "show_orbits": False,
        "show_starfield": True,
        "show_battery": False,
        "battery": 100,
        "elliptical": False,
    },
]


def font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    if bold:
        candidates = [
            "C:/Windows/Fonts/consolab.ttf",
            "C:/Windows/Fonts/lucon.ttf",
            "C:/Windows/Fonts/courbd.ttf",
            "C:/Windows/Fonts/segoeuib.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
        ]
    else:
        candidates = [
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
        ]
    for path in candidates:
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def draw_text_center(draw: ImageDraw.ImageDraw, text: str, y: int, w: int, fnt, fill) -> None:
    bbox = draw.textbbox((0, 0), text, font=fnt)
    tw = bbox[2] - bbox[0]
    draw.text(((w - tw) // 2, y), text, font=fnt, fill=fill)


def hour_angle(hour: int, minute: int) -> float:
    hour12 = hour % 12
    return (hour12 / 12.0 + minute / (12.0 * 60.0)) * 2 * math.pi


def minute_angle(minute: int, second: int) -> float:
    return (minute / 60.0 + second / (60.0 * 60.0)) * 2 * math.pi


def second_angle(second: int) -> float:
    return (second / 60.0) * 2 * math.pi


def orbit_point(cx: float, cy: float, rx: float, ry: float, angle: float) -> tuple[int, int]:
    # 0 = 12 o'clock, clockwise
    a = angle - math.pi / 2
    return (int(round(cx + math.cos(a) * rx)), int(round(cy + math.sin(a) * ry)))


def draw_ellipse_ring(draw: ImageDraw.ImageDraw, cx: int, cy: int, rx: int, ry: int, color) -> None:
    bbox = [cx - rx, cy - ry, cx + rx, cy + ry]
    draw.ellipse(bbox, outline=color, width=1)


def paste_centered(canvas: Image.Image, sprite: Image.Image, center: tuple[int, int]) -> None:
    x = center[0] - sprite.size[0] // 2
    y = center[1] - sprite.size[1] // 2
    canvas.paste(sprite, (x, y), sprite)


def load_sprite(name: str) -> Image.Image:
    return Image.open(PLANETS / name).convert("RGBA")


def twinkle(draw: ImageDraw.ImageDraw, w: int, h: int, color, pulse: int = 2) -> None:
    xs = [18, 42, 70, 95, 120, 148, 172, 28, 160, 88, 55, 185]
    ys = [22, 48, 16, 70, 34, 58, 90, 110, 130, 150, 175, 200]
    for i, (x0, y0) in enumerate(zip(xs, ys)):
        if ((pulse + i * 3) % 7) > 4:
            continue
        x = x0 % w
        y = y0 % h
        draw.point((x, y), fill=color)
        if (i + pulse) % 5 == 0:
            draw.point((x + 1, y), fill=color)
            draw.point((x, y + 1), fill=color)


def render(w: int, h: int, scene: dict) -> Image.Image:
    theme = THEMES[scene["theme"]]
    canvas = Image.new("RGB", (w, h), BLACK)
    draw = ImageDraw.Draw(canvas)
    is_round = w == h

    cy_nudge = 0 if is_round else (6 if scene["show_digital"] else 0)
    cx, cy = w // 2, h // 2 - cy_nudge

    if scene["show_starfield"]:
        sf_path = IMG / ("starfield_round.png" if is_round else "starfield.png")
        sf = Image.open(sf_path).convert("RGB")
        if sf.size != (w, h):
            # Center native art; do not stretch (matches watchface draw path).
            ox = (w - sf.size[0]) // 2
            oy = (h - sf.size[1]) // 2
            canvas.paste(sf, (ox, oy))
        else:
            canvas.paste(sf, (0, 0))

    twinkle(draw, w, h, theme["accent"], pulse=scene["second"] % 7)

    max_r = min(w, h) // 2 - (14 if is_round else 8)
    sec_rx = max_r * 38 // 100
    min_rx = max_r * 58 // 100
    hour_rx = max_r * 82 // 100
    sec_ry = sec_rx
    min_ry = min_rx
    hour_ry = hour_rx
    if scene["elliptical"]:
        sec_ry = sec_rx * 78 // 100
        min_ry = min_rx * 78 // 100
        hour_ry = hour_rx * 78 // 100

    if scene["show_orbits"]:
        draw_ellipse_ring(draw, cx, cy, sec_rx, sec_ry, theme["orbit"])
        draw_ellipse_ring(draw, cx, cy, min_rx, min_ry, theme["orbit"])
        draw_ellipse_ring(draw, cx, cy, hour_rx, hour_ry, theme["orbit"])

    if scene["show_battery"]:
        pct = max(0, min(100, scene["battery"]))
        span = pct / 100.0 * 2 * math.pi
        steps = 36
        prev = orbit_point(cx, cy, max_r - 2, max_r - 2, 0)
        for i in range(1, steps + 1):
            ang = i * span / steps
            p = orbit_point(cx, cy, max_r - 2, max_r - 2, ang)
            draw.line([prev, p], fill=theme["battery"], width=2)
            prev = p

    # Scale sprites for basalt (down) / gabbro (slightly up)
    if w >= 260:
        scale = 1.15
    elif w >= 200:
        scale = 1.0
    else:
        scale = 0.78
    sun = load_sprite("sun.png")
    earth = load_sprite("earth.png")
    mercury = load_sprite("mercury.png")
    hour_name = {"mars": "mars.png", "saturn": "saturn.png", "jupiter": "jupiter.png"}[
        scene["hour_body"]
    ]
    hour_body = load_sprite(hour_name)

    def sized(im: Image.Image) -> Image.Image:
        if abs(scale - 1.0) < 0.01:
            return im
        nw = max(1, int(round(im.size[0] * scale)))
        nh = max(1, int(round(im.size[1] * scale)))
        return im.resize((nw, nh), Image.NEAREST)

    sun, earth, mercury, hour_body = map(sized, (sun, earth, mercury, hour_body))

    paste_centered(canvas, sun, (cx, cy))
    paste_centered(
        canvas,
        hour_body,
        orbit_point(cx, cy, hour_rx, hour_ry, hour_angle(scene["hour"], scene["minute"])),
    )
    paste_centered(
        canvas,
        earth,
        orbit_point(cx, cy, min_rx, min_ry, minute_angle(scene["minute"], scene["second"])),
    )
    if scene["show_seconds"]:
        paste_centered(
            canvas,
            mercury,
            orbit_point(cx, cy, sec_rx, sec_ry, second_angle(scene["second"])),
        )

    # Redraw text atop sprites
    draw = ImageDraw.Draw(canvas)
    time_y = h - 56 if is_round else h - 40
    date_y = h - 34 if is_round else h - 22
    if scene["show_digital"]:
        hour = scene["hour"] % 12
        if hour == 0:
            hour = 12
        time_str = f"{hour:02d}:{scene['minute']:02d}"
        draw_text_center(
            draw,
            time_str,
            time_y,
            w,
            font(max(14, w // 11), bold=True),
            theme["text"],
        )

    if scene["show_date"]:
        draw_text_center(
            draw,
            scene.get("date", ""),
            date_y,
            w,
            font(max(11, w // 14)),
            theme["dim"],
        )

    # Soft circular crop for Round 2 store shots (no bezel chrome).
    if is_round:
        mask = Image.new("L", (w, h), 0)
        ImageDraw.Draw(mask).ellipse((0, 0, w - 1, h - 1), fill=255)
        rounded = Image.new("RGB", (w, h), BLACK)
        rounded.paste(canvas, (0, 0), mask)
        return rounded

    return canvas


def main() -> None:
    if OUT.exists():
        shutil.rmtree(OUT)
    for plat, (w, h, _color) in PLATFORMS.items():
        dest = OUT / plat
        dest.mkdir(parents=True, exist_ok=True)
        for scene in SCENES:
            shot = render(w, h, scene)
            path = dest / scene["file"]
            shot.save(path, "PNG", optimize=True)
            print(f"wrote {path.relative_to(ROOT)} ({w}x{h})")
    print("Done:", OUT)


if __name__ == "__main__":
    main()
