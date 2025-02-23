#include "raylib.h"
#include "game.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>


void draw_debug_info(Font *font, Resolution *resolution){

    const char *formatted_text = TextFormat("Resolution: %dx%d", resolution->width, resolution->height);
    int posY = 0;
    DrawTextEx(*font, "Debug info", (Vector2){0,posY}, 24.0f, 0, BLACK);
    
    posY += 24;
    DrawTextEx(*font, formatted_text, (Vector2){0,posY}, 24.0f, 0, BLACK);
    posY += 24;

}

void reset_game_state(Game_State *game_state){
   game_state->total_clicks = 0;
   game_state->seconds = 0;
   game_state->from_time = 0.0;
   game_state->to_time = 0.0;
   game_state->clicks_per_second = 0.0;
}


void draw_back_button(Resolution *resolution, Font *font, Screen_State *state){
    const int height = 35;
    DrawRectangle(resolution->width - 120, 0, 120, height, BLACK);
    const char *formatted_text = TextFormat("Main Menu");

    DrawTextEx(*font, formatted_text, (Vector2){resolution->width - 110 ,0}, 24.0f, 0, WHITE);
    Vector2 mouse_position = GetMousePosition();
    if(
        mouse_position.x > resolution->width - 120 &&
        mouse_position.y < height &&
        IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
    ){
        *state = STATE_START_SCREEN;
    }
}

void draw_start_screen(Resolution *resolution, Screen_State *state, Game_State *game_state){

    Color left_color = {0, 128, 0, 255};
    Color right_color = {128, 0, 0, 255};
    Vector2 mouse_position = GetMousePosition();
    bool is_mouse_over_right = false;
    if(IsCursorOnScreen()){
        if(mouse_position.x > resolution->width / 2){
            right_color.r = 200;
            is_mouse_over_right = true;
        }else {
            left_color.g = 200;
        }
    }
    #ifdef GAME_DEBUG
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        TraceLog(LOG_INFO, "Mouse button was clicked at %dx%d", (int)mouse_position.x, (int)mouse_position.y);
    }
    #endif // GAME_DEBUG


    
    if(!is_mouse_over_right && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
        *state = STATE_CPS_SCREEN;
        game_state->seconds = 0;
    }

    DrawRectangle(0, 0, resolution->width / 2, resolution->height, left_color);
    DrawRectangle(resolution->width / 2, 0, resolution->width / 2, resolution->height, right_color);
    Vector2 start_pos = {.x = resolution->width / 2, .y = 0};
    Vector2 end_pos = {.x = resolution->width / 2, .y = resolution->height};
    DrawLineEx(start_pos, end_pos, 2.5f, BLACK);
}

