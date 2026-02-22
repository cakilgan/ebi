# EBI — Easy Binary Image

**Version:** 1 &nbsp;|&nbsp; **Language:** C++17

EBI is a minimal, single-header binary image format for storing raw RGB/RGBA pixel data. It has no external dependencies — just drop `ebi.hpp` into your project and go.

---

## Features

- Single-header (`ebi.hpp`), no dependencies
- RGB, RGBA, and ARGB pixel formats
- Configurable image origin (top-left or bottom-left)
- Read/write EBI files from disk
- Utility functions: `set_pixel`, `fill_color`, `vertical_flip`, `to_ppm`
- Fixed 25-byte packed header with reserved space for future use

---

## File Format

Every EBI file begins with a 25-byte packed header followed by raw pixel data.

| Field       | Type      | Size | Description                                        |
|-------------|-----------|------|----------------------------------------------------|
| `magic`     | `char[4]` | 4 B  | `'E'`, `'B'`, `'I'`, then the version byte (`1`)  |
| `origin`    | `u8`      | 1 B  | `0` = top-left, `1` = bottom-left                 |
| `width`     | `u16`     | 2 B  | Image width in pixels                              |
| `height`    | `u16`     | 2 B  | Image height in pixels                             |
| `channels`  | `u8`      | 1 B  | Number of channels (`3` = RGB, `4` = ARGB/RGBA)   |
| `format`    | `u8`      | 1 B  | Pixel format (see below)                           |
| `data_size` | `u32`     | 4 B  | Size of pixel data in bytes                        |
| `flags`     | `u16`     | 2 B  | Reserved for future flags                          |
| `reserved`  | `u8[8]`   | 8 B  | Reserved, zero-padded                              |

**Pixel Formats:**

| Constant          | Value | Layout  |
|-------------------|-------|---------|
| `EBI_FORMAT_RGB`  | `0`   | R G B   |
| `EBI_FORMAT_ARGB` | `1`   | A R G B |
| `EBI_FORMAT_RGBA` | `2`   | R G B A |

**Origin:**

| Constant                 | Value | Description                      |
|--------------------------|-------|----------------------------------|
| `EBI_ORIGIN_TOP_LEFT`    | `0`   | Row 0 is the top of the image    |
| `EBI_ORIGIN_BOTTOM_LEFT` | `1`   | Row 0 is the bottom of the image |

---

## Usage

### Writing an image

```cpp
#include "ebi.hpp"

int main() {
    const int W = 256, H = 256, C = 3;

    ebi::file img;
    img.header.width     = W;
    img.header.height    = H;
    img.header.channels  = C;
    img.header.format    = EBI_FORMAT_RGB;
    img.header.origin    = EBI_ORIGIN_TOP_LEFT;
    img.header.data_size = W * H * C;
    img.header.flags     = 0;

    img.data = new ebi::u8[img.header.data_size];

    // Fill with solid red
    ebi::fill_color(img.data, W, H, C, 255, 0, 0);

    // Draw a white pixel at (10, 10)
    ebi::set_pixel(img.data, W, 10, 10, C, 255, 255, 255);

    ebi::error err = ebi::write("output.ebi", img);
    if (err != ebi::ok) { /* handle error */ }

    delete[] img.data;
}
```

### Reading an image

```cpp
#include "ebi.hpp"

int main() {
    ebi::file img;
    ebi::error err = ebi::read("output.ebi", img);

    if (err == ebi::file_not_found) { /* file missing */ }
    if (err == ebi::general)        { /* invalid format or read error */ }

    // use img.header and img.data ...

    // img.data is heap-allocated — free when done
    delete[] img.data;
}
```

### Converting to PPM

```cpp
ebi::to_ppm(img, "output.ppm");
```

> **Note:** PPM only supports RGB. If the image has 4 channels, the alpha channel is silently ignored and a warning is printed to stdout.

---

## API Reference

### `ebi::write(path, file) → error`

Writes an EBI file to disk. Automatically fills the `magic` bytes before writing.

### `ebi::read(path, file) → error`

Reads an EBI file from disk. Allocates `file.data` on the heap with `new[]` — **the caller is responsible for freeing it** with `delete[]`.

### `ebi::set_pixel(data, width, x, y, channels, r, g, b, a=255)`

Sets a single pixel at `(x, y)`. Alpha is written only when `channels == 4`.

### `ebi::fill_color(data, width, height, channels, r, g, b, a=255)`

Fills the entire pixel buffer with a solid color.

### `ebi::vertical_flip(data, width, height, channels)`

Flips the image vertically in-place. Useful for converting between top-left and bottom-left origins.

### `ebi::to_ppm(file, path)`

Exports the image as a binary PPM (P6) file. Supports RGB, RGBA, and ARGB input formats.

---

## Error Codes

| Code                  | Value | Meaning                                   |
|-----------------------|-------|-------------------------------------------|
| `ebi::ok`             | `1`   | Success                                   |
| `ebi::general`        | `-1`  | Generic error (bad magic, read failure…)  |
| `ebi::file_not_found` | `-33` | Could not open the file                   |

---

## Integration

EBI is a single-header library. Copy `ebi.hpp` into your project and include it:

```cpp
#include "ebi.hpp"
```

Requires **C++17** or later. No build system changes needed.
