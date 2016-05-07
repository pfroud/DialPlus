#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"


Window *main_window;
BitmapLayer *background_layers[4];
GBitmap *background_bitmap;

TextLayer *layer_date, *layer_batt_percent;
Layer *layer_needle, *layer_batt_bar, *layer_event_mark, *layer_bg_new;

int last_mins_since_midnight;

int s_batt_level;


void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // BACKGROUND
    /*
    background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
    for (unsigned i = 0; i < ARRAY_LENGTH(background_layers); i++) {
        background_layers[i] = bitmap_layer_create(bounds);
        bitmap_layer_set_bitmap(background_layers[i], background_bitmap);
        layer_add_child(window_layer, (Layer*) background_layers[i]);
    }
    */
    layer_bg_new = layer_create(bounds);
    layer_set_update_proc(layer_bg_new, draw_new_bg_layer);
    layer_add_child(window_layer, layer_bg_new);
      
    // EVENT MARK
    layer_event_mark = layer_create(GRect(0, 84, SCREEN_WIDTH, 21));
    layer_set_update_proc(layer_event_mark, draw_event_mark);
    layer_set_clips(layer_event_mark, false);
    layer_add_child(window_layer, layer_event_mark);
    
    
    // NEDLE
    layer_needle = layer_create(bounds);
    layer_set_update_proc(layer_needle, draw_needle);
    layer_add_child(window_layer, layer_needle);
    
    
    // DATE
    GFont date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MEDIUM_14));
    layer_date = text_layer_create(frame_date_offscreen);
    text_layer_set_text_alignment(layer_date, GTextAlignmentCenter);
    text_layer_set_text_color(layer_date, GColorWhite);
    text_layer_set_font(layer_date, date_font);
    text_layer_set_background_color(layer_date, GColorBlack);
    //layer_add_child(window_layer, (Layer*) layer_date);
    
    
    // BATTERY BAR
    layer_batt_bar = layer_create(frame_batt_bar_offscreen);
    layer_set_update_proc(layer_batt_bar, draw_batt_bar);
    //layer_add_child(window_layer, layer_batt_bar);
    
    
    // BATTERY PERCENT
    layer_batt_percent = text_layer_create(frame_batt_percent_offscreen);
    text_layer_set_text(layer_batt_percent, "000%");
    text_layer_set_font(layer_batt_percent, date_font);
    text_layer_set_text_alignment(layer_batt_percent, GTextAlignmentLeft);
    text_layer_set_background_color(layer_batt_percent, GColorClear);
    text_layer_set_text_color(layer_batt_percent, GColorWhite);
    //layer_add_child(window_layer, text_layer_get_layer(layer_batt_percent));

}

void main_window_unload(Window *window) {
    for (unsigned i = 0; i < ARRAY_LENGTH(background_layers); i++) {
        bitmap_layer_destroy(background_layers[i]);
    }
    gbitmap_destroy(background_bitmap);
    layer_destroy(layer_batt_bar);
    layer_destroy(layer_needle);
    text_layer_destroy(layer_batt_percent);
    text_layer_destroy(layer_date);
}

void init() {
    Window *main_window = window_create();
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
    
    // init shapes
    init_drawing_shapes();
    init_anim_frames();
    
}

void deinit() {
    window_destroy(main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}