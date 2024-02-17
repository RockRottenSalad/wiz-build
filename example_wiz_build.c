/* WIZ_BUILD example
 * 
 * Simple demonstration of what a wiz_build file can look like.
 * Note that commands like MAKE_CMD allocate memory from a 
 * scratch allocator with 4 kB available. 
 *
 * This is to ensure that configs stay simple.
 * (And to force myself to optimize CMD_APPEND because that's a really big memory hog right now)
 *
 */

// Define compiler and arguments before include
#define COMPILER "clang"

#define DEBUG_FLAGS "-Wall", "-Wextra", "-g", "-I", "./lib/"
#define OUTPUT "./bin/main"

#include<wiz_build.h>

int main(int argc, char** argv)
{
    // Wiz build will recompile itself if wiz_build.c has been modified
    WIZ_BUILD_INIT(argc, argv);

    // MAKE_CMD can be used for constructing commands, there also exists CMD()
    // for quickly running shell commands
    command_t compile = MAKE_CMD(BIN(COMPILER), DEBUG_FLAGS, "-o", OUTPUT);

    // Loops through directory(not recursive), filters out files
    // after condition specificed inside WHERE(). Conditions include
    // FILE_FORMAT(), FILE_NEWER_THAN(), FILE_OLDER_THAN().
    //
    // Using standard functions e.g. strcmp is also possible.
    // Furthermore, FILE_PATH and FILE_NAME are valid variables
    // within the scope of the for loop.
    FOR_FILE_IN_DIR("./src/", WHERE( FILE_FORMAT("c") ),
            CMD_APPEND(&compile, FILE_PATH);
            LOG("Adding " BLUE("%s") " to build", FILE_NAME); // Multiple commands can be added in a single loop
            );

    // Allows wiz_build to check whether if the compilation failed or not 
    SET_COMPILE_TARGET(OUTPUT);

    // Executes the previously assembled command
    EXEC_CMD(compile);

    // Prints whether if compilation failed or not to stderr
    CHECK_COMPILE_STATUS();

    LOG("Running " OUTPUT);

    // Run compiled output
    CMD(OUTPUT);
   
    // Cleanup
    WIZ_BUILD_DEINIT();
    return 0;
}

