#pragma once

extern void handler_tap(AccelAxisType axis, int32_t direction);
extern void handler_batt(BatteryChargeState state);
extern void handler_tick(struct tm *tick_time, TimeUnits units_changed);

extern int batt_level;