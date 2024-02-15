
// For re-building the build tool
#define COMPILER "clang"
#define DEBUG_FLAGS "-g", "-march=native", "-Wall", "-Wextra"

#include "wiz_build.h"

int main(int argc, char** argv)
{
    // NOTE: Program will recompile itself here if
    // changes have been made to the source file
    wiz_build_init(argc, argv); 

    LOG("Using for file in dir loop with cat command");

    // Provides each file in dir as arg for cat
    // NOTE: Currently only searches for .c and .h files
    CMD_FOR_FILE_IN_DIR("./testdir/", BIN("cat"), "-n");

    LOG("This is a normal log");
    WARN("This is a warning log");
    ERROR("This is an error log");

    LOG("Better mem now??? even better maybe?");

    wiz_build_deinit();
    return 0;
}


