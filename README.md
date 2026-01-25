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

### Basic Conversion (Web Component)
```bash
./ansi2html < artwork.ans > artwork.html
```
This generates a self-contained `<ansi-art>` web component with pixel-perfect absolute positioning.

### Full Page Output
```bash
./ansi2html -p < artwork.ans > artwork_page.html
```
This generates a complete HTML page with controls (font selector, etc.).

### With True Color Support
```bash
./ansi2html --truecolor < modern.ans > modern.html
```

### UTF-8 Output
```bash
./ansi2html --utf8 < artwork.ans > artwork.html
```

### Help
```bash
./ansi2html --help
```

## Output Format

### Web Component Output (Default)
The generated HTML is a self-contained `<ansi-art>` web component with absolute positioning for pixel-perfect rendering:

```html
<ansi-art data-width="80" data-height="25" data-font="Px437_IBM_VGA_8x16" data-char-width="8" data-char-height="16">
  <style>/* Scoped CSS with color definitions */</style>
  <pre style="position: relative; width: 640px; height: 400px;">
    <span style="position: absolute; left: 0px; top: 0px;" data-id="0" data-row="0" data-col="0" class="fg-7">A</span>
    <!-- More spans... -->
  </pre>
  <script>/* Font switching functionality */</script>
</ansi-art>
```

**Key Features:**
- ✅ Self-contained (CSS, HTML, JavaScript all embedded)
- ✅ Pixel-perfect rendering with absolute positioning
- ✅ Scoped CSS (won't conflict with page styles)
- ✅ Font switching support (6 fonts built-in)
- ✅ Data attributes for animation and scripting
- ✅ Multiple components can coexist on same page
- ✅ No external dependencies or fetch calls needed

### Full Page Output (With `-p` flag)
The full page output includes:
- Font selector dropdown at the top
- Styled container for the artwork
- Interactive font switching controls

### Character Span Attributes
Each character is wrapped in a `<span>` with these attributes:
- `data-id` - Unique sequential identifier (for animations)
- `data-row` - Row number (0-based)
- `data-col` - Column number (0-based)
- `class` - CSS classes (`fg-N`, `bg-N`, `bold`)

## Fonts

The HTML includes 6 embedded fonts that can be switched at runtime:

- **Px437_IBM_VGA_8x16** (Default) - 8×16 pixels, authentic VGA font
- **Px437_IBM_VGA_9x16** - 9×16 pixels, VGA variant
- **Px437_IBM_EGA_8x14** - 8×14 pixels, EGA font
- **Px437_IBM_BIOS** - BIOS variant
- **Px437_IBM_CGA** - CGA terminal font
- **Px437_DOS-V_re_JPN12** - Japanese DOS font variant

The web component automatically adjusts character positioning when fonts are switched, recalculating positions based on font dimensions.

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

## Embedding Web Components

### Quick Start
1. Generate an ANSI component: `./ansi2html < myart.ans > myart.html`
2. Open `myart.html` in a text editor and copy the entire `<ansi-art>...</ansi-art>` block
3. Paste it directly into your HTML page:

```html
<!DOCTYPE html>
<html>
<head>
    <title>My Page with ANSI Art</title>
</head>
<body>
    <h1>Welcome to my page</h1>
    
    <!-- Paste the ansi-art component here -->
    <ansi-art data-width="80" ...>
        <style>...</style>
        <pre>...</pre>
        <script>...</script>
    </ansi-art>
    
</body>
</html>
```

That's it! The component is completely self-contained and won't conflict with your page styles.

### Multiple Components on Same Page
You can embed multiple `<ansi-art>` components on the same page. Each one maintains its own styling and functionality independently.

### Viewing the Demo
Open `embed_demo.html` in your browser to see:
- 3 embedded ANSI art components
- Global font switcher affecting all components in real-time
- Pixel-perfect rendering with box-drawing characters
- Font switching with automatic position recalculation

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
