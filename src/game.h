#ifndef GAME_H
#define GAME_H

typedef struct Game_State
{
    int total_clicks;
    int seconds;
    double from_time;
    double to_time;
    double clicks_per_second;
} Game_State;

typedef struct Resolution
{
    int width;
    int height;

} Resolution;

typedef struct Previous_Resolution
{
    Resolution resolution;

} Previous_Resolution;

typedef enum {
    STATE_START_SCREEN = 0,
    STATE_CPS_SCREEN
} Screen_State;

void draw_start_screen(Resolution *resolution, Screen_State *state, Game_State *game_state);
void draw_cps_screen(Resolution *resolution, Font *font, Game_State *game_state);
void draw_debug_info(Font *font, Resolution *resolution);





#endif // GAME_H
