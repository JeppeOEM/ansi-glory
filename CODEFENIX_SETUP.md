# CODEFENIX GINGER - Setup & Usage

## What Was Done

✅ Converted `CODEFENIX-GINGER.ANS` (16.8KB) to HTML web component
✅ Created professional demo page (`index.html`) with font switcher
✅ Removed all old test files (13 files cleaned up)
✅ Created embedded web component (1.2MB, self-contained)

## Files

### Main Files
- **`index.html`** (7.6KB) - Beautiful viewer page with font selector
- **`codefenix-ginger.html`** (1.2MB) - ANSI art as web component
- **`CODEFENIX-GINGER.ANS`** (16.8KB) - Original ANSI source file

### What Was Removed
- test_*.ans (all 5 test ANSI files)
- test*.html (all 5 test HTML components)
- demo.html, embed_demo.html (old demo pages)
- test.html (old test page)

## How to Use

### View the Artwork
```bash
# Open in your browser
open index.html
# or
firefox index.html
```

### Font Switching
The page includes a dropdown with 6 fonts:
- IBM VGA 8x16 (Default)
- IBM VGA 9x16
- IBM EGA 8x14
- IBM BIOS
- IBM CGA
- DOS-V Japanese

Fonts switch in real-time without reloading!

## Component Details

**Artwork Dimensions:** 169×99 characters
**Component Size:** 1.2MB (self-contained)
**Format:** Web component (`<ansi-art>` element)
**Features:**
- Pixel-perfect absolute positioning
- Embedded CSS and JavaScript
- 6 fonts with runtime switching
- Self-contained (no external dependencies)

## Architecture

### index.html
- Beautiful gradient UI
- Font selector dropdown
- Loads component via fetch
- Handles font switching

### codefenix-ginger.html
- Complete `<ansi-art>` web component
- Embedded CSS (color palette, positioning)
- All character spans with data attributes
- Font switching JavaScript

### CODEFENIX-GINGER.ANS
- Original ANSI art file
- 169 columns × 99 rows
- Can regenerate component with: `./ansi2html < CODEFENIX-GINGER.ANS > codefenix-ginger.html`

## Regenerating the Component

If you modify the ANSI file or want to regenerate:

```bash
./ansi2html < CODEFENIX-GINGER.ANS > codefenix-ginger.html
```

The generated component will work with the existing index.html automatically.

## Browser Support

✅ Chrome 55+
✅ Firefox 52+
✅ Safari 11+
✅ Edge 15+
✅ Modern mobile browsers

❌ IE11 (not supported)

## Tips

- Component is large (1.2MB) but loads quickly in modern browsers
- Gzip compression reduces it to ~50KB for transfer
- Font switching is instant (no page reload needed)
- Works offline (all data embedded)

## Git Status

Latest commit: `898b40d`
- Converted CODEFENIX-GINGER.ANS to component
- Removed 13 old test files
- Created professional viewer page
- Ready for production

## Next Steps

You can now:
1. **View the artwork:** Open index.html in browser
2. **Customize styling:** Edit the CSS in index.html
3. **Add more artwork:** Generate components from other ANSI files
4. **Deploy online:** Upload to web server

---

**Setup Date:** January 25, 2026
**Status:** Production Ready
