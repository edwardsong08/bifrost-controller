"""Generate the provisional TROA controller icon. Requires Pillow; not needed to build."""
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
scale = 4
image = Image.new("RGBA", (128 * scale, 128 * scale))
draw = ImageDraw.Draw(image)
def box(values):
    return tuple(round(value * scale) for value in values)

draw.rounded_rectangle(box((4, 4, 124, 124)), radius=28 * scale, fill="#182635")
draw.rounded_rectangle(box((22, 40, 106, 91)), radius=20 * scale, outline="#ddb975", width=5 * scale)
draw.line(box((34, 57, 34, 73)), fill="#f5f1e8", width=5 * scale)
draw.line(box((26, 65, 42, 65)), fill="#f5f1e8", width=5 * scale)
draw.ellipse(box((82, 55, 90, 63)), fill="#f5f1e8")
draw.ellipse(box((92, 65, 100, 73)), fill="#f5f1e8")
image = image.resize((256, 256), Image.Resampling.LANCZOS)
image.save(root / "src/images/troa-controller.ico", sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
image.save(root / "src/images/troa-controller.png")
