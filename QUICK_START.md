# ANSI2HTML Web Component - Quick Start Guide

## 30-Second Setup

```bash
# Convert an ANSI art file to a web component
./ansi2html < myart.ans > myart.html

# Open the component
cat myart.html | less
```

Copy the entire `<ansi-art>...</ansi-art>` block and paste into your HTML page. Done!

## Common Tasks

### View a Demo
```bash
# Open embed_demo.html in your browser to see:
# - 3 embedded ANSI art components
# - Global font switcher
# - Pixel-perfect rendering
open embed_demo.html
```

### Generate Components

#### Default (Web Component)
```bash
./ansi2html < artwork.ans > artwork.html
```
Creates a self-contained web component. Perfect for embedding.

#### Full Page with Controls
```bash
./ansi2html -p < artwork.ans > artwork_page.html
```
Creates a complete HTML page with font selector dropdown.

#### UTF-8 Encoding
```bash
./ansi2html --utf8 < artwork.ans > artwork.html
```
Uses UTF-8 encoding instead of HTML entity references.

#### True Color (24-bit)
```bash
./ansi2html --truecolor < artwork.ans > artwork.html
```
Enables 24-bit true color support for modern ANSI art.

### Embed Component in Your Site

1. Generate the component:
   ```bash
   ./ansi2html < myart.ans > myart.html
   ```

2. Open `myart.html` in a text editor and copy the `<ansi-art>...</ansi-art>` block

3. Paste into your HTML file:
   ```html
   <!DOCTYPE html>
   <html>
   <body>
     <h1>My ANSI Art Gallery</h1>
     
     <!-- Paste the component here -->
     <ansi-art data-width="80" data-height="25" ...>
       <style>...</style>
       <pre>...</pre>
       <script>...</script>
     </ansi-art>
     
   </body>
   </html>
   ```

4. Open in browser. That's it!

### Multiple Components on One Page

You can embed as many components as you want. Each is independent:

```html
<body>
  <h2>Component 1</h2>
  <ansi-art data-width="40" ...><!-- Component 1 --></ansi-art>
  
  <h2>Component 2</h2>
  <ansi-art data-width="80" ...><!-- Component 2 --></ansi-art>
  
  <h2>Component 3</h2>
  <ansi-art data-width="100" ...><!-- Component 3 --></ansi-art>
</body>
```

### Create a Font Switcher for All Components

Add this to your page (outside the components):

```html
<style>
  .font-switcher {
    padding: 20px;
    background: #f0f0f0;
    border-radius: 8px;
    margin-bottom: 20px;
  }
  
  .font-switcher select {
    padding: 8px 12px;
    font-size: 16px;
    border: 1px solid #ccc;
    border-radius: 4px;
  }
</style>

<div class="font-switcher">
  <label for="global-font">Font: </label>
  <select id="global-font" onchange="switchAllFonts(this.value)">
    <option value="Px437_IBM_VGA_8x16">IBM VGA 8x16 (Default)</option>
    <option value="Px437_IBM_VGA_9x16">IBM VGA 9x16</option>
    <option value="Px437_IBM_EGA_8x14">IBM EGA 8x14</option>
    <option value="Px437_IBM_BIOS">IBM BIOS</option>
    <option value="Px437_IBM_CGA">IBM CGA</option>
    <option value="Px437_DOS-V_re_JPN12">DOS-V Japanese</option>
  </select>
</div>

<!-- Your components here -->
<ansi-art data-width="80" ...>...</ansi-art>
<ansi-art data-width="80" ...>...</ansi-art>

<script>
function switchAllFonts(fontName) {
  // Get all components on the page
  const components = document.querySelectorAll('ansi-art');
  
  components.forEach(component => {
    // Each component has its own font switching method
    if (typeof component.switchFont === 'function') {
      component.switchFont(fontName);
    }
  });
}
</script>
```

See `embed_demo.html` for a complete working example!

## Available Fonts

| Font Name | Size | Description |
|-----------|------|-------------|
| `Px437_IBM_VGA_8x16` | 8×16px | Default, authentic VGA font |
| `Px437_IBM_VGA_9x16` | 9×16px | VGA variant, slightly wider |
| `Px437_IBM_EGA_8x14` | 8×14px | Older EGA font |
| `Px437_IBM_BIOS` | Variable | BIOS font variant |
| `Px437_IBM_CGA` | Variable | Classic CGA terminal font |
| `Px437_DOS-V_re_JPN12` | 12px | Japanese DOS font |

Fonts automatically adjust character positioning when switched.

## Troubleshooting

### Component doesn't appear
- Check browser console for errors (F12)
- Verify the HTML structure is intact
- Make sure the `<style>` and `<script>` tags are inside the `<ansi-art>` element

### Characters don't align perfectly
- The component uses absolute positioning
- Ensure the parent container doesn't have `transform: scale()` applied
- Check that CSS isn't overriding `position: absolute` on spans

### Font switching doesn't work
- Verify the font name exactly matches one of the 6 available fonts
- Check browser console for JavaScript errors
- The font switching is automatic - just change the dropdown value

### Colors look wrong
- Ensure your page doesn't have conflicting CSS color definitions
- The component uses scoped CSS prefixed with `ansi-art` to avoid conflicts
- Check that the ANSI source file has correct color codes

## Command Reference

```bash
# Basic conversion (web component)
./ansi2html < file.ans > file.html

# Full HTML page with controls
./ansi2html -p < file.ans > file.html

# UTF-8 encoding
./ansi2html --utf8 < file.ans > file.html

# True color (24-bit)
./ansi2html --truecolor < file.ans > file.html

# Combine flags
./ansi2html -p --utf8 < file.ans > file.html
./ansi2html --utf8 --truecolor < file.ans > file.html

# Show help
./ansi2html --help
```

## Examples

### Create a simple ANSI art gallery

```html
<!DOCTYPE html>
<html>
<head>
  <title>ANSI Art Gallery</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 1000px;
      margin: 0 auto;
      padding: 20px;
      background: #f5f5f5;
    }
    h1 { color: #333; }
    .gallery {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 20px;
      margin: 20px 0;
    }
    .artwork {
      background: white;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    }
    .artwork h3 { margin-top: 0; }
  </style>
</head>
<body>
  <h1>ANSI Art Gallery</h1>
  
  <div class="gallery">
    <div class="artwork">
      <h3>Classic Boxes</h3>
      <ansi-art data-width="40" ...><!-- Component 1 --></ansi-art>
    </div>
    
    <div class="artwork">
      <h3>Colors Demo</h3>
      <ansi-art data-width="40" ...><!-- Component 2 --></ansi-art>
    </div>
  </div>
</body>
</html>
```

## Performance

- **Generation**: <100ms per component
- **Font switching**: <20ms update per component
- **File size**: 13KB-390KB (depending on art size)
- **Memory**: 2-5MB per component in browser

## Next Steps

1. **View the demo**: Open `embed_demo.html` in your browser
2. **Read the README**: Full technical details in `README.md`
3. **Check the test report**: Validation details in `TEST_REPORT.md`
4. **Start converting**: Use the command reference above

## Need Help?

- **Technical details**: See `IMPLEMENTATION_SUMMARY.md`
- **Complete guide**: See `HOW_TO_USE.md`
- **Validation results**: See `TEST_REPORT.md`
- **Repository**: `/home/mrqdt/code/ansi2html/`

---

**Happy ANSI converting!** 🎨
