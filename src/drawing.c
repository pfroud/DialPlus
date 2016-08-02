#include <pebble.h>
#include "main.h"
#include "drawing.h"
#include "handlers.h"


#define PX_PER_MINUTE 2
#define TICK_Y_START 86
#define TICK_WIDTH 2
#define TICK_HEIGHT_HOUR 21
#define TICK_HEIGHT_HALFHOUR 11
#define TICK_HEIGHT_10MIN 4

struct tm time_now;

//////// VARIABLES ////////

static GRect rect_needle, rect_tick, rect_label;
static GPath *tri_path;
const int NEEDLE_X_START = SCREEN_WIDTH / 2 - 1;

GFont font_label;


//////// PRIVATE ////////

/**
 * Converts a time to the corresponding x coordinate on the watchface.
 *
 * @param current the current time
 * @param convertee the time to convert to an x coordinate
 * @return the x coordinate of the time
 */
static int time_to_x_pos(struct tm *current, struct tm *convertee) {
    // Built-in difftime func is bad. https://developer.pebble.com/docs/c/Standard_C/Time/#difftime
    time_t diff_sec = mktime(convertee) - mktime(current);
    int diff_min = diff_sec / SECONDS_PER_MINUTE;
    return NEEDLE_X_START + diff_min * PX_PER_MINUTE;
}

/**
 * Corrects minutes being negative or greater than 59.
 *
 * @param input time which may need to wrap around
 * @return time the time which has been wrapped around
 */
static struct tm wrap_time(struct tm input) {
    if (input.tm_min > 59) {
        input.tm_hour++;
        input.tm_min -= 60;
    }
    if (input.tm_min < 0) {
        input.tm_hour--;
        input.tm_min += 60;
    }
    return input;
}

/**
 * Gets the previous or next time where the minute is a multiple of ten.
 *
 * @param t_old the original time
 * @param up true to go forward, false to go backwards
 * @return the time where minute is a multiple of ten
 */
static struct tm closest_multiple_of_ten(struct tm *t_old, bool up) {
    struct tm t_new = *t_old; //makes a copy of t_old

    int min_old = t_old->tm_min;
    if (min_old % 10 == 0) {
        t_new.tm_min += (up ? 10 : -10);
    } else {
        t_new.tm_min = (min_old / 10 + (up ? 1 : 0)) * 10;
    }

    return t_new;
}

/**
 * Gets the time where a tick goes, given a tick is at current_time.
 *
 * @param current_time the time from which to get the next time
 * @param up true to go forward, false to go backwards
 * @return the time where the next tick goes
 */
static struct tm get_next_time(struct tm current_time, bool up) {
    struct tm next_time = current_time; //makes a copy of current_time
    next_time.tm_min += (up ? 10 : -10);
    return wrap_time(next_time);
}



//////// PUBLIC ////////

/**
 * Sets up shapes which will be used for drawing.
 */
void init_drawing_shapes() {
    time_now = (struct tm) {.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=15, .tm_wday=4};
    rect_needle = GRect(NEEDLE_X_START, 0, 2, SCREEN_HEIGHT * 0.6);
}

/**
 * Draws the needle.
 *
 * @param layer the layer that needs to be rendered
 * @param ctx the destination graphics context in which to draw
 */
void draw_needle(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, rect_needle, 0, 0);
}

/**
 * Draws a calendar event mark. Experimental and not used.
 *
 * @param layer the layer that needs to be rendered
 * @param ctx the destination graphics context in which to draw
 */
