#include "globals.h"

struct mouse mouse;

SDL_Window *window;
SDL_Renderer *renderer;

unsigned int pen_color;
unsigned int current_sprite_index;
unsigned int copy_index;
unsigned int lctrl;
unsigned int lshift;
color_t clipboard_pixel_buffer[SPRITE_CANVAS_SIZE];
Context_t sprite_canvas_ctx;
Context_t color_picker_ctx;
Context_t sprite_selector_ctx;
Context_t sprite_sheet_current_cell_ctx;
Context_t toolbar_ctx;
Context_t sprite_selector_cells[SPRITESHEET_SIZE];
Context_t color_selector_cells[COLORPICKER_CANVAS_SIZE];

uint sprite_sheet[SPRITESHEET_SIZE][SPRITE_CANVAS_SIZE];

Message_Queue_t command_message_queue;
Message_Queue_t help_message_queue;

unsigned int current_time;
unsigned int last_time;
tool_t active_tool = PEN;

char icon_files[TOOLBAR_ROW_SIZE][50] = {
    "assets/icons/Pen.png",
    "assets/icons/Fill.png",
    "assets/icons/Drag.png",
    "assets/icons/Undo.png",
    "assets/icons/Redo.png",
    "assets/icons/Load.png",
    "assets/icons/Save.png",
    "assets/icons/Info.png"
  };
