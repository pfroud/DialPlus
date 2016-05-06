#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"


GRect frame_date_onscreen, frame_date_offscreen;
GRect frame_batt_bar_onscreen, frame_batt_bar_offscreen;
GRect frame_batt_percent_onscreen, frame_batt_percent_offscreen;
bool isAnimating = 0;

void animate_batt_bar(){
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

void animation_stopped_handler(Animation *animation, bool finished, void *context) {
    isAnimating = 0;
}

