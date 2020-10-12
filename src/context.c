#include "globals.h"
#include "util.h"

struct Commit
{
    color_t color[2];
    uint position;
    struct Commit *previous;
    struct Commit *next;
};


struct Context
{
    uint pixel_size;
    uint canvas_size;
    uint row_size;
    uint col_size;
    uint x_offset;
    uint y_offset;
    color_t pixels[SPRITE_CANVAS_SIZE];
    SDL_Rect rects[SPRITE_CANVAS_SIZE];
    bool has_indicator;
    SDL_Rect indicator;

    /**
     * is_transparent - do we render this context?
     */
    bool is_transparent;

    struct Commit *commit;
    signed int previous_direction;
};

Context_t Context_make(uint pixel_size, uint row_size, uint col_size, uint x_offset, uint y_offset)
{
    Context_t ctx = malloc(sizeof(struct Context));

    ctx->row_size = row_size;
    ctx->col_size = col_size;

    unsigned int i, j;
    unsigned int index = 0;
    for (i = 0; i < col_size; i++)
    {
        for (j = 0; j < row_size; j++)
        {
            ctx->pixels[index] = BLACK;
            ctx->rects[index].x = x_offset + j * pixel_size;
            ctx->rects[index].y = y_offset + i * pixel_size;
            ctx->rects[index].w = pixel_size;
            ctx->rects[index].h = pixel_size;
            index++;
        }
    }

    ctx->is_transparent = false;
    ctx->has_indicator = false;
    ctx->commit = NULL;
    ctx->previous_direction = 0;

    return ctx;
}

void Context_free(Context_t ctx)
{
    if (ctx->commit)
    {
        while (ctx->commit->previous != NULL)
            ctx->commit = ctx->commit->previous;

        Context_free_future_commits(ctx);
        free(ctx->commit);
        ctx->commit = NULL;
    }

    free(ctx);
    ctx = NULL;
}

void Context_render_sprite_in_context(Context_t ctx, Sprite_sheet_t sprite_sheet, uint index, uint context_index)
{
    Sprite_sheet_render_sprite(sprite_sheet, index, ctx->rects[context_index].x, ctx->rects[context_index].y);
}

void Context_make_transparent(Context_t ctx)
{
    ctx->is_transparent = true;
}

void Context_render(Context_t ctx)
{
    unsigned int i;

    if(!ctx->is_transparent)
    {
        for (i = 0; i < (ctx->row_size * ctx->col_size); i++)
        {
            set_pixel_render_color(ctx->pixels[i]);
            SDL_RenderFillRect(renderer, &ctx->rects[i]);
        }
    }

    if (!ctx->has_indicator) return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &ctx->indicator);
}

void Context_swap_pixels(Context_t dest, Context_t source)
{
    memcpy(dest->pixels, source->pixels, sizeof(dest->pixels));
}

void Context_indicator_focus(Context_t ctx, unsigned int rect_index)
{
    if (!ctx->has_indicator) return;
    memcpy(&ctx->indicator, &ctx->rects[rect_index], sizeof(SDL_Rect));
}


void Context_make_indicator(Context_t ctx)
{
    ctx->has_indicator = true;
    Context_indicator_focus(ctx, 0);
}


static int XYInRect(const SDL_Rect rect)
{
    return ((mouse.x >= rect.x && mouse.x <= rect.x + rect.w) && (mouse.y >= rect.y && mouse.y <= rect.y + rect.h));
}

void Context_handle_rect_click(Context_t ctx, void (*fn)(const unsigned int))
{
    unsigned int i;
    for (i = 0; i < (ctx->row_size * ctx->col_size); i++)
        if (XYInRect(ctx->rects[i]))
        {
            (*fn)(i);
            Context_indicator_focus(ctx, i);
        }
}

void Context_from_pixel_buffer(Context_t ctx, color_t *pixel_buffer)
{
    memcpy(ctx->pixels, pixel_buffer, sizeof(ctx->pixels));
}

void Context_to_pixel_buffer(Context_t ctx, color_t *pixel_buffer)
{
    memcpy(pixel_buffer, ctx->pixels, sizeof(ctx->pixels));
}

void Context_swap_rect_buffer(Context_t ctx, SDL_Rect *rect_buffer)
{
    memcpy(ctx->rects, rect_buffer, sizeof(ctx->rects));
}

color_t Context_get_pixel(Context_t ctx, const unsigned int pixel_index)
{
    return ctx->pixels[pixel_index];
}

void Context_set_pixel(Context_t ctx, const unsigned int pixel_index, color_t color)
{
    Context_new_commit(ctx, ctx->pixels[pixel_index], color, pixel_index);
    ctx->pixels[pixel_index] = color;
}

void Context_free_future_commits(Context_t ctx)
{
    if (ctx->commit == NULL || ctx->commit->next == NULL)
        return;

    struct Commit *commit_iterator = ctx->commit->next;
    while (commit_iterator != NULL)
    {
        struct Commit *commit_to_free = commit_iterator;
        commit_iterator = commit_iterator->next;
        free(commit_to_free);
    }
    ctx->commit->next = NULL;
}

void Context_new_commit(Context_t ctx, color_t pre_color, color_t post_color, uint position)
{
    if (pre_color == post_color)
        return;

    Context_free_future_commits(ctx);

    struct Commit *new_commit = (struct Commit *)malloc(sizeof(struct Commit));
    if (!new_commit)
        return;

    new_commit->color[0] = pre_color;
    new_commit->color[1] = post_color;
    new_commit->position = position;
    new_commit->previous = ctx->commit;
    new_commit->next = NULL;

    if (ctx->commit)
        ctx->commit->next = new_commit;

    ctx->commit = new_commit;
}

void Context_move_commits(Context_t ctx, int offset)
{
    if (offset == 0 || ctx->commit == NULL)
        return;

    signed int direction = -1 + (2 * (offset > 0));

    for (uint i = 0; i < direction * offset; i++)
    {

	if (direction == ctx->previous_direction && ctx->previous_direction != 0)
		if (direction < 0 && ctx->commit->previous != NULL)
		    ctx->commit = ctx->commit->previous;
		else if (direction > 0 && ctx->commit->next != NULL)
		    ctx->commit = ctx->commit->next;
		else
		    break;

        ctx->pixels[ctx->commit->position] = ctx->commit->color[direction > 0];
    }

    ctx->previous_direction = direction;
}

int Context_is_solid_color(Context_t ctx, color_t color) {
    for (int i = 0; i < SPRITE_CANVAS_SIZE; i += 1) {
        if (ctx->pixels[i] != color) {
            return 0;
        }
    }

    return 1;
}

