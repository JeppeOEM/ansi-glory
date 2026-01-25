/*
 * ansi2html - Convert ANSI art files to HTML with CSS
 * 
 * Usage: ansi2html [options] < input.ans > output.html
 * 
 * Options:
 *   -t, --truecolor    Enable 24-bit true color support
 *   -h, --help         Show help message
 * 
 * Each character is wrapped in a span with:
 *   - data-id: unique sequential identifier
 *   - data-row: row number (0-based)
 *   - data-col: column number (0-based)
 *   - class: color and style classes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define INITIAL_ROWS 256
#define INITIAL_COLS 256
#define MAX_ESC_PARAMS 16
#define INPUT_BUFFER_SIZE (1024 * 1024)  /* 1MB initial buffer */

/* Cell attributes */
typedef struct {
    unsigned int codepoint;       /* Unicode codepoint (0 = empty/space) */
    int fg;                       /* Foreground color (-1 = default, 0-15 = standard, 256+ = RGB) */
    int bg;                       /* Background color (-1 = default, 0-15 = standard, 256+ = RGB) */
    bool bold;                    /* Bold attribute */
    bool has_content;             /* Whether this cell has been written to */
} Cell;

/* Screen buffer */
typedef struct {
    Cell **cells;
    int rows;
    int cols;
    int max_row;                /* Highest row with content */
    int max_col;                /* Highest column with content */
} Screen;

/* Current cursor and attribute state */
typedef struct {
    int row;
    int col;
    int fg;
    int bg;
    bool bold;
} State;

/* Global options */
static bool opt_truecolor = false;

/* Standard ANSI colors (CGA palette) */
static const char *ansi_colors[16] = {
    "#000000",  /* 0: Black */
    "#aa0000",  /* 1: Red */
    "#00aa00",  /* 2: Green */
    "#aa5500",  /* 3: Yellow/Brown */
    "#0000aa",  /* 4: Blue */
    "#aa00aa",  /* 5: Magenta */
    "#00aaaa",  /* 6: Cyan */
    "#aaaaaa",  /* 7: White/Light Gray */
    "#555555",  /* 8: Bright Black/Dark Gray */
    "#ff5555",  /* 9: Bright Red */
    "#55ff55",  /* 10: Bright Green */
    "#ffff55",  /* 11: Bright Yellow */
    "#5555ff",  /* 12: Bright Blue */
    "#ff55ff",  /* 13: Bright Magenta */
    "#55ffff",  /* 14: Bright Cyan */
    "#ffffff",  /* 15: Bright White */
};

/* Font metadata with pixel dimensions and aspect ratio */
typedef struct {
    const char *name;           /* Font name */
    int width;                  /* Character width in pixels */
    int height;                 /* Character height in pixels */
    float aspect_ratio;         /* width/height ratio */
} FontMetadata;

static const FontMetadata font_info[] = {
    {"Px437_IBM_VGA_8x16",   8,  16, 0.5f},      /* 2:1 aspect (DOS standard) */
    {"Px437_IBM_VGA_9x16",   9,  16, 0.5625f},   /* 1.78:1 aspect, needs correction to 2:1 */
    {"Px437_IBM_EGA_8x14",   8,  14, 0.5714f},   /* 1.75:1 aspect, needs correction to 2:1 */
    {"Px437_IBM_BIOS",       8,  16, 0.5f},      /* 2:1 aspect */
    {"Px437_IBM_CGA",        8,  16, 0.5f},      /* 2:1 aspect */
    {"Px437_DOS-V_re_JPN12", 10, 20, 0.5f},      /* 2:1 aspect */
    {NULL, 0, 0, 0}
};

/* Legacy font list for compatibility */
static const char *font_list[] = {
    "Px437_IBM_VGA_8x16",
    "Px437_IBM_VGA_9x16",
    "Px437_IBM_EGA_8x14",
    "Px437_IBM_BIOS",
    "Px437_IBM_CGA",
    "Px437_DOS-V_re_JPN12",
    NULL
};

