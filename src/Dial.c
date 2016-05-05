#include <pebble.h>

#define BACKGROUND_WIDTH 1366
#define SCREEN_WIDTH (PBL_IF_ROUND_ELSE(180, 144))
#define SCREEN_HEIGHT (PBL_IF_ROUND_ELSE(180, 168))

static Window *main_window;
static BitmapLayer *background_layers[4];
static GBitmap *background_bitmap;

static TextLayer *layer_date, *layer_batt_percent;
static Layer *layer_needle, *layer_batt_bar, *layer_event_mark;

static int last_mins_since_midnight;

static const int NEEDLE_X_START = SCREEN_WIDTH / 2 - 1;

//// ANIMATION ////

#define ANIM_DURATION_IN 400
#define ANIM_DURATION_OUT 200
#define ANIM_DURATION_VISIBILITY 4000
static GRect frame_date_onscreen, frame_date_offscreen;
static GRect frame_batt_bar_onscreen, frame_batt_bar_offscreen;
static GRect frame_batt_percent_onscreen, frame_batt_percent_offscreen;
static bool isAnimating = 0;

static void animate_batt_bar(){
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) layer_batt_bar, &frame_batt_bar_offscreen, &frame_batt_bar_onscreen);
    animation_set_duration((Animation*) in, ANIM_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) layer_batt_bar, &frame_batt_bar_onscreen, &frame_batt_bar_offscreen);
    animation_set_duration((Animation*) out, ANIM_DURATION_OUT);
    animation_set_delay((Animation*) out, ANIM_DURATION_VISIBILITY);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static void animate_batt_percent(){
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) layer_batt_percent, &frame_batt_percent_offscreen, &frame_batt_percent_onscreen);
    animation_set_duration((Animation*) in, ANIM_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) layer_batt_percent, &frame_batt_percent_onscreen, &frame_batt_percent_offscreen);
    animation_set_duration((Animation*) out, ANIM_DURATION_OUT);
    animation_set_delay((Animation*) out, ANIM_DURATION_VISIBILITY);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static void animation_stopped_handler(Animation *animation, bool finished, void *context) {
    isAnimating = 0;
}


//// HANDLERS ////

static void handler_tap(AccelAxisType axis, int32_t direction) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tap handler");
    if (isAnimating) return;
    isAnimating = 1;
    
    animate_batt_bar();
    animate_batt_percent();
    
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) layer_date, &frame_date_offscreen, &frame_date_onscreen);
    animation_set_duration((Animation*) in, ANIM_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) layer_date, &frame_date_onscreen, &frame_date_offscreen);
    animation_set_duration((Animation*) out, ANIM_DURATION_OUT);
    animation_set_delay((Animation*) out, ANIM_DURATION_VISIBILITY);
    animation_set_handlers((Animation*) out, (AnimationHandlers) {
        .stopped = animation_stopped_handler
    }, NULL);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static int s_batt_level;
static void handler_batt(BatteryChargeState state) {
    s_batt_level = state.charge_percent;
    
    static char buffer[5]; //needs static
    snprintf(buffer, sizeof(buffer), "%d%%", s_batt_level);
    text_layer_set_text(layer_batt_percent, buffer);
    
    layer_mark_dirty(layer_batt_bar);
    layer_mark_dirty(text_layer_get_layer(layer_batt_percent));
}

static const char *days_of_week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
static void update_day(struct tm *tick_time) {
    static char buffer[7]; //needs static
    snprintf(buffer, sizeof(buffer), "%s %d", days_of_week[tick_time->tm_wday], tick_time->tm_mday);
    text_layer_set_text(layer_date, buffer);
}

static void handler_tick(struct tm *tick_time, TimeUnits units_changed) {
    const int mins_since_midnight = tick_time->tm_hour * 60 + tick_time->tm_min;
    const int background_x_offset = mins_since_midnight * BACKGROUND_WIDTH * 2 / MINUTES_PER_DAY;
    last_mins_since_midnight = mins_since_midnight;
    
    for (int i = 0; i < 4; i++) {
        GRect frame = GRect((-background_x_offset) + (SCREEN_WIDTH / 2) + BACKGROUND_WIDTH * (i - 1), 0, BACKGROUND_WIDTH, SCREEN_HEIGHT);
        layer_set_frame(bitmap_layer_get_layer(background_layers[i]), frame);
    }
    
    // units_changed is a bit mask. Might need to subscribe to MINUTE_UNIT|DAY_UNIT
    if (units_changed & DAY_UNIT) update_day(tick_time);
}


//// UPDATE PROCS ////

static GRect needle_rect;
static void draw_needle(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, needle_rect, 0, 0);
}

GPathInfo TRI_PATH_INFO; //set in init()
GPath *s_tri_path;
static const int event_time_start = 60*(1) + 30;
static const int event_time_end = 60*(1) + 45;
static void draw_event_mark(Layer *layer, GContext *ctx) {
    
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
    
    #define TRI_W 10
    #define TRI_H 10

    gpath_move_to(s_tri_path, GPoint(30, -TRI_H));
    gpath_draw_filled(ctx, s_tri_path);
    
}