void draw_event_mark(Layer *layer, GContext *ctx) {

    struct tm event_start = (struct tm) {.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=25, .tm_wday=4};
    struct tm event_end = (struct tm) {.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=40, .tm_wday=4};

    int x_start = time_to_x_pos(&time_now, &event_start);
    int x_end = time_to_x_pos(&time_now, &event_end);

    #define MARK_HEIGHT 8
    #define MARK_THICKNESS 2

    // https://developer.pebble.com/guides/tools-and-resources/color-picker/
    graphics_context_set_fill_color(ctx, GColorCyan);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT - 2, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_end, -MARK_HEIGHT - 2, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT - 2, x_end - x_start, MARK_THICKNESS), 0, GCornerNone);


    graphics_context_set_fill_color(ctx, GColorCadetBlue);
    graphics_fill_rect(ctx, GRect(x_start, 0, x_end - x_start + TICK_WIDTH, TICK_HEIGHT_HOUR), 0, GCornerNone);

    #define TRI_W 10
    #define TRI_H 10

    GPathInfo tri_path_info = {
            .num_points = 3, //    left      right          bottom
            .points = (GPoint[]) {{0, 0}, {TRI_W, 0}, {TRI_W / 2, TRI_H}}
    };
    tri_path = gpath_create(&tri_path_info);


    gpath_move_to(tri_path, GPoint(x_start - TRI_W / 2, -TRI_H));
    graphics_context_set_fill_color(ctx, GColorCyan);
    gpath_draw_filled(ctx, tri_path);


}

/**
 * Draws the battery bar.
 *
 * @param layer the layer that needs to be rendered
 * @param ctx the destination graphics context in which to draw
 */
void draw_batt_bar(Layer *layer, GContext *ctx) {
    int batt = batt_level;
    GColor theColor = GColorRed;
    if (batt >= 20) theColor = GColorYellow; // this is pretty shitty
    if (batt >= 40) theColor = GColorGreen;
    graphics_context_set_fill_color(ctx, theColor);

    int barWidth = batt * SCREEN_WIDTH / 100; //can't use floating point numbers!
    GRect theBar = GRect(0, 0, barWidth, BAR_THICKNESS); // x, y, w, h
    graphics_fill_rect(ctx, theBar, 0, GCornerNone);
}

/**
 * Draws one tick.
 *
 * @param ctx the destination graphics context in which to draw
 * @param tick_location the time to draw the tick for
 */
void draw_tick(GContext *ctx, struct tm *tick_location) {
    int x = time_to_x_pos(&time_now, tick_location);

    int height = -10;
    int min = tick_location->tm_min;
    if (min == 0) height = TICK_HEIGHT_HOUR;
    else if (min == 30) height = TICK_HEIGHT_HALFHOUR;
    else if (min % 10 == 0) height = TICK_HEIGHT_10MIN;


    rect_tick = GRect(x, TICK_Y_START, TICK_WIDTH, height);
    graphics_fill_rect(ctx, rect_tick, 0, GCornerNone);

    rect_label = GRect(x - 5, TICK_Y_START + TICK_HEIGHT_HOUR + 5, 40, 40);

    char buffer[7];
    if (min == 0) {
        snprintf(buffer, sizeof(buffer), "%d", tick_location->tm_hour);
        font_label = fonts_get_system_font(
                FONT_KEY_GOTHIC_24_BOLD); // https://developer.pebble.com/guides/app-resources/system-fonts/
    } else {
        //snprintf(buffer, sizeof(buffer), "%d:\n%02d", tick_location->tm_hour, tick_location->tm_min);
        //font_label = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    }
    graphics_draw_text(ctx, buffer, font_label, rect_label, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

}

/**
 * Draws the layer containing the hour numbers and ticks.
 *
 * @param layer the layer that needs to be rendered
 * @param ctx the destination graphics context in which to draw
 */
void draw_time_layer(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorWhite);

    if (time_now.tm_min == 0) draw_tick(ctx, &time_now);

    struct tm left = closest_multiple_of_ten(&time_now, false);
    struct tm right = closest_multiple_of_ten(&time_now, true);
    draw_tick(ctx, &left);
    draw_tick(ctx, &right);

    time_t ts_now = mktime(&time_now);
    time_t ts_max = ts_now + SECONDS_PER_MINUTE * 30;
    time_t ts_min = ts_now - SECONDS_PER_MINUTE * 30;

    for (struct tm curr = get_next_time(left, false); mktime(&curr) >= ts_min; curr = get_next_time(curr, false))
        draw_tick(ctx, &curr);

    for (struct tm curr = get_next_time(right, true); mktime(&curr) <= ts_max; curr = get_next_time(curr, true))
        draw_tick(ctx, &curr);

}
