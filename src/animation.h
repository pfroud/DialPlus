#pragma once

#define ANIM_DURATION_IN 400
#define ANIM_DURATION_OUT 200
#define ANIM_DURATION_VISIBILITY 4000

extern GRect frame_date_onscreen, frame_date_offscreen;
extern GRect frame_batt_bar_onscreen, frame_batt_bar_offscreen;
extern GRect frame_batt_percent_onscreen, frame_batt_percent_offscreen;
extern bool isAnimating;

extern void animate_batt_bar();
extern void animate_batt_percent();
extern void animation_stopped_handler(Animation *animation, bool finished, void *context);