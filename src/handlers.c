#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"

const char *days_of_week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

void handler_tap(AccelAxisType axis, int32_t direction) {
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

void handler_batt(BatteryChargeState state) {
    s_batt_level = state.charge_percent;
    
    static char buffer[5]; //needs static
    snprintf(buffer, sizeof(buffer), "%d%%", s_batt_level);
    text_layer_set_text(layer_batt_percent, buffer);
    
    layer_mark_dirty(layer_batt_bar);
    layer_mark_dirty(text_layer_get_layer(layer_batt_percent));
}

void update_day(struct tm *tick_time) {
    static char buffer[7]; //needs static
    snprintf(buffer, sizeof(buffer), "%s %d", days_of_week[tick_time->tm_wday], tick_time->tm_mday);
    text_layer_set_text(layer_date, buffer);
}

void handler_tick(struct tm *tick_time, TimeUnits units_changed) {
    //time_now = &tick_time;
    
    // units_changed is a bit mask. Might need to subscribe to MINUTE_UNIT|DAY_UNIT
    if (units_changed & DAY_UNIT) update_day(tick_time);
}

