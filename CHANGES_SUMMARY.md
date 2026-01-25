# ansi2html - CSS/HTML Rendering Improvements

## Summary of Changes

All changes were made to the `output_html()` function in `ansi2html.c` to ensure deterministic, pixel-perfect rendering of ANSI art with no HTML/CSS interference.

## Changes Made

### 1. Added HTML Metadata Comments (Lines 547-550)
Metadata comments now appear after the `<title>` tag showing:
- Width: actual character width detected from ANSI file
- Height: actual line count from ANSI file
- Font Size: 16px (pixel-perfect Px437 fonts)
- Line Height: 1.0 (terminal-accurate 1:1 cell ratio)
- Character Grid: Monospace 1:1 cell ratio

**Example:**
```html
<!-- ANSI Art Metadata -->
<!-- Width: 80 characters -->
<!-- Height: 25 lines -->
<!-- Font Size: 16px (Px437 fonts are pixel-perfect) -->
<!-- Line Height: 1.0 (terminal-accurate) -->
<!-- Character Grid: Monospace 1:1 cell ratio -->
```

### 2. Added Global CSS Reset (Lines 551-565)
A comprehensive `*` selector rule that resets all browser defaults:
- `margin: 0` - No margins
- `padding: 0` - No padding
- `border: 0` - No borders
- `box-sizing: border-box` - Consistent box model
- `text-decoration: none` - No underlines/strikethrough
- `text-transform: none` - No text transformations
- `font-style: normal` - No italic
- `font-weight: normal` - No bold by default
- `font-variant: normal` - No variants
- **`font-variant-ligatures: none`** - **Disable font ligatures** (critical for ANSI art)
- `line-height: 1` - Reset line-height globally

### 3. Updated Body Font Family (Line 586)
Changed from `font-family: sans-serif;` to `font-family: monospace;`
- Ensures fallback to system monospace if custom fonts fail

### 4. Fixed .content Padding (Line 590)
Changed from `padding: 20px;` to `padding: 0;`
- Removes unwanted offset from the rendered artwork

### 5. Enhanced pre.ansi-art Styles (Lines 592-605)
Updated the pre element styling:
- **`line-height: 1.0`** (changed from `1.2`) - Terminal-accurate 1:1 cell ratio
- **Added `text-rendering: geometricPrecision`** - Pixel-perfect alignment
- **Added `font-variant-ligatures: none`** - Prevent font ligatures
- **Added `white-space: pre`** - Preserve exact spacing
- **Added `word-wrap: normal`** - Prevent word wrapping
- **Added `display: block`** - Ensure proper block-level rendering

### 6. Enhanced pre.ansi-art span Styles (Lines 606-613)
Updated the span element styling:
- **Added `text-rendering: geometricPrecision`** - Consistent rendering
- **Added `font-variant-ligatures: none`** - Prevent ligatures
- **Added `text-decoration: none`** - Explicit no decoration
- **Added `display: inline`** - Ensure inline behavior

## Technical Details

### Font Ligatures
Font ligatures are a typographic feature where multiple characters are rendered as a single glyph. Disabling them is critical for ANSI art because:
- Ligatures can change character widths (e.g., `fi` becomes a single narrow glyph)
- This breaks the strict 80-character (or custom width) grid
- The artwork becomes misaligned and distorted

### Line Height = 1.0
Standard terminals use a 1:1 aspect ratio for character cells. Setting `line-height: 1.0` ensures:
- Each line takes up exactly the height of one character
- No extra vertical spacing is introduced
- Visual appearance matches the original BBS/terminal rendering

### Text Rendering: geometricPrecision
This CSS property tells the browser to prioritize geometric accuracy over speed:
- Ensures pixel-perfect alignment
- Prevents browser font hinting that might distort spacing
- Critical for retro pixel fonts

### Global CSS Reset
The global `*` selector reset ensures:
- No inherited styles from parent elements interfere
- All elements start with a clean slate
- Deterministic rendering across all browsers

## Testing

Both test files produce correct metadata:
1. **CODEFENIX-GINGER.ANS**: 169 × 99 characters
2. **COLLY-newscole.ANS**: 238 × 749 characters

Each character has proper `data-row` and `data-col` attributes for grid positioning.

## Browser Compatibility

These CSS properties are supported in:
- All modern browsers (Chrome, Firefox, Safari, Edge)
- `text-rendering` is well-supported across all browsers
- `font-variant-ligatures` is CSS Fonts Module Level 3+ compatible
- Fallbacks to `font-family: monospace` work universally

## Result

ANSI art now renders:
- ✅ Deterministically (no browser variations)
- ✅ With exact character grid preservation
- ✅ With terminal-accurate line spacing
- ✅ Without ligature-induced distortions
- ✅ With pixel-perfect font rendering
- ✅ With metadata for validation
