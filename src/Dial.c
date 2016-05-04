#include <pebble.h>

#define BACKGROUND_WIDTH (1366)
#define SCREEN_WIDTH (PBL_IF_ROUND_ELSE(180, 144))
#define SCREEN_HEIGHT (PBL_IF_ROUND_ELSE(180, 168))



#define BAR_THICKNESS (2)


static Window *s_main_window;
static BitmapLayer *s_background_layers[4];
static GBitmap *s_background_bitmap;
static TextLayer *s_date_layer, *s_batt_percent_layer;
static Layer *needle_layer, *s_batt_bar_layer, *s_event_mark_layer;

static int last_mins_since_midnight, s_batt_level;

static const int NEEDLE_X_START = SCREEN_WIDTH / 2 - 1;


//// in progress things ////

static void draw_event_mark(GContext *ctx, int event_time_start, int event_time_end){
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "event_time is %d", event_time);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "current time is %d", last_mins_since_midnight);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "difference is %d", difference);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "------------------------");

    // http://stackoverflow.com/a/9772491
    //int abs = difference * ((difference>0) - (difference<0));
    
    int difference_now_start = event_time_start - last_mins_since_midnight;
    int difference_now_end = event_time_end - last_mins_since_midnight;
    
    int x_start = NEEDLE_X_START + difference_now_start*2 - 1 - 2;
    int x_end   = NEEDLE_X_START + difference_now_end*2 - 1 - 2;
    
    
    #define MARK_HEIGHT 10
    #define MARK_THICKNESS 2
    
    //GRect bounds = layer_get_bounds(s_event_mark_layer);
    graphics_context_set_fill_color(ctx, GColorCyan);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_end,   -MARK_HEIGHT-1, MARK_THICKNESS, MARK_HEIGHT), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(x_start, -MARK_HEIGHT-1, x_end-x_start, MARK_THICKNESS), 0, GCornerNone);
    
    // https://developer.pebble.com/docs/c/Graphics/Drawing_Paths/
    const GPathInfo TRIANGLE_PATH_INFO = {
        .num_points = 3,
        .points = (GPoint []) {{21, 0}, {14, 26}, {28, 26}}
    };
    GPath *s_my_path_ptr = gpath_create(&TRIANGLE_PATH_INFO);
    gpath_draw_filled(ctx, s_my_path_ptr);
    
}


//// ANIMATION ////

#define ANIM_DURATION_IN (400)
#define ANIM_DURATION_OUT (200)
#define ANIM_VISIBILITY_DURATION (4000)
static GRect date_frame_onscreen, date_frame_offscreen;
static GRect batt_bar_frame_onscreen, batt_bar_frame_offscreen;
static GRect batt_percent_frame_onscreen, batt_percent_frame_offscreen;
bool isAnimating = 0;

static void animate_batt_bar(){
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) s_batt_bar_layer, &batt_bar_frame_offscreen, &batt_bar_frame_onscreen);
    animation_set_duration((Animation*) in, ANIM_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) s_batt_bar_layer, &batt_bar_frame_onscreen, &batt_bar_frame_offscreen);
    animation_set_duration((Animation*) out, ANIM_DURATION_OUT);
    animation_set_delay((Animation*) out, ANIM_VISIBILITY_DURATION);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static void animate_batt_percent(){
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) s_batt_percent_layer, &batt_percent_frame_offscreen, &batt_percent_frame_onscreen);
    animation_set_duration((Animation*) in, ANIM_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) s_batt_percent_layer, &batt_percent_frame_onscreen, &batt_percent_frame_offscreen);
    animation_set_duration((Animation*) out, ANIM_DURATION_OUT);
    animation_set_delay((Animation*) out, ANIM_VISIBILITY_DURATION);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static void animation_stopped_handler(Animation *animation, bool finished, void *context) {
    isAnimating = 0;
}


//// HANDLERS ////

static void tap_handler(AccelAxisType axis, int32_t direction) {
    if (isAnimating) return;
    isAnimating = 1;
    
    animate_batt_bar();
    animate_batt_percent();
    
    PropertyAnimation *in = property_animation_create_layer_frame(
        (Layer*) s_date_layer, &date_frame_offscreen, &date_frame_onscreen);
    animation_set_duration((Animation*) in, DATE_ANIMATION_DURATION_IN);
    animation_set_curve((Animation*) in, AnimationCurveEaseInOut);
    animation_schedule((Animation*) in);
    
    PropertyAnimation *out = property_animation_create_layer_frame(
        (Layer*) s_date_layer, &date_frame_onscreen, &date_frame_offscreen);
    animation_set_duration((Animation*) out, DATE_ANIMATION_DURATION_OUT);
    animation_set_delay((Animation*) out, DATE_VISIBILITY_DURATION);
    animation_set_handlers((Animation*) out, (AnimationHandlers) {
        .stopped = animation_stopped_handler
    }, NULL);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}

static void batt_handler(BatteryChargeState state) {
    s_batt_level = state.charge_percent;
    
    static char buffer[5];
    snprintf(buffer, sizeof(buffer), "%d%%", s_batt_level);
    text_layer_set_text(s_batt_percent_layer, buffer);
    
    layer_mark_dirty(s_batt_bar_layer);
    layer_mark_dirty(text_layer_get_layer(s_batt_percent_layer));
}

