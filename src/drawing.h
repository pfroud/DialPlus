#pragma once


#define BACKGROUND_WIDTH 1366
#define SCREEN_WIDTH (PBL_IF_ROUND_ELSE(180, 144))
#define SCREEN_HEIGHT (PBL_IF_ROUND_ELSE(180, 168))


#define PX_PER_MINUTE 2
#define TICK_Y_START 86
#define TICK_WIDTH 2
#define TICK_HEIGHT_HOUR 21
#define TICK_HEIGHT_HALFHOUR 11
#define TICK_HEIGHT_10MINS 4

#define TRI_W 10
#define TRI_H 10

#define TIME 58

#define BAR_THICKNESS 2

extern void init_drawing();

extern void draw_needle(Layer *layer, GContext *ctx);

extern void draw_event_mark(Layer *layer, GContext *ctx);

extern void draw_batt_bar(Layer *layer, GContext *ctx);

extern void draw_tick(GContext *ctx, int time);

extern void draw_bg_new(Layer *layer, GContext *ctx);