/* CP437 to Unicode mapping for characters 128-255 */
static const unsigned short cp437_to_unicode[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

/* Function prototypes */
static Screen *screen_create(int rows, int cols);
static void screen_free(Screen *screen);
static void screen_ensure_size(Screen *screen, int row, int col);
static void screen_put_char(Screen *screen, State *state, unsigned int codepoint);
static void state_reset(State *state);
static unsigned char *read_all_stdin(size_t *out_size);
static int convert_256_to_rgb(int n);
static void parse_ansi(Screen *screen, State *state, const unsigned char *data, size_t size);
static void output_html(Screen *screen);
static void print_usage(const char *prog);

/* Create a new screen buffer */
static Screen *screen_create(int rows, int cols) {
    Screen *screen = malloc(sizeof(Screen));
    if (!screen) return NULL;
    
    screen->rows = rows;
    screen->cols = cols;
    screen->max_row = -1;
    screen->max_col = -1;
    
    screen->cells = malloc(rows * sizeof(Cell *));
    if (!screen->cells) {
        free(screen);
        return NULL;
    }
    
    for (int r = 0; r < rows; r++) {
        screen->cells[r] = calloc(cols, sizeof(Cell));
        if (!screen->cells[r]) {
            for (int i = 0; i < r; i++) free(screen->cells[i]);
            free(screen->cells);
            free(screen);
            return NULL;
        }
        /* Initialize cells */
        for (int c = 0; c < cols; c++) {
            screen->cells[r][c].codepoint = ' ';
            screen->cells[r][c].fg = 7;  /* Default light gray */
            screen->cells[r][c].bg = 0;  /* Default black */
            screen->cells[r][c].bold = false;
            screen->cells[r][c].has_content = false;
        }
    }
    
    return screen;
}

/* Free screen buffer */
static void screen_free(Screen *screen) {
    if (!screen) return;
    for (int r = 0; r < screen->rows; r++) {
        free(screen->cells[r]);
    }
    free(screen->cells);
    free(screen);
}

/* Ensure screen is large enough for given position */
static void screen_ensure_size(Screen *screen, int row, int col) {
    /* Expand rows if needed */
    if (row >= screen->rows) {
        int new_rows = screen->rows;
        while (new_rows <= row) new_rows *= 2;
        
        Cell **new_cells = realloc(screen->cells, new_rows * sizeof(Cell *));
        if (!new_cells) return;
        screen->cells = new_cells;
        
        for (int r = screen->rows; r < new_rows; r++) {
            screen->cells[r] = calloc(screen->cols, sizeof(Cell));
            if (!screen->cells[r]) return;
            for (int c = 0; c < screen->cols; c++) {
                screen->cells[r][c].codepoint = ' ';
                screen->cells[r][c].fg = 7;
                screen->cells[r][c].bg = 0;
                screen->cells[r][c].bold = false;
                screen->cells[r][c].has_content = false;
            }
        }
        screen->rows = new_rows;
    }
    
    /* Expand columns if needed */
    if (col >= screen->cols) {
        int new_cols = screen->cols;
        while (new_cols <= col) new_cols *= 2;
        
        for (int r = 0; r < screen->rows; r++) {
            Cell *new_row = realloc(screen->cells[r], new_cols * sizeof(Cell));
            if (!new_row) return;
            screen->cells[r] = new_row;
            
            for (int c = screen->cols; c < new_cols; c++) {
                screen->cells[r][c].codepoint = ' ';
                screen->cells[r][c].fg = 7;
                screen->cells[r][c].bg = 0;
                screen->cells[r][c].bold = false;
                screen->cells[r][c].has_content = false;
            }
        }
        screen->cols = new_cols;
    }
}

/* Put a character at current cursor position */
static void screen_put_char(Screen *screen, State *state, unsigned int codepoint) {
    screen_ensure_size(screen, state->row, state->col);
    
    Cell *cell = &screen->cells[state->row][state->col];
    cell->codepoint = codepoint;
    cell->fg = state->fg;
    cell->bg = state->bg;
    cell->bold = state->bold;
    cell->has_content = true;
    
    /* Track max dimensions */
    if (state->row > screen->max_row) screen->max_row = state->row;
    if (state->col > screen->max_col) screen->max_col = state->col;
    
    /* Advance cursor */
    state->col++;
}

/* Reset state to defaults */
static void state_reset(State *state) {
    state->fg = 7;   /* Light gray */
    state->bg = 0;   /* Black */
    state->bold = false;
}

/* Get UTF-8 character length from first byte */
static int utf8_char_len(unsigned char first_byte) {
    if ((first_byte & 0x80) == 0) return 1;      /* ASCII: 0xxxxxxx */
    if ((first_byte & 0xE0) == 0xC0) return 2;   /* 110xxxxx */
    if ((first_byte & 0xF0) == 0xE0) return 3;   /* 1110xxxx */
    if ((first_byte & 0xF8) == 0xF0) return 4;   /* 11110xxx */
    return 1;  /* Invalid UTF-8, treat as single byte */
}

/* Decode UTF-8 character to Unicode codepoint */
static unsigned int utf8_decode(const unsigned char *bytes, int len) {
    if (len == 1) {
        return bytes[0];
    } else if (len == 2) {
        return ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
    } else if (len == 3) {
        return ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
    } else if (len == 4) {
        return ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) | 
               ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
    }
    return 0;
}

