#include <pebble.h>
#include "main.h"
#include "drawing.h"


#define PX_PER_MINUTE 2
#define TICK_Y_START 86
#define TICK_WIDTH 2
#define TICK_HEIGHT_HOUR 21
#define TICK_HEIGHT_HALFHOUR 11
#define TICK_HEIGHT_10MIN 4

struct tm time_now;

//////// VARIABLES ////////

static GRect needle_rect;
static GRect rectTick, rectLabel;
static GPath *s_tri_path;
const int NEEDLE_X_START = SCREEN_WIDTH / 2 - 1;

GFont label_font;


//////// PRIVATE ////////

static int time_to_x_pos(struct tm* current, struct tm* convertee){
    // Built-in difftime func is bad. https://developer.pebble.com/docs/c/Standard_C/Time/#difftime
    time_t diff_sec = mktime(convertee) - mktime(current);
    int diff_min = diff_sec / SECONDS_PER_MINUTE; 
    return NEEDLE_X_START + diff_min * PX_PER_MINUTE;
}

static struct tm wrap_time(struct tm input){
    if(input.tm_min > 59){
        input.tm_hour++;
        input.tm_min -= 60;
    }
    if(input.tm_min < 0){
        input.tm_hour--;
        input.tm_min += 60;
    }
    return input;
}

static struct tm closest_multiple_of_ten(struct tm* t_old, bool up){
    struct tm t_new = *t_old; //makes a copy of t_old
    
    int min_old = t_old->tm_min;
    if(min_old % 10 == 0){
        t_new.tm_min += (up ? 10 : -10);
    } else {
        t_new.tm_min = (min_old/10 + (up?1:0)) * 10;
    }
    
    return t_new;
}

static struct tm get_next_time(struct tm current_time, bool up){
    struct tm next_time = current_time; //makes a copy of current_time
        
    // TODO wrap at 1 and 12
    
    next_time.tm_min += (up ? 10 : -10);
    
    return wrap_time(next_time);
}



//////// PUBLIC ////////

void init_drawing_shapes() {
    time_now = (struct tm){.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=15, .tm_wday=4};
    needle_rect = GRect(NEEDLE_X_START, 0, 2, SCREEN_HEIGHT * 0.6);
}

void draw_needle(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, needle_rect, 0, 0);
}

void draw_event_mark(Layer *layer, GContext *ctx) {
    
    //int abs = difference * ((difference>0) - (difference<0)); // http://stackoverflow.com/a/9772491
    
    struct tm event_start = (struct tm){.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=25, .tm_wday=4};
    struct tm event_end   = (struct tm){.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=40, .tm_wday=4};
    
    int x_start = time_to_x_pos(&time_now, &event_start);
    int x_end   = time_to_x_pos(&time_now, &event_end);
    
    #define MARK_HEIGHT 10
    #define MARK_THICKNESS 2
    
    //GRect bounds = layer_get_bounds(layer_event_mark);
    /*
    graphics_context_set_fill_color(ctx, GColorCyan); // https://developer.pebble.com/guides/tools-and-resources/color-picker/
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_end,   -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1,  x_end-x_start, MARK_THICKNESS), 0, GCornerNone);
    */
    
    graphics_context_set_fill_color(ctx, GColorCadetBlue);
    graphics_fill_rect(ctx, GRect(x_start, 0, x_end-x_start + TICK_WIDTH, TICK_HEIGHT_HOUR), 0, GCornerNone);
    
    #define TRI_W 10
    #define TRI_H 10
    GPathInfo TRI_PATH_INFO = {
        .num_points = 3,
        //                      left      right          bottom
        .points = (GPoint []) {{0, 0}, {TRI_W, 0}, {TRI_W/2, TRI_H}}
    };
    s_tri_path = gpath_create(&TRI_PATH_INFO);
    
    gpath_move_to(s_tri_path, GPoint(30, -TRI_H));
    graphics_context_set_fill_color(ctx, GColorCyan);
    gpath_draw_filled(ctx, s_tri_path);
    
}

void draw_batt_bar(Layer *layer, GContext *ctx) {
    int batt = s_batt_level;
    GColor theColor = GColorRed;
    if(batt >= 20) theColor = GColorYellow; // this is pretty shitty
    if(batt >= 40) theColor = GColorGreen;
    graphics_context_set_fill_color(ctx, theColor);
    
    int barWidth = batt * SCREEN_WIDTH / 100; //can't use floating point numbers!
    GRect theBar = GRect(0, 0, barWidth, BAR_THICKNESS); // x, y, w, h
    graphics_fill_rect(ctx, theBar, 0, GCornerNone);
}

void draw_tick(GContext *ctx, struct tm* tick_location){
    int x = time_to_x_pos(&time_now, tick_location);

    int height = -10;
    int min = tick_location->tm_min;
    if(min == 0)          height = TICK_HEIGHT_HOUR;
    else if (min == 30)   height = TICK_HEIGHT_HALFHOUR;
    else if (min%10 == 0) height = TICK_HEIGHT_10MIN;
    
    
    rectTick = GRect(x, TICK_Y_START, TICK_WIDTH, height);
    graphics_fill_rect(ctx, rectTick, 0, GCornerNone);
    
    rectLabel = GRect(x-5, TICK_Y_START+TICK_HEIGHT_HOUR+5, 40, 40);
    
    char buffer[7];
    if(min == 0){
        snprintf(buffer, sizeof(buffer), "%d", tick_location->tm_hour);
        label_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD); // https://developer.pebble.com/guides/app-resources/system-fonts/
    } else {    
        //snprintf(buffer, sizeof(buffer), "%d:\n%02d", tick_location->tm_hour, tick_location->tm_min);
        //label_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    }
    graphics_draw_text(ctx, buffer, label_font, rectLabel, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    
}

void draw_time_layer(Layer *layer, GContext *ctx){
    //draw_bg_color(ctx);
    graphics_context_set_fill_color(ctx, GColorWhite);
    
    if(time_now.tm_min == 0) draw_tick(ctx, &time_now);
    
    struct tm left  = closest_multiple_of_ten(&time_now, false);
    struct tm right = closest_multiple_of_ten(&time_now, true);
    draw_tick(ctx, &left);
    draw_tick(ctx, &right);
    
    time_t ts_now = mktime(&time_now);
    time_t ts_max = ts_now + SECONDS_PER_MINUTE * 30;
    time_t ts_min = ts_now - SECONDS_PER_MINUTE * 30;
    
    for(struct tm active = get_next_time(left, false); mktime(&active) >= ts_min; active = get_next_time(active, false)) draw_tick(ctx, &active);
    for(struct tm active = get_next_time(right, true); mktime(&active) <= ts_max; active = get_next_time(active, true)) draw_tick(ctx, &active);
    
   
    
    
}


