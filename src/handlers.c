#include <pebble.h>

#include "main.h"
#include "drawing.h"
#include "handlers.h"
#include "animation.h"

const char *days_of_week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
int batt_level;

/**
 * Handles interrupts when the accelerometer detects a "tap" (a shake along one of three axes)
 *
 * @param axis the axis on which a tap was registered (x, y, or z)
 * @param direction the direction (-1 or +1) of the tap
 */
void handler_tap(AccelAxisType axis, int32_t direction) {
    if (isAnimating) return;
    isAnimating = 1;

    animate_batt_bar();
    animate_batt_percent();
    animate_date();
}

/**
 * Handles interrupts when the battery state changes.
 *
 * @param state structure for retrieval of the battery charge state
 */
void handler_batt(BatteryChargeState state) {
    batt_level = state.charge_percent;

    static char buffer[5]; //needs static
    snprintf(buffer, sizeof(buffer), "%d%%", batt_level);
    text_layer_set_text(layer_batt_percent, buffer);

    layer_mark_dirty(layer_batt_bar);
    layer_mark_dirty(text_layer_get_layer(layer_batt_percent));
}

/**
 * Updates the day of the week and the day number.
 *
 * @param tick_time the datetime to update the date to
 */
static void update_day(struct tm *tick_time) {
    static char buffer[7]; //needs static
    snprintf(buffer, sizeof(buffer), "%s %d", days_of_week[tick_time->tm_wday], tick_time->tm_mday);
    text_layer_set_text(layer_date, buffer);
}

/**
 * Handles interrupts when the time changes.
 *
 * @param tick_time the time at which the tick event was triggered
 * @param units_changed which unit change triggered this tick event
 */
void handler_tick(struct tm *tick_time, TimeUnits units_changed) {
    //time_now = &tick_time;

    // it seems I removed the part that updates the clock

    // units_changed is a bit mask. Might need to subscribe to MINUTE_UNIT|DAY_UNIT
    // https://developer.pebble.com/docs/c/Foundation/Event_Service/TickTimerService/#TimeUnits
    if (units_changed & DAY_UNIT) update_day(tick_time);
}