/* Read all data from stdin */
static unsigned char *read_all_stdin(size_t *out_size) {
    size_t capacity = INPUT_BUFFER_SIZE;
    size_t size = 0;
    unsigned char *buffer = malloc(capacity);
    if (!buffer) return NULL;
    
    int ch;
    while ((ch = getchar()) != EOF) {
        if (size >= capacity) {
            capacity *= 2;
            unsigned char *new_buf = realloc(buffer, capacity);
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }
        buffer[size++] = (unsigned char)ch;
    }
    
    *out_size = size;
    return buffer;
}

/* Convert 256-color index to RGB value */
static int convert_256_to_rgb(int n) {
    if (n < 16) {
        /* Standard colors - shouldn't reach here */
        return 0;
    } else if (n < 232) {
        /* 216 color cube: 6x6x6 */
        n -= 16;
        int r = (n / 36) * 51;
        int g = ((n / 6) % 6) * 51;
        int b = (n % 6) * 51;
        return (r << 16) | (g << 8) | b;
    } else {
        /* Grayscale: 24 shades */
        int gray = (n - 232) * 10 + 8;
        return (gray << 16) | (gray << 8) | gray;
    }
}

/* Parse ANSI escape sequences and populate screen buffer */
static void parse_ansi(Screen *screen, State *state, const unsigned char *data, size_t size) {
    size_t i = 0;
    
    while (i < size) {
        unsigned char ch = data[i];
        
        /* Check for ESC character */
        if (ch == 0x1B && i + 1 < size && data[i + 1] == '[') {
            /* CSI sequence */
            i += 2;
            
            /* Parse parameters */
            int params[MAX_ESC_PARAMS];
            int param_count = 0;
            int current_param = 0;
            bool has_param = false;
            
            while (i < size) {
                ch = data[i];
                
                if (ch >= '0' && ch <= '9') {
                    current_param = current_param * 10 + (ch - '0');
                    has_param = true;
                    i++;
                } else if (ch == ';') {
                    if (param_count < MAX_ESC_PARAMS) {
                        params[param_count++] = has_param ? current_param : 0;
                    }
                    current_param = 0;
                    has_param = false;
                    i++;
                } else if (ch >= 0x40 && ch <= 0x7E) {
                    /* Final byte - end of sequence */
                    if (has_param && param_count < MAX_ESC_PARAMS) {
                        params[param_count++] = current_param;
                    }
                    i++;
                    
                    /* Process command */
                    switch (ch) {
                        case 'A':  /* Cursor Up */
                            {
                                int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                                state->row -= n;
                                if (state->row < 0) state->row = 0;
                            }
                            break;
                            
                        case 'B':  /* Cursor Down */
                            {
                                int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                                state->row += n;
                            }
                            break;
                            
                        case 'C':  /* Cursor Forward */
                            {
                                int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                                state->col += n;
                            }
                            break;
                            
                        case 'D':  /* Cursor Back */
                            {
                                int n = (param_count > 0 && params[0] > 0) ? params[0] : 1;
                                state->col -= n;
                                if (state->col < 0) state->col = 0;
                            }
                            break;
                            
                        case 'H':  /* Cursor Position */
                        case 'f':
                            {
                                int row = (param_count > 0 && params[0] > 0) ? params[0] - 1 : 0;
                                int col = (param_count > 1 && params[1] > 0) ? params[1] - 1 : 0;
                                state->row = row;
                                state->col = col;
                            }
                            break;
                            
                        case 'J':  /* Erase in Display - ignore for now */
                        case 'K':  /* Erase in Line - ignore for now */
                            break;
                            
                        case 'm':  /* SGR - Select Graphic Rendition */
                            if (param_count == 0) {
                                state_reset(state);
                            } else {
                                for (int p = 0; p < param_count; p++) {
                                    int code = params[p];
                                    
                                    if (code == 0) {
                                        state_reset(state);
                                    } else if (code == 1) {
                                        state->bold = true;
                                    } else if (code == 22) {
                                        state->bold = false;
                                    } else if (code >= 30 && code <= 37) {
                                        /* Standard foreground colors */
                                        state->fg = code - 30;
                                        if (state->bold) state->fg += 8;
                                    } else if (code == 39) {
                                        /* Default foreground */
                                        state->fg = 7;
                                    } else if (code >= 40 && code <= 47) {
                                        /* Standard background colors */
                                        state->bg = code - 40;
                                    } else if (code == 49) {
                                        /* Default background */
                                        state->bg = 0;
                                    } else if (code >= 90 && code <= 97) {
                                        /* Bright foreground colors */
                                        state->fg = code - 90 + 8;
                                    } else if (code >= 100 && code <= 107) {
                                        /* Bright background colors */
                                        state->bg = code - 100 + 8;
                                    } else if (code == 38 && opt_truecolor) {
                                        /* Extended foreground color */
                                        if (p + 1 < param_count && params[p + 1] == 2) {
                                            /* True color: 38;2;R;G;B */
                                            if (p + 4 < param_count) {
                                                int r = params[p + 2];
                                                int g = params[p + 3];
                                                int b = params[p + 4];
                                                state->fg = 0x1000000 | (r << 16) | (g << 8) | b;
                                                p += 4;
                                            }
                                        } else if (p + 1 < param_count && params[p + 1] == 5) {
                                            /* 256 color: 38;5;N */
                                            if (p + 2 < param_count) {
                                                int n = params[p + 2];
                                                if (n < 16) {
                                                    state->fg = n;
                                                } else {
                                                    /* Convert 256 color to RGB */
                                                    state->fg = 0x1000000 | convert_256_to_rgb(n);
                                                }
                                                p += 2;
                                            }
                                        }
                                    } else if (code == 48 && opt_truecolor) {
                                        /* Extended background color */
                                        if (p + 1 < param_count && params[p + 1] == 2) {
                                            /* True color: 48;2;R;G;B */
                                            if (p + 4 < param_count) {
                                                int r = params[p + 2];
                                                int g = params[p + 3];
                                                int b = params[p + 4];
                                                state->bg = 0x1000000 | (r << 16) | (g << 8) | b;
                                                p += 4;
                                            }
                                        } else if (p + 1 < param_count && params[p + 1] == 5) {
                                            /* 256 color: 48;5;N */
                                            if (p + 2 < param_count) {
                                                int n = params[p + 2];
                                                if (n < 16) {
                                                    state->bg = n;
                                                } else {
                                                    state->bg = 0x1000000 | convert_256_to_rgb(n);
                                                }
                                                p += 2;
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                            
                        case 's':  /* Save cursor position - ignore */
                        case 'u':  /* Restore cursor position - ignore */
                            break;
                            
                        default:
                            /* Unknown command, ignore */
                            break;
                    }
                    break;
                } else {
                    /* Invalid character in sequence, abort */
                    break;
                }
            }
        } else if (ch == '\n') {
            /* Newline handling - check for \r\n and \n\r sequences */
            if (i + 1 < size && data[i + 1] == '\r') {
                /* \n\r sequence: move to next row, reset column */
                state->row++;
                state->col = 0;
                i += 2;
            } else {
                /* Just \n: move to next row, reset column */
                state->row++;
                state->col = 0;
                i++;
            }
        } else if (ch == '\r') {
            /* Carriage return handling - check for \r\n sequence */
            if (i + 1 < size && data[i + 1] == '\n') {
                /* \r\n sequence: move to next row, reset column */
                state->row++;
                state->col = 0;
                i += 2;
            } else {
                /* Just \r: carriage return only, reset column and allow overwrite */
                state->row++;
                state->col = 0;
                i++;
            }
        } else if (ch == '\t') {
            /* Tab - move to next 8-column boundary */
            state->col = (state->col + 8) & ~7;
            i++;
        } else if (ch >= 32 && ch < 127) {
            /* ASCII printable character */
            screen_put_char(screen, state, ch);
            i++;
        } else if ((ch & 0x80) != 0) {
            /* Potential UTF-8 sequence */
            int utf8_len = utf8_char_len(ch);
            if (i + utf8_len <= size) {
                /* Valid UTF-8 sequence within bounds */
                unsigned int codepoint = utf8_decode(&data[i], utf8_len);
                screen_put_char(screen, state, codepoint);
                i += utf8_len;
            } else {
                /* Truncated UTF-8 sequence - skip it */
                i++;
            }
        } else {
            /* Other control characters - skip */
            i++;
        }
    }
}

/* Output HTML entity for special characters */
static void output_char_escaped(unsigned int codepoint) {
    switch (codepoint) {
        case '<':  printf("&lt;"); break;
        case '>':  printf("&gt;"); break;
        case '&':  printf("&amp;"); break;
        case '"':  printf("&quot;"); break;
        case ' ':  printf(" "); break;  /* Regular space */
        case 0:    printf(" "); break;  /* Null -> space */
        default:
            if (codepoint >= 32 && codepoint < 127) {
                putchar(codepoint);
            } else if (codepoint < 256) {
                /* Extended ASCII/other characters - output as HTML numeric entity */
                printf("&#%u;", codepoint);
            } else {
                /* High Unicode - output as hex entity */
                printf("&#x%X;", codepoint);
            }
            break;
    }
}

/* Generate font-specific CSS with proper line heights for 2:1 DOS aspect ratio */
static void generate_font_css(void) {
    /* DOS terminal standard: 2:1 aspect ratio (char height = 2 * char width) */
    
    for (int i = 0; font_info[i].name != NULL; i++) {
        /* Calculate line-height to maintain 2:1 aspect ratio */
        /* Formula: target_aspect / current_aspect */
        /* target_aspect = 2.0 (DOS standard) */
        /* current_aspect = height / width */
        float current_aspect = (float)font_info[i].height / font_info[i].width;
        float target_aspect = 2.0f;
        float line_height = target_aspect / current_aspect;
        
        printf("/* %s: %dx%d pixels, aspect correction: %.3f */\n",
               font_info[i].name, font_info[i].width, font_info[i].height, line_height);
        printf(".font-%s-lh { line-height: %.4f; }\n", font_info[i].name, line_height);
    }
}

/* Generate list of available fonts */
static void generate_font_list(void) {
    /* This will be called to generate font @font-face declarations */
    for (int i = 0; font_list[i] != NULL; i++) {
        printf("@font-face {\n");
        printf("  font-family: '%s';\n", font_list[i]);
        printf("  src: url('old_fonts/ttf - Px (pixel outline)/%s.ttf') format('truetype');\n", font_list[i]);
        printf("  font-weight: normal;\n");
        printf("  font-style: normal;\n");
        printf("}\n");
    }
}

/* Output the HTML document */
static void output_html(Screen *screen) {
    int total_rows = screen->max_row + 1;
    int total_cols = screen->max_col + 1;
    
    if (total_rows <= 0 || total_cols <= 0) {
        total_rows = 1;
        total_cols = 1;
    }
    
     /* HTML header */
     printf("<!DOCTYPE html>\n");
     printf("<html>\n");
     printf("<head>\n");
     printf("<meta charset=\"UTF-8\">\n");
     printf("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
     printf("<title>ANSI Art</title>\n");
     printf("<!-- ANSI Art Metadata -->\n");
     printf("<!-- Width: %d characters -->\n", total_cols);
     printf("<!-- Height: %d lines -->\n", total_rows);
     printf("<!-- Font Size: 16px (Px437 fonts are pixel-perfect) -->\n");
     printf("<!-- Line Height: 1.0 (terminal-accurate) -->\n");
     printf("<!-- Character Grid: Monospace 1:1 cell ratio -->\n");
     printf("<style>\n");
     
     /* Global CSS reset - remove all browser defaults */
      printf("/* Global reset - remove all browser defaults */\n");
      printf("* {\n");
      printf("  margin: 0;\n");
      printf("  padding: 0;\n");
      printf("  border: 0;\n");
      printf("  box-sizing: border-box;\n");
      printf("  text-decoration: none;\n");
      printf("  text-transform: none;\n");
      printf("  font-style: normal;\n");
      printf("  font-weight: normal;\n");
      printf("  font-variant: normal;\n");
      printf("  font-variant-ligatures: none;\n");
      printf("  font-kerning: none;\n");
      printf("  line-height: 1;\n");
      printf("  text-rendering: geometricPrecision;\n");
      printf("  -webkit-font-smoothing: none;\n");
      printf("  -moz-osx-font-smoothing: auto;\n");
      printf("}\n");
     
      /* Font declarations */
      generate_font_list();
      
      /* Font-specific line heights for 2:1 DOS aspect ratio */
      printf("\n/* Font-specific line heights for DOS 2:1 aspect ratio */\n");
      generate_font_css();
    
     /* Base styles */
     printf("body {\n");
     printf("  background-color: #000000;\n");
     printf("  margin: 0;\n");
     printf("  padding: 0;\n");
     printf("  color: #aaaaaa;\n");
     printf("  font-family: monospace;\n");
     printf("}\n");
    printf(".controls {\n");
    printf("  background-color: #1a1a1a;\n");
    printf("  padding: 10px 20px;\n");
    printf("  border-bottom: 1px solid #333;\n");
    printf("  display: flex;\n");
    printf("  align-items: center;\n");
    printf("  gap: 10px;\n");
    printf("}\n");
    printf(".controls label {\n");
    printf("  margin: 0;\n");
    printf("}\n");
    printf(".controls select {\n");
    printf("  background-color: #2a2a2a;\n");
    printf("  color: #aaaaaa;\n");
    printf("  border: 1px solid #444;\n");
    printf("  padding: 5px 10px;\n");
    printf("  border-radius: 3px;\n");
    printf("  font-size: 14px;\n");
    printf("}\n");
     printf(".content {\n");
     printf("  padding: 0;\n");
     printf("  overflow: auto;\n");
     printf("}\n");
      printf("pre.ansi-art {\n");
      printf("  font-family: '%s', 'Courier New', monospace;\n", font_list[0]);
      printf("  font-size: 16px;\n");
      printf("  line-height: 1.0;\n");
      printf("  margin: 0;\n");
      printf("  padding: 0;\n");
      printf("  letter-spacing: 0;\n");
      printf("  word-spacing: 0;\n");
      printf("  -webkit-text-rendering: geometricPrecision;\n");
      printf("  text-rendering: pixelated;\n");
      printf("  text-size-adjust: 100%%;\n");
      printf("  -webkit-text-size-adjust: 100%%;\n");
      printf("  font-variant-ligatures: none;\n");
      printf("  font-kerning: none;\n");
      printf("  font-feature-settings: 'kern' 0, 'liga' 0;\n");
      printf("  -webkit-font-smoothing: none;\n");
      printf("  -moz-osx-font-smoothing: grayscale;\n");
      printf("  image-rendering: pixelated;\n");
      printf("  image-rendering: crisp-edges;\n");
      printf("  white-space: pre;\n");
      printf("  word-wrap: normal;\n");
      printf("  display: block;\n");
      printf("}\n");
      printf("pre.ansi-art span {\n");
      printf("  font-family: inherit;\n");
      printf("  font-size: inherit;\n");
      printf("  line-height: inherit;\n");
      printf("  letter-spacing: inherit;\n");
      printf("  word-spacing: inherit;\n");
      printf("  -webkit-text-rendering: geometricPrecision;\n");
      printf("  text-rendering: pixelated;\n");
      printf("  font-variant-ligatures: none;\n");
      printf("  font-kerning: none;\n");
      printf("  font-feature-settings: 'kern' 0, 'liga' 0;\n");
      printf("  -webkit-font-smoothing: none;\n");
      printf("  text-decoration: none;\n");
      printf("  display: inline;\n");
      printf("  margin: 0;\n");
      printf("  padding: 0;\n");
      printf("  border: 0;\n");
     printf("}\n");
    
    /* Color classes */
    for (int i = 0; i < 16; i++) {
        printf(".fg-%d { color: %s; }\n", i, ansi_colors[i]);
    }
    for (int i = 0; i < 16; i++) {
        printf(".bg-%d { background-color: %s; }\n", i, ansi_colors[i]);
    }
    printf(".bold { font-weight: bold; }\n");
    
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<div class=\"controls\">\n");
    printf("  <label for=\"font-select\">Font:</label>\n");
    printf("  <select id=\"font-select\" onchange=\"changeFont(this.value)\">\n");
    for (int i = 0; font_list[i] != NULL; i++) {
        printf("    <option value=\"%s\"%s>%s</option>\n", 
               font_list[i], i == 0 ? " selected" : "", font_list[i]);
    }
    printf("  </select>\n");
    printf("</div>\n");
     printf("<div class=\"content\">\n");
     printf("<pre class=\"ansi-art\" id=\"ansi-art\">");
     
      /* Output characters with trailing whitespace trimming */
      int char_id = 0;
      for (int row = 0; row < total_rows; row++) {
          /* Calculate actual content end for this row */
          int row_content_end = 0;
          
          /* Find RIGHTMOST non-space content (regardless of styling) */
          for (int col = total_cols - 1; col >= 0; col--) {
              Cell *cell = &screen->cells[row][col];
              if (cell->codepoint != 0 && cell->codepoint != ' ') {
                  row_content_end = col + 1;
                  break;
              }
          }
          
          /* ALSO check for any cells with explicit styling (non-default colors) */
          for (int col = 0; col < total_cols; col++) {
              Cell *cell = &screen->cells[row][col];
              /* Include if it has non-default styling */
              if ((cell->fg >= 0 && cell->fg < 16 && cell->fg != 7) ||
                  (cell->bg >= 0 && cell->bg != 0) ||
                  cell->bold) {
                  /* Update content end if this styled cell is past current end */
                  if (col + 1 > row_content_end) {
                      row_content_end = col + 1;
                  }
              }
          }
          
          /* Output up to content end */
         int output_cols = (row_content_end > 0) ? row_content_end : 1;
         
         for (int col = 0; col < output_cols; col++) {
             Cell *cell = &screen->cells[row][col];
             
             /* Build class string */
             char class_str[256] = "";
             int class_len = 0;
             
             if (cell->fg >= 0 && cell->fg < 16) {
                 class_len += snprintf(class_str + class_len, sizeof(class_str) - class_len, 
                                       "fg-%d", cell->fg);
             } else if (cell->fg >= 0x1000000) {
                 /* True color - will use inline style instead */
             }
             
             if (cell->bg >= 0 && cell->bg < 16 && cell->bg != 0) {
                 if (class_len > 0) class_str[class_len++] = ' ';
                 class_len += snprintf(class_str + class_len, sizeof(class_str) - class_len,
                                       "bg-%d", cell->bg);
             }
             
             if (cell->bold) {
                 if (class_len > 0) class_str[class_len++] = ' ';
                 class_len += snprintf(class_str + class_len, sizeof(class_str) - class_len, "bold");
             }
             
             /* Build inline style for true colors */
             char style_str[128] = "";
             if (opt_truecolor) {
                 int style_len = 0;
                 if (cell->fg >= 0x1000000) {
                     int rgb = cell->fg & 0xFFFFFF;
                     style_len += snprintf(style_str + style_len, sizeof(style_str) - style_len,
                                           "color:#%06x;", rgb);
                 }
                 if (cell->bg >= 0x1000000) {
                     int rgb = cell->bg & 0xFFFFFF;
                     style_len += snprintf(style_str + style_len, sizeof(style_str) - style_len,
                                           "background-color:#%06x;", rgb);
                 }
             }
             
             /* Output span */
             printf("<span data-id=\"%d\" data-row=\"%d\" data-col=\"%d\"",
                    char_id++, row, col);
             
             if (class_str[0]) {
                 printf(" class=\"%s\"", class_str);
             }
             if (style_str[0]) {
                 printf(" style=\"%s\"", style_str);
             }
              
              printf(">");
              output_char_escaped(cell->codepoint);
              printf("</span>");
         }
         /* End of row - add newline in output */
          printf("\n");
      }
     
     printf("</pre>\n");
     printf("</div>\n");
     printf("<script>\n");
     printf("// Font metadata for maintaining 2:1 DOS aspect ratio\n");
     printf("const fontAspectMap = {\n");
     printf("  'Px437_IBM_VGA_8x16': { w: 8, h: 16, lh: 1.0 },\n");
     printf("  'Px437_IBM_VGA_9x16': { w: 9, h: 16, lh: 0.8889 },\n");
     printf("  'Px437_IBM_EGA_8x14': { w: 8, h: 14, lh: 1.1429 },\n");
     printf("  'Px437_IBM_BIOS': { w: 8, h: 16, lh: 1.0 },\n");
     printf("  'Px437_IBM_CGA': { w: 8, h: 16, lh: 1.0 },\n");
     printf("  'Px437_DOS-V_re_JPN12': { w: 10, h: 20, lh: 1.0 }\n");
     printf("};\n");
     printf("function changeFont(fontName) {\n");
     printf("  const ansiArt = document.getElementById('ansi-art');\n");
     printf("  ansiArt.style.fontFamily = \"'\" + fontName + \"', 'Courier New', monospace\";\n");
     printf("  \n");
     printf("  // Adjust line-height to maintain 2:1 DOS aspect ratio\n");
     printf("  if (fontAspectMap[fontName]) {\n");
     printf("    const lineHeight = fontAspectMap[fontName].lh;\n");
     printf("    ansiArt.style.lineHeight = lineHeight;\n");
     printf("  } else {\n");
     printf("    ansiArt.style.lineHeight = '1.0';\n");
     printf("  }\n");
     printf("}\n");
     printf("</script>\n");
    printf("</body>\n");
    printf("</html>\n");
}

/* Print usage information */
static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] < input.ans > output.html\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Convert ANSI art files to HTML with CSS styling.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -t, --truecolor    Enable 24-bit true color support\n");
    fprintf(stderr, "  -h, --help         Show this help message\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Output format:\n");
    fprintf(stderr, "  Each character is wrapped in a <span> element with:\n");
    fprintf(stderr, "    data-id    - Unique sequential identifier (for animations)\n");
    fprintf(stderr, "    data-row   - Row number (0-based)\n");
    fprintf(stderr, "    data-col   - Column number (0-based)\n");
    fprintf(stderr, "    class      - CSS classes for colors (fg-N, bg-N, bold)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s < artwork.ans > artwork.html\n", prog);
    fprintf(stderr, "  %s --truecolor < modern.ans > modern.html\n", prog);
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--truecolor") == 0) {
            opt_truecolor = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    /* Read all input from stdin */
    size_t data_size;
    unsigned char *data = read_all_stdin(&data_size);
    if (!data) {
        fprintf(stderr, "Error: Failed to read input\n");
        return 1;
    }
    
    if (data_size == 0) {
        fprintf(stderr, "Error: No input data\n");
        free(data);
        return 1;
    }
    
    /* Create screen buffer */
    Screen *screen = screen_create(INITIAL_ROWS, INITIAL_COLS);
    if (!screen) {
        fprintf(stderr, "Error: Failed to allocate screen buffer\n");
        free(data);
        return 1;
    }
    
    /* Initialize state */
    State state = {0, 0, 7, 0, false};  /* row, col, fg, bg, bold */
    
    /* Parse ANSI data */
    parse_ansi(screen, &state, data, data_size);
    
    /* Output HTML */
    output_html(screen);
    
    /* Cleanup */
    screen_free(screen);
    free(data);
    
    return 0;
}
