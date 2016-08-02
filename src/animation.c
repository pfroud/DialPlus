#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"

GRect frame_date_onscreen, frame_date_offscreen;
GRect frame_batt_bar_onscreen, frame_batt_bar_offscreen;
GRect frame_batt_percent_onscreen, frame_batt_percent_offscreen;
bool isAnimating = 0;

void init_anim_frames(){
    frame_date_onscreen = GRect(SCREEN_WIDTH / 2 + 2, 20, SCREEN_WIDTH / 2 - 2, 25);
    frame_date_offscreen = (GRect) { .origin = GPoint(frame_date_onscreen.origin.x, -50), .size = frame_date_onscreen.size };
    
    frame_batt_bar_onscreen = GRect(0, SCREEN_HEIGHT-BAR_THICKNESS, SCREEN_WIDTH, BAR_THICKNESS);
    frame_batt_bar_offscreen = (GRect) { .origin = GPoint(frame_batt_bar_onscreen.origin.x, SCREEN_HEIGHT+30), .size = frame_batt_bar_onscreen.size };
    
    frame_batt_percent_onscreen = GRect(3, SCREEN_HEIGHT-BAR_THICKNESS-22, 50, 20);
    frame_batt_percent_offscreen = (GRect) { .origin = GPoint(frame_batt_percent_onscreen.origin.x, SCREEN_HEIGHT+30), .size = frame_batt_percent_onscreen.size };
}

void animate_batt_bar(){
    
    // these must be reallocated every time
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

void animate_batt_percent(){
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

void animate_date(){
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

void animation_stopped_handler(Animation *animation, bool finished, void *context) {
    isAnimating = 0;
}

