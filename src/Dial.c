#include <pebble.h>

#define BACKGROUND_WIDTH (1366)
#define SCREEN_WIDTH (PBL_IF_ROUND_ELSE(180, 144))
#define SCREEN_HEIGHT (PBL_IF_ROUND_ELSE(180, 168))

#define DATE_ANIMATION_DURATION_IN (400)
#define DATE_ANIMATION_DURATION_OUT (200)
#define DATE_VISIBILITY_DURATION (4000)

# define BAR_THICKNESS 2


static Window *s_main_window;
static BitmapLayer *s_background_layers[4];
static GBitmap *s_background_bitmap;
static TextLayer *s_date_layer, *s_battery_percent_layer;

static GRect date_frame_onscreen, date_frame_offscreen;

bool dateIsAnimating = 0;

static Layer *s_battery_meter_layer;
static int s_battery_level;

// https://developer.pebble.com/tutorials/intermediate/add-battery/


static void battery_handler(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    
    static char buffer[5];
    snprintf(buffer, sizeof(buffer), "%d%%", s_battery_level);
    text_layer_set_text(s_battery_percent_layer, buffer);
    
    layer_mark_dirty(s_battery_meter_layer);
    layer_mark_dirty(text_layer_get_layer(s_battery_percent_layer));

}

static void battery_meter_update_proc(Layer *layer, GContext *ctx) {
    int batt = s_battery_level;
    GColor theColor = GColorRed;
    if(batt >= 20) theColor = GColorYellow;
    if(batt >= 40) theColor = GColorGreen;
    graphics_context_set_fill_color(ctx, theColor);
    
    int barWidth = batt * SCREEN_WIDTH / 100; //can't use floating point numbers!
    GRect theBar = GRect(0, SCREEN_HEIGHT-BAR_THICKNESS, barWidth, BAR_THICKNESS); // x, y, w, h
    graphics_fill_rect(ctx, theBar, 0, GCornerNone);
}


static const char *days_of_week[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
};

static void date_animation_stopped_handler(Animation *animation, bool finished, void *context) {
    dateIsAnimating = 0;
}

/** Called by tap_handler(). */
static void animate_date() {
    if (dateIsAnimating) return;
    dateIsAnimating = 1;
    
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
        .stopped = date_animation_stopped_handler
    }, NULL);
    animation_set_curve((Animation*) out, AnimationCurveEaseIn);
    animation_schedule((Animation*) out);
}


/** Called by tick_handler(). */
static void draw_clock(struct tm *tick_time) {
    const int64_t mins_in_day = 24 * 60;
    const int64_t mins_since_midnight = tick_time->tm_hour * 60 + tick_time->tm_min;
    const int64_t background_x_offset = mins_since_midnight * BACKGROUND_WIDTH * 2 / mins_in_day;
    
    for (int i = 0; i < 4; i++) {
        GRect frame = GRect((-background_x_offset) + (SCREEN_WIDTH / 2) + BACKGROUND_WIDTH * (i - 1), 0, BACKGROUND_WIDTH, SCREEN_HEIGHT);
        layer_set_frame(bitmap_layer_get_layer(s_background_layers[i]), frame);
    }
}

/** Called by tick_handler(). */
static void draw_date(struct tm *tick_time) {
    static char buffer[7];
    snprintf(buffer, sizeof(buffer), "%s %d", days_of_week[tick_time->tm_wday], tick_time->tm_mday);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, buffer);
    text_layer_set_text(s_date_layer, buffer);
}


/**
 * Assigned in init().
 * Calls animate_date().
 */
static void tap_handler(AccelAxisType axis, int32_t direction) {
    animate_date();
}


/**
 * Assigned in init().
 * Calls draw_clock() and draw_date().
 */
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    draw_clock(tick_time);
    draw_date(tick_time);
}


/** Assigned in main_window_load(). */
static void needle_layer_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_rect(ctx, GRect(SCREEN_WIDTH / 2 - 1, 0, 2, SCREEN_HEIGHT * 0.6), 0, 0);
}


static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    
    // BACKGROUND
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_background_layers); i++) {
        s_background_layers[i] = bitmap_layer_create(bounds);
        bitmap_layer_set_bitmap(s_background_layers[i], s_background_bitmap);
        layer_add_child(window_layer, (Layer*) s_background_layers[i]);
    }
    
    
    // NEDLE
    Layer *needle_layer = layer_create(bounds);
    layer_set_update_proc(needle_layer, needle_layer_update_proc);
    layer_add_child(window_layer, needle_layer);
    
    
    // DATE
    date_frame_onscreen = GRect(SCREEN_WIDTH / 2 + 2, PBL_IF_ROUND_ELSE(30, 20), PBL_IF_ROUND_ELSE(67, SCREEN_WIDTH / 2 - 2), 15);
    date_frame_offscreen = (GRect) { .origin = GPoint(date_frame_onscreen.origin.x, -50), .size = date_frame_onscreen.size };
    GFont date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MEDIUM_14));
    s_date_layer = text_layer_create(date_frame_offscreen);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_font(s_date_layer, date_font);
    text_layer_set_background_color(s_date_layer, GColorBlack);
    layer_add_child(window_layer, (Layer*) s_date_layer);
    
    
    // BATTERY METER
    s_battery_meter_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_meter_layer, battery_meter_update_proc);
    layer_add_child(window_layer, s_battery_meter_layer);
    
    
    // BATTERY PERCENT
    s_battery_percent_layer = text_layer_create(GRect(0, SCREEN_HEIGHT-BAR_THICKNESS-20, 50, 20)); // x, y, w, h
    text_layer_set_text(s_battery_percent_layer, "000%");
    text_layer_set_font(s_battery_percent_layer, date_font);
    text_layer_set_text_alignment(s_battery_percent_layer, GTextAlignmentLeft);
    text_layer_set_background_color(s_battery_percent_layer, GColorClear);
    text_layer_set_text_color(s_battery_percent_layer, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(s_battery_percent_layer));

}

static void main_window_unload(Window *window) {
    for (unsigned i = 0; i < ARRAY_LENGTH(s_background_layers); i++) {
        bitmap_layer_destroy(s_background_layers[i]);
    }
    gbitmap_destroy(s_background_bitmap);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_battery_percent_layer);
    layer_destroy(s_battery_meter_layer);

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
    draw_clock(localtime(&current_time));
    draw_date(localtime(&current_time));
    
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    accel_tap_service_subscribe(tap_handler);
    battery_state_service_subscribe(battery_handler);
    
    // Ensure battery level is displayed from the start
    battery_handler(battery_state_service_peek());

}

static void deinit() {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
