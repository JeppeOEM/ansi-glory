# ansi2html - ANSI Art to HTML Converter

Convert ANSI art files (.ANS) to HTML with accurate colors, CP437 character support, and authentic retro fonts.

## Features

- **Full ANSI color support** - 16 standard colors with proper CGA palette
- **CP437 character set** - All extended ASCII characters (128-255) converted to Unicode
- **Authentic DOS fonts** - Embedded fonts from `old_fonts` directory
- **Font selector** - Interactive dropdown to switch between different retro fonts
- **CSS animations ready** - Each character has unique `data-id`, `data-row`, and `data-col` attributes
- **24-bit true color** - Optional support with `--truecolor` flag
- **Cursor movement** - Handles `ESC[A/B/C/D/H` sequences for proper ANSI art rendering

## Installation

```bash
gcc -Wall -Wextra -O2 -o ansi2html ansi2html.c
```

## Usage

### Basic Conversion
```bash
./ansi2html < artwork.ans > artwork.html
```

### With True Color Support
```bash
./ansi2html --truecolor < modern.ans > modern.html
```

### Help
```bash
./ansi2html --help
```

## Output Format

The generated HTML includes:
- **Embedded CSS** with all color definitions
- **Font declarations** linking to fonts in `old_fonts/ttf - Px (pixel outline)/`
- **Font selector** dropdown at the top of the page
- **Character spans** with attributes:
  - `data-id` - Unique sequential identifier (for animations)
  - `data-row` - Row number (0-based)
  - `data-col` - Column number (0-based)
  - `class` - CSS classes (`fg-N`, `bg-N`, `bold`)

### Example HTML Structure
```html
<div class="controls">
  <label for="font-select">Font:</label>
  <select id="font-select" onchange="changeFont(this.value)">
    <option value="Px437_IBM_VGA_8x16" selected>Px437_IBM_VGA_8x16</option>
    ...
  </select>
</div>
<div class="content">
  <pre class="ansi-art" id="ansi-art"><span data-id="0" data-row="0" data-col="0" class="fg-7">X</span><span data-id="1" data-row="0" data-col="1" class="fg-10 bold">Y</span>...</pre>
</div>
```

**Note**: The `<pre>` tag is essential for proper character alignment. Each character is wrapped in a `<span>` with no extra whitespace between tags to ensure pixel-perfect rendering.

## Fonts

The HTML references fonts from the `old_fonts/ttf - Px (pixel outline)/` directory. Make sure this folder is in the same location as your HTML output.

Default fonts included:
- **Px437_IBM_VGA_8x16** (Default)
- Px437_IBM_VGA_9x16
- Px437_IBM_EGA_8x14
- Px437_IBM_BIOS
- Px437_IBM_CGA
- Px437_DOS-V_re_JPN12

## Color Palette

Standard ANSI/CGA color palette:
- 0: Black (#000000)
- 1: Red (#aa0000)
- 2: Green (#00aa00)
- 3: Yellow/Brown (#aa5500)
- 4: Blue (#0000aa)
- 5: Magenta (#aa00aa)
- 6: Cyan (#00aaaa)
- 7: White/Light Gray (#aaaaaa)
- 8: Bright Black/Dark Gray (#555555)
- 9: Bright Red (#ff5555)
- 10: Bright Green (#55ff55)
- 11: Bright Yellow (#ffff55)
- 12: Bright Blue (#5555ff)
- 13: Bright Magenta (#ff55ff)
- 14: Bright Cyan (#55ffff)
- 15: Bright White (#ffffff)

## CSS Animation Examples

Each character can be animated using CSS or JavaScript:

```css
/* Fade in characters sequentially */
@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}

pre.ansi-art span {
  animation: fadeIn 0.1s ease-in;
  animation-fill-mode: backwards;
}

/* Delay based on data-id */
pre.ansi-art span[data-id="0"] { animation-delay: 0ms; }
pre.ansi-art span[data-id="1"] { animation-delay: 10ms; }
/* ... or use JavaScript to set delays dynamically */
```

```javascript
// Typewriter effect
document.querySelectorAll('pre.ansi-art span').forEach((span, index) => {
  span.style.animationDelay = (index * 10) + 'ms';
});

// Wave effect based on row
document.querySelectorAll('pre.ansi-art span').forEach(span => {
  const row = parseInt(span.dataset.row);
  span.style.animationDelay = (row * 50) + 'ms';
});

// Glitch effect on specific rows
document.querySelectorAll('pre.ansi-art span[data-row="10"]').forEach(span => {
  span.style.animation = 'glitch 0.3s infinite';
});
```

## Supported ANSI Codes

- **SGR (Select Graphic Rendition)**
  - `0m` - Reset all attributes
  - `1m` - Bold
  - `22m` - Normal intensity
  - `30-37m` - Foreground colors
  - `39m` - Default foreground
  - `40-47m` - Background colors
  - `49m` - Default background
  - `90-97m` - Bright foreground colors
  - `100-107m` - Bright background colors
  - `38;2;R;G;Bm` - 24-bit foreground (with `--truecolor`)
  - `48;2;R;G;Bm` - 24-bit background (with `--truecolor`)

- **Cursor Movement**
  - `A` - Cursor up
  - `B` - Cursor down
  - `C` - Cursor forward
  - `D` - Cursor back
  - `H` / `f` - Cursor position

## License

This tool is open source. Font files in `old_fonts` have their own licenses - see LICENSE.TXT in that directory.

## Requirements

- C compiler (GCC, Clang, etc.)
- Font files in `old_fonts/ttf - Px (pixel outline)/` directory
- Web browser with @font-face support

## Notes

- Input is read from stdin, output to stdout
- Auto-detects dimensions from ANSI content
- Handles CP437 extended ASCII correctly
- Preserves exact layout and colors from original ANSI art
- Works with ANSI files from BBSes, demos, and art collections
