#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"


Window *main_window;

TextLayer *layer_date, *layer_batt_percent;
Layer *layer_needle, *layer_batt_bar, *layer_event_mark, *layer_time;


static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    // EVENT MARK
    layer_event_mark = layer_create(GRect(0, 84, SCREEN_WIDTH, 21));
    layer_set_update_proc(layer_event_mark, draw_event_mark);
    layer_set_clips(layer_event_mark, false); //might not be needed
    //layer_add_child(window_layer, layer_event_mark);
   
    
    // TIME
    layer_time = layer_create(bounds);
    layer_set_update_proc(layer_time, draw_time_layer);
    layer_add_child(window_layer, layer_time);
      
    
    // NEEDLE
    layer_needle = layer_create(bounds);
    layer_set_update_proc(layer_needle, draw_needle);
    layer_add_child(window_layer, layer_needle); 
    
    
    // DATE
    layer_date = text_layer_create(frame_date_offscreen);
    text_layer_set_font(layer_date, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(layer_date, "WWW DD");
    text_layer_set_text_alignment(layer_date, GTextAlignmentCenter);
    text_layer_set_background_color(layer_date, GColorClear);
    text_layer_set_text_color(layer_date, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(layer_date));
    
   
    // BATTERY BAR
    layer_batt_bar = layer_create(frame_batt_bar_offscreen);
    layer_set_update_proc(layer_batt_bar, draw_batt_bar);
    layer_add_child(window_layer, layer_batt_bar);
    
    
    // BATTERY PERCENT
    layer_batt_percent = text_layer_create(frame_batt_percent_offscreen);
    text_layer_set_text(layer_batt_percent, "000%");
    text_layer_set_font(layer_batt_percent, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(layer_batt_percent, GTextAlignmentLeft);
    text_layer_set_background_color(layer_batt_percent, GColorClear);
    text_layer_set_text_color(layer_batt_percent, GColorWhite);
    layer_add_child(window_layer, text_layer_get_layer(layer_batt_percent));
    
}

static void main_window_unload(Window *window) {
    layer_destroy(layer_batt_bar);
    layer_destroy(layer_needle);
    layer_destroy(layer_event_mark);
    layer_destroy(layer_time);
    text_layer_destroy(layer_batt_percent);
    text_layer_destroy(layer_date);
}

static void init() {
       
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

static void deinit() {
    window_destroy(main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}