static void tick_handler_minute(struct tm *tick_time, TimeUnits units_changed) {
    const int mins_since_midnight = tick_time->tm_hour * 60 + tick_time->tm_min;
    const int background_x_offset = mins_since_midnight * BACKGROUND_WIDTH * 2 / MINUTES_PER_DAY;
    last_mins_since_midnight = mins_since_midnight;
    
    for (int i = 0; i < 4; i++) {
        GRect frame = GRect((-background_x_offset) + (SCREEN_WIDTH / 2) + BACKGROUND_WIDTH * (i - 1), 0, BACKGROUND_WIDTH, SCREEN_HEIGHT);
        layer_set_frame(bitmap_layer_get_layer(s_background_layers[i]), frame);
    }
}

static const char *days_of_week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
static void tick_handler_day(struct tm *tick_time, TimeUnits units_changed) {
    static char buffer[7];
    snprintf(buffer, sizeof(buffer), "%s %d", days_of_week[tick_time->tm_wday], tick_time->tm_mday);
    text_layer_set_text(s_date_layer, buffer);
}


//// UPDATE PROCS ////

static void needle_layer_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, GRect(NEEDLE_X_START, 0, 2, SCREEN_HEIGHT * 0.6), 0, 0);
}

static void event_mark_update_proc(Layer *layer, GContext *ctx) {
    draw_event_mark(ctx, 60*(12+4) + 10, 60*(12+4) + 30);
}

static void batt_bar_update_proc(Layer *layer, GContext *ctx) {
    int batt = s_batt_level;
    GColor theColor = GColorRed;
    if(batt >= 20) theColor = GColorYellow;
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
    
    
    date_frame_onscreen = GRect(SCREEN_WIDTH / 2 + 2, PBL_IF_ROUND_ELSE(30, 20), PBL_IF_ROUND_ELSE(67, SCREEN_WIDTH / 2 - 2), 15);
    date_frame_offscreen = (GRect) { .origin = GPoint(date_frame_onscreen.origin.x, -50), .size = date_frame_onscreen.size };
    
    batt_bar_frame_onscreen = GRect(0, SCREEN_HEIGHT-BAR_THICKNESS, SCREEN_WIDTH, BAR_THICKNESS);
    batt_bar_frame_offscreen = (GRect) { .origin = GPoint(batt_bar_frame_onscreen.origin.x, SCREEN_HEIGHT+30), .size = batt_bar_frame_onscreen.size };
    
    batt_percent_frame_onscreen = GRect(3, SCREEN_HEIGHT-BAR_THICKNESS-20, 50, 20);
    batt_percent_frame_offscreen = (GRect) { .origin = GPoint(batt_percent_frame_onscreen.origin.x, SCREEN_HEIGHT+30), .size = batt_percent_frame_onscreen.size };
    
    // BACKGROUND
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_background_layers); i++) {
        s_background_layers[i] = bitmap_layer_create(bounds);
        bitmap_layer_set_bitmap(s_background_layers[i], s_background_bitmap);
        layer_add_child(window_layer, (Layer*) s_background_layers[i]);
    }
    
    // EVENT MARK
    s_event_mark_layer = layer_create(GRect(0, 84, SCREEN_WIDTH, 21));
    layer_set_update_proc(s_event_mark_layer, event_mark_update_proc);
    layer_set_clips(s_event_mark_layer, false);
    layer_add_child(window_layer, s_event_mark_layer);
    
    
    // NEDLE
    needle_layer = layer_create(bounds);
    layer_set_update_proc(needle_layer, needle_layer_update_proc);
    layer_add_child(window_layer, needle_layer);
    
    
    // DATE
    GFont date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MEDIUM_14));
    s_date_layer = text_layer_create(date_frame_offscreen);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_font(s_date_layer, date_font);
    text_layer_set_background_color(s_date_layer, GColorBlack);
    layer_add_child(window_layer, (Layer*) s_date_layer);
    
    
    // BATTERY BAR
    s_batt_bar_layer = layer_create(batt_bar_frame_offscreen);
    layer_set_update_proc(s_batt_bar_layer, batt_bar_update_proc);
    layer_add_child(window_layer, s_batt_bar_layer);
    
    
    // BATTERY PERCENT
    s_batt_percent_layer = text_layer_create(batt_percent_frame_offscreen);
    text_layer_set_text(s_batt_percent_layer, "000%");
    text_layer_set_font(s_batt_percent_layer, date_font);
    text_layer_set_text_alignment(s_batt_percent_layer, GTextAlignmentLeft);
    text_layer_set_background_color(s_batt_percent_layer, GColorClear);
    text_layer_set_text_color(s_batt_percent_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(s_batt_percent_layer));

}

static void main_window_unload(Window *window) {
    for (unsigned i = 0; i < ARRAY_LENGTH(s_background_layers); i++) {
        bitmap_layer_destroy(s_background_layers[i]);
    }
    gbitmap_destroy(s_background_bitmap);
    layer_destroy(s_batt_bar_layer);
    layer_destroy(needle_layer);
    text_layer_destroy(s_batt_percent_layer);
    text_layer_destroy(s_date_layer);
}

static void init() {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);
    
    time_t current_time = time(NULL);
    tick_handler_minute(localtime(&current_time), 0);
    tick_handler_day(localtime(&current_time), 0);
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_minute);
    tick_timer_service_subscribe(DAY_UNIT, tick_handler_day);
    accel_tap_service_subscribe(tap_handler);
    battery_state_service_subscribe(batt_handler);
    
    // Ensure batt level is displayed from the start
    batt_handler(battery_state_service_peek());

}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}