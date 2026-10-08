"""Package the unmodified deployed TROA favicon as a native Windows icon.

The site serves PNG bytes at /favicon.ico. Keep those exact bytes in
src/images/troa-logo.png; this script creates the ICO container/sizes required
by Windows resources and NSIS. It does not redraw or recolor the artwork.
Requires Pillow, only for maintainers regenerating the already committed icon.
"""
from pathlib import Path
from PIL import Image

images = Path(__file__).resolve().parents[1] / "src" / "images"
with Image.open(images / "troa-logo.png") as logo:
    logo.save(images / "troa-logo.ico", format="ICO",
              sizes=[(size, size) for size in (16, 24, 32, 48, 64, 128, 256)])
