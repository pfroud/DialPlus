#include <pebble.h>
#include "main.h"
#include "drawing.h"


#define PX_PER_MINUTE 2
#define TICK_Y_START 86
#define TICK_WIDTH 2
#define TICK_HEIGHT_HOUR 21
#define TICK_HEIGHT_HALFHOUR 11
#define TICK_HEIGHT_10MINS 4

#define TRI_W 10
#define TRI_H 10

#define TIME 58

struct tm time_now;

//////// VARIABLES ////////

static const int event_time_start = 60*(1) + 30;
static const int event_time_end = 60*(1) + 45;
static GRect needle_rect;
static GRect rectTick, rectLabel;
static GPath *s_tri_path;
const int NEEDLE_X_START = SCREEN_WIDTH / 2 - 1;


//////// PRIVATE ////////

static void draw_bg_color(GContext *ctx){
    GRect bounds = layer_get_bounds(layer_bg_new);
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static int time_to_x_pos_new(struct tm* current, struct tm* convertee){
    // Built-in difftime func is bad. https://developer.pebble.com/docs/c/Standard_C/Time/#difftime
    time_t diff_sec = mktime(convertee) - mktime(current);
    int diff_min = diff_sec / SECONDS_PER_MINUTE; 
    return NEEDLE_X_START + diff_min * PX_PER_MINUTE;
}

static struct tm closest_multiple_of_ten_new(struct tm* t_old, bool up){
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
    /*
    if(next_time.tm_min > 59){
        next_time.tm_hour++;
        next_time.tm_min -= 60;
    }
    if(next_time.tm_min < 0){
        next_time.tm_hour--;
        next_time.tm_min += 60;
    }
    */
    
    return next_time;
}


//////// PUBLIC ////////

void init_drawing_shapes() {
    
    time_now = (struct tm){.tm_year=116, .tm_mon=4, .tm_mday=5, .tm_hour=3, .tm_min=15, .tm_wday=4};
    
    needle_rect = GRect(NEEDLE_X_START, 0, 2, SCREEN_HEIGHT * 0.6);
    GPathInfo TRI_PATH_INFO = {
        .num_points = 3,
        //                      left      right          bottom
        .points = (GPoint []) {{0, 0}, {TRI_W, 0}, {TRI_W/2, TRI_H}} //triangle size constants defined in draw_event_mark()
    };
    s_tri_path = gpath_create(&TRI_PATH_INFO);
    
}

void draw_needle(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, needle_rect, 0, 0);
}

void draw_event_mark(Layer *layer, GContext *ctx) {
    
    //int abs = difference * ((difference>0) - (difference<0)); // http://stackoverflow.com/a/9772491
    
    int difference_now_start = event_time_start - last_mins_since_midnight;
    int difference_now_end = event_time_end - last_mins_since_midnight;
    
    int x_start = NEEDLE_X_START + difference_now_start*2 - 1 - 2;
    int x_end   = NEEDLE_X_START + difference_now_end*2 - 1 - 2;
    
    #define MARK_HEIGHT 10
    #define MARK_THICKNESS 2
    
    //GRect bounds = layer_get_bounds(layer_event_mark);
    graphics_context_set_fill_color(ctx, GColorCyan);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_end,   -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1,  x_end-x_start, MARK_THICKNESS), 0, GCornerNone);
    
    gpath_move_to(s_tri_path, GPoint(30, -TRI_H));
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

void draw_tick_new(GContext *ctx, struct tm* tick_location){
    APP_LOG(APP_LOG_LEVEL_DEBUG, "drawing tick at %d:%d", tick_location->tm_hour, tick_location->tm_min);
    int x = time_to_x_pos_new(&time_now, tick_location);

    rectTick = GRect(x, TICK_Y_START, TICK_WIDTH, TICK_HEIGHT_HOUR);
    graphics_fill_rect(ctx, rectTick, 0, GCornerNone);
    
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%d:\n%02d", tick_location->tm_hour, tick_location->tm_min);
    rectLabel = GRect(x-5, TICK_Y_START+TICK_HEIGHT_HOUR, 40, 40);
    graphics_draw_text(ctx, buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), rectLabel, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void draw_new_bg_layer(Layer *layer, GContext *ctx){
    draw_bg_color(ctx);
    graphics_context_set_fill_color(ctx, GColorWhite);
      
    struct tm left  = closest_multiple_of_ten_new(&time_now, false);
    struct tm right = closest_multiple_of_ten_new(&time_now, true);
    draw_tick_new(ctx, &left);
    draw_tick_new(ctx, &right);
    
    time_t ts_now = mktime(&time_now);
    time_t ts_max = ts_now + SECONDS_PER_MINUTE * 30;
    time_t ts_min = ts_now - SECONDS_PER_MINUTE * 30;
    
    for(struct tm active = get_next_time(left, false); mktime(&active) >= ts_min; active = get_next_time(active, false)) draw_tick_new(ctx, &active);
    for(struct tm active = get_next_time(right, true); mktime(&active) <= ts_max; active = get_next_time(active, true)) draw_tick_new(ctx, &active);
    
   
    
    
}


