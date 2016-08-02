#pragma once


#define BACKGROUND_WIDTH 1366
#define SCREEN_WIDTH (PBL_IF_ROUND_ELSE(180, 144))
#define SCREEN_HEIGHT (PBL_IF_ROUND_ELSE(180, 168))

#define BAR_THICKNESS 2

#define ANIM_DURATION_IN 400
#define ANIM_DURATION_OUT 200
#define ANIM_DURATION_VISIBILITY 4000


extern void init_drawing_shapes();

extern void draw_needle(Layer *layer, GContext *ctx);

extern void draw_event_mark(Layer *layer, GContext *ctx);

extern void draw_batt_bar(Layer *layer, GContext *ctx);

extern void draw_tick(GContext *ctx, struct tm *tick_time);

extern void draw_time_layer(Layer *layer, GContext *ctx);