// nob.c
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif // _WIN32


#define EXECUTABLE_NAME "click_games"



// Some of this code is taken from https://github.com/tsoding/musializer


#ifdef _WIN32
#define DEFAULT_COMPILER "gcc"
#else
#define DEFAULT_COMPILER "cc"
#endif // _WIN32

#define BUILD_PATH "build"

static const char *raylib_modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};


void build_raylib(void)
{
    Nob_Cmd cmd = {0};

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = nob_temp_sprintf("./deps/raylib/src/%s.c", raylib_modules[i]);
        const char *output_path = nob_temp_sprintf("%s/%s.o", BUILD_PATH, raylib_modules[i]);

        if (nob_needs_rebuild(output_path, &input_path, 1)) {
            nob_cmd_append(&cmd, DEFAULT_COMPILER,
                "-DPLATFORM_DESKTOP",
                "-I./deps/raylib/src/external/glfw/include/",
                "-c", input_path,
                "-o", output_path);
            #ifdef __linux__
                nob_cmd_append(&cmd, "-D_GLFW_X11");
            #endif // __linux__
            nob_cmd_run_sync_and_reset(&cmd); // TODO: Make this multithreaded. 
                                              // There were some issues before so I had to revert to single-thread. 
            
        
        }
        
    }
    nob_cmd_append(&cmd, "ld", "-r", "-o", "build/libraylib.a");
    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = nob_temp_sprintf("%s/%s.o", BUILD_PATH, raylib_modules[i]);
        nob_cmd_append(&cmd, input_path);
    }
    nob_cmd_run_sync_and_reset(&cmd);

    
}

void clean_build_files(void){
    for(size_t i = 0; i < sizeof(raylib_modules) / 8; i++){
        const char *path = nob_temp_sprintf(BUILD_PATH"/%s.o", raylib_modules[i]);
        if(nob_file_exists(path)){
            nob_delete_file(path);

        }

    }
    if(nob_file_exists(BUILD_PATH"/libraylib.a"))
    nob_delete_file(BUILD_PATH"/libraylib.a");
#ifdef _WIN32
    if(nob_file_exists(BUILD_PATH"/"EXECUTABLE_NAME".exe"))
        nob_delete_file(BUILD_PATH"/"EXECUTABLE_NAME".exe");
#else
    if(nob_file_exists(BUILD_PATH"/"EXECUTABLE_NAME))
        nob_delete_file(BUILD_PATH"/"EXECUTABLE_NAME);
#endif // _WIN32
    nob_delete_directory(BUILD_PATH); // TODO: Implement nob_recursively_delete_directory in nob.h

}

typedef struct argument_options {
    bool cleanup;
    bool rebuild;
    bool debug;
    bool werror;

} argument_options;

int main(int argc, char **argv)
{

    NOB_GO_REBUILD_URSELF(argc, argv);

    argument_options arg_options = {0};
    const char *program_name = nob_shift(argv, argc);
    bool invalid_options = false;

  
        while (argc > 0){
            const char *argument = nob_shift(argv, argc);
            
            if(!strcmp(argument, "--rebuild") || !strcmp(argument, "-r")){
                arg_options.rebuild = true;
            }else if(!strcmp(argument, "--clean") || !strcmp(argument, "-c")){
                arg_options.cleanup = true;
            }else if(!strcmp(argument, "--debug") || !strcmp(argument, "-d")){
                arg_options.debug = true;
            }else if(!strcmp(argument, "--werror")){
                arg_options.werror = true;
            }else if(!strcmp(argument, "--help")) {
                printf("Usage: %s [options]\n", program_name);
                printf("-r, --rebuild:\n");
                printf("\tDeletes the build folder and rebuilds the whole project including dependencies\n");
                printf("-c, --clean:\n");
                printf("\tDeletes the build folder, then exits\n");
                printf("-d, --debug:\n");
                printf("\tEnables debug version of the game\n");
                printf("--werror:\n");
                printf("\tAll warnings are treated as errors\n");
                printf("--run:\n");
                printf("\tAttempts to run the program after compiling. [NOT IMPLEMENTED]\n");
                printf("--help:\n");
                printf("\tShow this list with a list over all arguments, then exits\n");
                return 0;
            }else {
                printf("Argument '%s' is not a valid argument\n", argument);
                invalid_options = true;
            }
        }
    if(invalid_options){
        printf("See '%s --help' for a list over all arguments\n", program_name);
        return 1;
    }
    



    if(arg_options.cleanup){
        clean_build_files();
        return 0;
    }
    if(arg_options.rebuild){
        clean_build_files();
    }

    
    if(!nob_mkdir_if_not_exists(BUILD_PATH)) return 1;

    build_raylib();
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd,
            DEFAULT_COMPILER,
            "-Wall","-Wextra", "-Wpedantic",
            "-o", "build/"EXECUTABLE_NAME, "src/main.c",
            "-I", "deps/raylib/src",
            "-L", "build/",
            "-lraylib", "-lm",
            "-Werror=empty-body",
            "-Werror=misleading-indentation"
            );
#ifdef _WIN32
    nob_cmd_append(&cmd, "-lgdi32", "-lwinmm"); 
#endif // _WIN32
    if(arg_options.debug){
        nob_cmd_append(&cmd, "-DGAME_DEBUG");
    }
    if(arg_options.werror){
        nob_cmd_append(&cmd, "-Werror");
    }

    if (!nob_cmd_run_sync(cmd)) return 1;
    return 0;
}
