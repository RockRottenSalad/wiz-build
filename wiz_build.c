
#define COMPILER "clang"

#define DEBUG_FLAGS "-g", "-march=native", "-Wall", "-Wextra"
#define RELEASE_FLAGS "-O3", "-Wall", "-Wextra"

#define LD "-lm"

#define MAIN_SRC "main.c"
#define OUTPUT "./main"

#define SRC "./src"

#include "wiz_build.h"

void link_objects(size_t files)
{
        command_t command = {0};
        command.args = wiz_allocate(sizeof(char*) * (files+10));
        size_t index = 1;

        STRCPY(command.args[0], BIN(COMPILER));
        command.command = command.args[0];

        FOR_FILE_IN_DIR("./", WHERE(FILE_FORMAT("o")), CMD(BIN("mv"), FILE_PATH, "./obj"));
        size_t obj_files = FOR_FILE_IN_DIR("./obj", WHERE(FILE_FORMAT("o")),
                            STRCPY(command.args[index], FILE_PATH); index++);
        ASSERT(obj_files == 3);

        STRCPY(command.args[index], LD);
        index++;
        STRCPY(command.args[index], "-o");
        index++;
        STRCPY(command.args[index], OUTPUT);
        index++;
        command.args[index] = NULL;

        LOG("Changes found, " YELLOW("RELINKING..."));

        SET_COMPILE_TARGET(OUTPUT);
        command_execute(command);
        CHECK_COMPILE_STATUS();
}

void rebuild_changed()
{
    size_t files = FOR_FILE_IN_DIR(SRC,
                    WHERE( FILE_FORMAT("c") && FILE_NEWER_THAN(OUTPUT) ),
                     CMD(BIN(COMPILER), DEBUG_FLAGS, "-c", FILE_PATH);
                     LOG( "Changes in " BLUE("%s") " detected, " YELLOW("RECOMPILING..."), FILE_PATH);
                     );
    if(files > 0)
        link_objects(files);
    else{ LOG("Already " GREEN("up to date!")); }
}

void rebuild()
{
    size_t files = FOR_FILE_IN_DIR(SRC,
                    WHERE( FILE_FORMAT("c") ),
                     CMD(BIN(COMPILER), DEBUG_FLAGS, "-c", FILE_PATH);
                     LOG(BLUE("%s") " found, " YELLOW("RECOMPILING..."), FILE_PATH);
                     );
    if(files > 0)
        link_objects(files);
    else{ LOG("Already " GREEN("up to date!")); }
}

int main(int argc, char** argv)
{
    WIZ_BUILD_INIT(argc, argv);  // Build tool will recompile itself & restart here

    if(FILE_EXISTS("./main") != 0)
        TOUCH("main");

    LOG("Rebuilding changed source files in " BLUE(SRC "..."));

    if(argc < 2)
    {
        rebuild_changed();
    }else if(argc > 2 && strcmp(argv[1], "rebuild"))
    {
        rebuild();
    }
    

    WIZ_BUILD_DEINIT();
    return 0;
}