void draw_cps_screen(Resolution *resolution, Font *font, Game_State *game_state){
    const double max_seconds = 5.0;
    Color background_color = {0, 200, 0, 255};
    ClearBackground(background_color);
    const char *info_text = "Click the blue box as fast as you can to measure your clicks per second";
    Vector2 info_text_size = MeasureTextEx(*font, info_text, 24.0f, 0.0f);
    DrawTextEx(*font, info_text, (Vector2){resolution->width / 2 - info_text_size.x / 2, (resolution->height / 2) - 255}, 24.0f, 0, BLACK);
    
    Rectangle cps_rect = { // TODO: Make width/height dependant on resolution
        .height = 250, 
        .width = 450,
        .x = (resolution->width / 2) - 450 / 2,
        .y = resolution->height / 2
    }; 

    DrawRectangle(cps_rect.x, cps_rect.y, cps_rect.width, cps_rect.height, BLUE);
    Vector2 mouse_position = GetMousePosition();
    double time = GetTime();
    if(
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        mouse_position.x > cps_rect.x &&
        mouse_position.y > cps_rect.y &&
        mouse_position.x < cps_rect.x + cps_rect.width &&
        mouse_position.y < cps_rect.y + cps_rect.height
    ){
        if(game_state->total_clicks == 0 && game_state->from_time == 0.0){
            game_state->from_time = time;
        }

        if(game_state->clicks_per_second == 0.0)
            game_state->total_clicks++;
    }
    if(game_state->from_time - game_state->from_time > max_seconds && game_state->to_time == 0.0){
        game_state->to_time = time;
    }
    const char *total_clicks_text = TextFormat("Total clicks: %d", game_state->total_clicks);
    Vector2 total_clicks_text_size = MeasureTextEx(*font, total_clicks_text, 24.0f, 0.0f);
    DrawTextEx(
              *font,
              total_clicks_text,
              (Vector2){resolution->width / 2 - total_clicks_text_size.x / 2, cps_rect.height },
              24.0f, 0, BLACK
            );
    int seconds = time - game_state->from_time;
    if(seconds > max_seconds) seconds = max_seconds;
    const char *seconds_text = TextFormat("Seconds: %d", seconds);
    if(game_state->total_clicks > 0){
        DrawTextEx(
                   *font,
                   seconds_text,
                   (Vector2){resolution->width / 2 - MeasureTextEx(*font, seconds_text, 24.0f, 0.0f).x / 2, cps_rect.height + 24.0f},
                   24.0f, 0.0f, BLACK
    
        );
    }
    if(game_state->total_clicks > 0 && time - game_state->from_time > max_seconds && game_state->to_time == 0){
        game_state->to_time = time;
    }
    if(game_state->to_time > 0.0 && game_state->clicks_per_second == 0.0){
        game_state->clicks_per_second = game_state->total_clicks / (game_state->to_time - game_state->from_time);
    }
    if(game_state->clicks_per_second != 0){
        const char *clicks_per_second_text = TextFormat("Clicks per second: %.3f", game_state->clicks_per_second);
        Vector2 clicks_per_second_text_size = MeasureTextEx(*font, clicks_per_second_text, 24.0f, 0.0f);
        DrawTextEx(
            *font,
            clicks_per_second_text,
            (Vector2){resolution->width / 2 - clicks_per_second_text_size.x / 2, cps_rect.height + 48.0f},
            24.0f, 0.0f, BLACK);
    }


}


void initialize_game(Resolution *resolution){
    const int config_flags = FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN | FLAG_MSAA_4X_HINT;
    SetConfigFlags(config_flags);
    SetTargetFPS(1000);
    InitWindow(resolution->width, resolution->height, "Click Games");

}




int main(void)
{
    Screen_State screen_state = STATE_START_SCREEN;
    Resolution resolution = { .width = 1600, .height = 900};
    Previous_Resolution previous_resolution = {.resolution = resolution};
    initialize_game(&resolution);
    Font font = LoadFontEx("fonts/Iosevka-Regular.ttf", 24, NULL, 0);

    Game_State game_state;
    reset_game_state(&game_state);
    assert(IsFontValid(font));
    
    while(!WindowShouldClose()){
        if(IsWindowResized()){
            previous_resolution.resolution = resolution;
            resolution.width = GetRenderWidth();
            resolution.height = GetRenderHeight();
            TraceLog(LOG_INFO, "Resolution changed from %dx%d to %dx%d",
                                previous_resolution.resolution.width,
                                previous_resolution.resolution.height,
                                resolution.width, resolution.height);
            
        }
        if(IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER)){
                // if(IsWindowFullscreen()){
                //     resolution = previous_resolution.resolution;
                // }else {
                //     resolution.height = GetScreenHeight();
                //     resolution.width = GetScreenWidth();
                // }
                // ToggleFullscreen(); TODO: Fix fullscreen toggle
                
        }
        BeginDrawing();
            ClearBackground(BLACK);
            switch (screen_state)
            {
            case STATE_START_SCREEN:
                reset_game_state(&game_state);
                draw_start_screen(&resolution, &screen_state, &game_state);
                break;
            
            case STATE_CPS_SCREEN:
                draw_cps_screen(&resolution, &font, &game_state);
                draw_back_button(&resolution, &font, &screen_state);
            
            default:
                break;
            }


            #ifdef GAME_DEBUG
            
            draw_debug_info(&font, &resolution);
            #endif // GAME_DEBUG
        EndDrawing();
    }
    CloseWindow();
    return 0;
}