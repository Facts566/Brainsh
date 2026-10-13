#!/usr/bin/env python3
"""Generate the Brainsh extension icon (128x128 PNG).

Draws a rounded-square badge with a brain-like split and the language's
signature symbols. Kept dependency-light: only Pillow is required.
"""
from PIL import Image, ImageDraw

S = 128
SCALE = 4  # supersample for smooth edges
W = S * SCALE

BG_TOP = (56, 120, 200)
BG_BOTTOM = (32, 74, 140)
BRAIN = (238, 244, 252)
INK = (28, 52, 92)
ACCENT = (247, 181, 56)


def lerp(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def rounded_mask():
    img = Image.new("L", (W, W), 0)
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([0, 0, W - 1, W - 1], radius=int(W * 0.22), fill=255)
    return img


def badge():
    img = Image.new("RGBA", (W, W))
    px = img.load()
    for y in range(W):
        row = lerp(BG_TOP, BG_BOTTOM, y / (W - 1))
        for x in range(W):
            px[x, y] = row + (255,)
    return img


def brain_shape(d, cx, cy, w, h):
    """Two rounded lobes separated by a clear gap: a simple brain glyph."""
    gap = w * 0.09
    lobe_w = (w - gap) / 2
    top, bot = cy - h / 2, cy + h / 2
    left = (cx - gap / 2 - lobe_w, cx - gap / 2)
    right = (cx + gap / 2, cx + gap / 2 + lobe_w)

    for x0, x1 in (left, right):
        d.rounded_rectangle([x0, top, x1, bot], radius=lobe_w * 0.30, fill=BRAIN)


def main():
    img = badge()
    d = ImageDraw.Draw(img)
    cx = W / 2

    # Brain lobes
    brain_shape(d, cx, W * 0.38, W * 0.66, W * 0.34)

    # Pointer arrow under the brain: the one symbol every brainfuck has
    ay = W * 0.72
    d.line([cx - W * 0.17, ay, cx + W * 0.10, ay], fill=INK, width=int(W * 0.055))
    d.polygon([
        (cx + W * 0.12, ay),
        (cx + W * 0.03, ay - W * 0.06),
        (cx + W * 0.03, ay + W * 0.06),
    ], fill=INK)

    # Accent dot: the current cell
    r = W * 0.038
    d.ellipse([cx - W * 0.17 - r, ay - r, cx - W * 0.17 + r, ay + r], fill=ACCENT)

    img = img.resize((S, S), Image.LANCZOS)
    out = img.convert("RGBA")
    out.putalpha(rounded_mask().resize((S, S), Image.LANCZOS))

    path = "/home/dima/Documents/c++/Brainsh/brainsh-vscode/brainsh.png"
    out.save(path, "PNG", optimize=True)
    print(f"wrote {path} ({out.size[0]}x{out.size[1]})")


if __name__ == "__main__":
    main()