static void draw_batt_bar(Layer *layer, GContext *ctx) {
    #define BAR_THICKNESS (2)
    int batt = s_batt_level;
    GColor theColor = GColorRed;
    if(batt >= 20) theColor = GColorYellow; // this is pretty shitty
    if(batt >= 40) theColor = GColorGreen;
    graphics_context_set_fill_color(ctx, theColor);
    
    int barWidth = batt * SCREEN_WIDTH / 100; //can't use floating point numbers!
    GRect theBar = GRect(0, 0, barWidth, BAR_THICKNESS); // x, y, w, h
    graphics_fill_rect(ctx, theBar, 0, GCornerNone);
}


//// CORE ////

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // BACKGROUND
    background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    for (unsigned i = 0; i < ARRAY_LENGTH(background_layers); i++) {
        background_layers[i] = bitmap_layer_create(bounds);
        bitmap_layer_set_bitmap(background_layers[i], background_bitmap);
        layer_add_child(window_layer, (Layer*) background_layers[i]);
    }
    
    // EVENT MARK
    layer_event_mark = layer_create(GRect(0, 84, SCREEN_WIDTH, 21));
    layer_set_update_proc(layer_event_mark, draw_event_mark);
    layer_set_clips(layer_event_mark, false);
    layer_add_child(window_layer, layer_event_mark);
    
    
    // NEDLE
    layer_needle = layer_create(bounds);
    layer_set_update_proc(layer_needle, draw_needle);
    layer_set_clips(layer_needle, true);
    layer_add_child(window_layer, layer_needle);
    
    
    // DATE
    GFont date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MEDIUM_14));
    layer_date = text_layer_create(frame_date_offscreen);
    text_layer_set_text_alignment(layer_date, GTextAlignmentCenter);
    text_layer_set_text_color(layer_date, GColorWhite);
    text_layer_set_font(layer_date, date_font);
    text_layer_set_background_color(layer_date, GColorBlack);
    layer_add_child(window_layer, (Layer*) layer_date);
    
    
    // BATTERY BAR
    layer_batt_bar = layer_create(frame_batt_bar_offscreen);
    layer_set_update_proc(layer_batt_bar, draw_batt_bar);
    layer_add_child(window_layer, layer_batt_bar);
    
    
    // BATTERY PERCENT
    layer_batt_percent = text_layer_create(frame_batt_percent_offscreen);
    text_layer_set_text(layer_batt_percent, "000%");
    text_layer_set_font(layer_batt_percent, date_font);
    text_layer_set_text_alignment(layer_batt_percent, GTextAlignmentLeft);
    text_layer_set_background_color(layer_batt_percent, GColorClear);
    text_layer_set_text_color(layer_batt_percent, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(layer_batt_percent));

}

static void main_window_unload(Window *window) {
    for (unsigned i = 0; i < ARRAY_LENGTH(background_layers); i++) {
        bitmap_layer_destroy(background_layers[i]);
    }
    gbitmap_destroy(background_bitmap);
    layer_destroy(layer_batt_bar);
    layer_destroy(layer_needle);
    text_layer_destroy(layer_batt_percent);
    text_layer_destroy(layer_date);
}

static void init() {
    main_window = window_create();
    window_set_background_color(main_window, GColorBlack);
    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(main_window, true);
    
    // subscribe
    tick_timer_service_subscribe(MINUTE_UNIT, handler_tick);
    accel_tap_service_subscribe(handler_tap);
    battery_state_service_subscribe(handler_batt);
    
    // draw elements for first time
    handler_batt(battery_state_service_peek());
    time_t current_time = time(NULL);
    handler_tick(localtime(&current_time), MINUTE_UNIT|DAY_UNIT);
    
    // init some shapes
    needle_rect = GRect(NEEDLE_X_START, 0, 2, SCREEN_HEIGHT * 0.6);
    GPathInfo TRI_PATH_INFO = {
        .num_points = 3,
        //                      left      right          bottom
        .points = (GPoint []) {{0, 0}, {TRI_W, 0}, {TRI_W/2, TRI_H}} //triangle size constants defined in draw_event_mark()
    };
    s_tri_path = gpath_create(&TRI_PATH_INFO);
    
    frame_date_onscreen = GRect(SCREEN_WIDTH / 2 + 2, PBL_IF_ROUND_ELSE(30, 20), PBL_IF_ROUND_ELSE(67, SCREEN_WIDTH / 2 - 2), 15);
    frame_date_offscreen = (GRect) { .origin = GPoint(frame_date_onscreen.origin.x, -50), .size = frame_date_onscreen.size };
    
    frame_batt_bar_onscreen = GRect(0, SCREEN_HEIGHT-BAR_THICKNESS, SCREEN_WIDTH, BAR_THICKNESS);
    frame_batt_bar_offscreen = (GRect) { .origin = GPoint(frame_batt_bar_onscreen.origin.x, SCREEN_HEIGHT+30), .size = frame_batt_bar_onscreen.size };
    
    frame_batt_percent_onscreen = GRect(3, SCREEN_HEIGHT-BAR_THICKNESS-20, 50, 20);
    frame_batt_percent_offscreen = (GRect) { .origin = GPoint(frame_batt_percent_onscreen.origin.x, SCREEN_HEIGHT+30), .size = frame_batt_percent_onscreen.size };
    

}

static void deinit() {
    window_destroy(main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}