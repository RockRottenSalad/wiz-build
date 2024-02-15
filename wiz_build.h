#ifndef WIZARDRY_BUILD_H
#define WIZARDRY_BUILD_H

#ifndef RELEASE_BUILD

#include<stdlib.h>
#include<stdio.h>
#include<stdarg.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/wait.h>

// REPLACE ALL ASSERTS WITH OWN IMPLEMENTATION
#include<assert.h>

// MEMORY
// Just a scratch pad that gets malloc'ed
// The idea is it's faster to do one fat malloc and use that for
// everything rather than making seperate mallocs
// Some "frees" are made throughout by placing "free points" and
// moving the memory pointer back to that point
#define MAX_MEMORY 1024 * 4
static void* wiz_memory; 
static void* memory_ptr, *free_point;
static size_t max_alloc;
void* wiz_allocate(size_t size);
void wiz_alloc_reset();
void wiz_alloc_set_free_point(); // scratch within a scratch, 
void wiz_alloc_free_to_point();  // set a temporary point at the current mem pointer and roll back to it


// FILE OPERATIONS
enum file_operations_t { FILE_OLDER = 0, FILE_NEWER, FILE_VALID_FORMAT, FILE_INVALID_FORMAT };
typedef enum file_operations_t file_operations_t;

typedef struct
{
    char* file_path; // relative
    struct stat file_info;
} file_t;
file_t file_init(const char* file_path);
file_t file_init_stat_only(char* file_path); // file_path is already allocated
void file_update_stat(file_t* file);
file_operations_t file_compare_date(file_t* file_a, file_t* file_b);
char* file_get_format(char* file_path); // from path

typedef struct
{
    char* dir_path;
    DIR* dir;
    file_t* files;
    size_t len; // length of files array
} dir_t;
// va args for file formats to look for e.g. .h .c .cpp
#define dir_init(DIR_PATH)\
    dir_init_wrapped(DIR_PATH)
dir_t dir_init_wrapped(const char* dir_path);


// SHELL COMMANDS
typedef struct
{
    char* command;
    char** args; // NOTE: args[0] == command, args come after, NULL terminated
    size_t len; // number of of args
} command_t;

command_t command_init(const char* command, ...); 
void command_append_arg(command_t* command, command_t command_to_append);
void command_execute(command_t command);

// Welcome to macro land
#define MAKE_CMD(...)\
    command_init(__VA_ARGS__, NULL)
#define EXEC_CMD(COMMAND)\
    command_execute(COMMAND)
#define CMD_APPEND(COMMAND, ...)\
    command_append_arg(COMMAND, MAKE_CMD(__VA_ARGS__), NULL)
#define CMD(...)\
do{\
    wiz_alloc_set_free_point();\
    command_t COMMAND = MAKE_CMD(__VA_ARGS__);\
    command_execute( COMMAND );\
    wiz_alloc_free_to_point();\
}while(0)

//TODO: Currently only operates on .c and .h
//Need to make it possible to specify file formats(and wildcard)
// NOTE: Little hacky with the mem pointer for reduced memory usage
// Also using MAKE_CMD instead of CMD to not override free point
#define CMD_FOR_FILE_IN_DIR(DIR_PATH, ...)\
do{\
    wiz_alloc_set_free_point();\
    dir_t DIR = dir_init(DIR_PATH);\
    command_t COMMAND = {0};\
    void* ptr = NULL;\
    for(size_t i = 0; i < DIR.len; i++)\
    {\
        ptr = memory_ptr;\
        COMMAND = MAKE_CMD(__VA_ARGS__, DIR.files[i].file_path);\
        EXEC_CMD(COMMAND);\
        memory_ptr = ptr;\
    }\
    wiz_alloc_free_to_point();\
    break;\
}while(0)

// Strictly for debugging wiz_build
#define PRINT_CMD(COMMAND)\
    for(size_t i = 0; i < COMMAND.len; i++)\
    {\
        fprintf(stderr, "%s ", COMMAND.args[i]);\
    }

#define BIN(BIN_NAME)\
    "/usr/bin/" BIN_NAME

#define NULL_TERMINATOR 1

#ifndef COMPILER
#define COMPILER "clang"
#endif

#define STRCPY(DEST, SRC)\
    DEST = wiz_allocate(strlen(SRC) + NULL_TERMINATOR);\
    assert(DEST != NULL);\
    (void)memcpy(DEST, SRC, strlen(SRC) + 1);

#define STRCAT(DEST, SRC)\
({\
    size_t DEST_LEN = strlen(DEST);\
    size_t SRC_LEN = strlen(SRC);\
    char* NEW_STR = wiz_allocate(DEST_LEN + SRC_LEN + NULL_TERMINATOR);\
    (void)memcpy(NEW_STR, DEST, DEST_LEN);\
    (void)memcpy(NEW_STR + DEST_LEN, SRC, SRC_LEN);\
    NEW_STR[DEST_LEN + SRC_LEN] = '\0';\
    NEW_STR;\
})

#define STRDUP(STRING)\
({\
    size_t LEN = strlen(STRING);\
    char* NEW_STR = wiz_allocate(LEN + NULL_TERMINATOR);\
    (void)memcpy(NEW_STR, STRING, LEN);\
    NEW_STR;\
})

#define COUNT_VA_ARGS(VA_LIST, FIRST)\
{\
    size_t count = 0;\
    va_start(VA_LIST, FIRST);\
    for(char* arg = va_arg(VA_LIST); arg != NULL; arg = va_arg(VA_LIST, char*))\
    { count++; }\
    va_end(VA_LIST);\
    count;\
}

// LOGGING
#define RESET_COLOR         "\033[0m"

#define FG_BRIGHT_GRAY      "\033[0;90m"
#define FG_BRIGHT_RED       "\033[0;91m"
#define FG_BRIGHT_GREEN     "\033[0;92m"
#define FG_BRIGHT_YELLOW    "\033[0;93m"
#define FG_BRIGHT_BLUE      "\033[0;94m"
#define FG_BRIGHT_MAGENTA   "\033[0;95m"
#define FG_BRIGHT_CYAN      "\033[0;96m"
#define FG_BRIGHT_WHITE     "\033[0;97m"

#define BLUE(FILE_NAME)\
    FG_BRIGHT_BLUE FILE_NAME RESET_COLOR
#define RED(FILE_NAME)\
    FG_BRIGHT_RED FILE_NAME RESET_COLOR
#define YELLOW(FILE_NAME)\
    FG_BRIGHT_YELLOW FILE_NAME RESET_COLOR
#define GREEN(FILE_NAME)\
    FG_BRIGHT_GREEN FILE_NAME RESET_COLOR
#define MAGENTA(FILE_NAME)\
    FG_BRIGHT_MAGENTA FILE_NAME RESET_COLOR
#define CYAN(FILE_NAME)\
    FG_BRIGHT_CYAN FILE_NAME RESET_COLOR

#define SET_COLOR(COLOR) printf(COLOR)
#define LOG(...)\
    fprintf(stderr, FG_BRIGHT_WHITE "|WIZ-LOG| " RESET_COLOR __VA_ARGS__);\
    fprintf(stderr, "\n")
#define WARN(...)\
    fprintf(stderr, FG_BRIGHT_YELLOW "|WIZ-LOG| " RESET_COLOR __VA_ARGS__);\
    fprintf(stderr, "\n")
#define ERROR(...)\
    fprintf(stderr, FG_BRIGHT_RED "|WIZ-LOG| " RESET_COLOR __VA_ARGS__);\
    fprintf(stderr, "\n")

// wiz_build self-rebuild + alloc scratch
void wiz_build_init(int argc, char **argv);
void wiz_build_deinit();

#define PANIC(...)\
    ERROR(__VA_ARGS__);\
    wiz_build_deinit();


// ## Function definitions

void* wiz_allocate(size_t size)
{
    // TODO: Replace with own implementation
    assert((memory_ptr + size) <= (wiz_memory + (size_t)(MAX_MEMORY)));

    void* alloc = memory_ptr;
    memory_ptr += size;

    if((size_t)(memory_ptr - wiz_memory) > max_alloc)
        max_alloc = memory_ptr - wiz_memory;

    return alloc;
}

void wiz_alloc_reset()
{
    memory_ptr = wiz_memory;
}

void wiz_alloc_set_free_point()
{
    free_point = memory_ptr;
}
void wiz_alloc_free_to_point()
{
    assert(free_point != NULL);
    memory_ptr = free_point;
}

file_t file_init(const char* file_path)
{
    file_t new_file = {0};
    size_t file_name_len = strlen(file_path) + NULL_TERMINATOR;
    int result = 0;
    void *alloc = NULL;

    result = stat(file_path, &new_file.file_info);
    assert(result == 0);

    alloc = wiz_allocate(file_name_len);
    assert(alloc != NULL);
    new_file.file_path = alloc;

    (void)memcpy(alloc, file_path, file_name_len);

    return new_file;
}

file_t file_init_stat_only(char* file_path)
{
    file_t new_file = {0};
    int result = 0;

    result = stat(file_path, &new_file.file_info);
    assert(result == 0);

    new_file.file_path = file_path;

    return new_file;
}

// Compares last modified date
file_operations_t
file_compare_date(file_t* file_a, file_t* file_b)
{
    // stat -> status modified time -> time spec.tv_sec(onds) 
    long time_a = file_a->file_info.st_mtim.tv_sec;
    long time_b = file_b->file_info.st_mtim.tv_sec;

//    fprintf(stderr, "a-bin: %ld, b-src: %ld\n", time_a, time_b);
    if(time_a > time_b)
        return FILE_OLDER;
    return FILE_NEWER;
}

void file_update_stat(file_t* file)
{
    assert( stat(file->file_path, &file->file_info) == 0 );
}

char* file_get_format(char* file_path)
{
    char* substr = strtok(file_path, ".");
    char* prev_substr = substr;

    while((substr = strtok(NULL, ".")) != NULL)
        prev_substr = substr;
    return prev_substr;
}

// TODO: Make it recursive and add file formats as argument
dir_t dir_init_wrapped(const char* dir_path)
{
    dir_t new_dir = {0};
    struct dirent *current_file = NULL;

    char* file_format = NULL;
    file_t new_file = {0};
    size_t max_files = 0, files = 0;

    //TODO: handle this case v
//    if(dir_path[strlen(dir_path)] != '/');

    new_dir.dir = opendir(dir_path);
    assert(new_dir.dir != NULL);

    while((current_file = readdir(new_dir.dir)) != NULL)
    {
        if(current_file->d_name[0] == '.')
            continue; 
        max_files++;
    }
    rewinddir(new_dir.dir);

    // TODO: better way to approximate max files
    new_dir.files = wiz_allocate(max_files * sizeof(file_t));

    while((current_file = readdir(new_dir.dir)) != NULL)
    {
        if(current_file->d_name[0] == '.')
            continue; 
        // TODO: cut down memory usage by passing reference
        file_format = file_get_format(STRDUP(current_file->d_name));
        // TODO: support for custom file types via VA_LIST
        if(file_format[0] == 'c' || file_format[0] == 'h')
        {
            new_file = file_init_stat_only(STRCAT(dir_path, current_file->d_name));
            new_dir.files[files] = new_file;
            files++;
        }
    }

    assert(closedir(new_dir.dir) == 0);
    new_dir.len = files;

    return new_dir;
}


command_t command_init(const char* command, ...)
{
    command_t new_command = {0};

    va_list arguments;

    // TODO: make this a macro
    size_t va_list_len = 0; 
    va_start(arguments, command);
    for(char* arg = va_arg(arguments, char*); arg != NULL; arg = va_arg(arguments, char*))
    {va_list_len++;}
    va_end(arguments);

    // +1 to account for command also needing to be in args[0]
    // +1 to account for NULL terminator
    new_command.args = wiz_allocate(sizeof(char*) * (va_list_len + NULL_TERMINATOR + 1));

    STRCPY(new_command.command, command);
    STRCPY(new_command.args[0], command);
    new_command.len++;

    va_start(arguments, command);

    for(char* arg = va_arg(arguments, char*);
        arg != NULL;
        arg = va_arg(arguments, char*))
    {
        STRCPY(new_command.args[new_command.len], arg);
        new_command.len++;
    }

    va_end(arguments);
    
    // execvp() expects last element in argv to be NULL
    new_command.args[new_command.len] = NULL;

    return new_command;
}

void command_append_arg(command_t* command, command_t command_to_append)
{
    size_t args_buffer_size = 
        (command->len + command_to_append.len + NULL_TERMINATOR)
            * (sizeof(char*) );

    char** args_buffer = wiz_allocate(args_buffer_size);

    for(size_t i = 0; i < command->len; i++)
    {
        (void)memcpy(
                args_buffer[i], 
                command->args[i],
                strlen(command->args[i])
                );
    }

    for(size_t i = 0; i <= command_to_append.len; i++)
    {
        (void)memcpy(
                command->args[command->len+i],
                command_to_append.args[i], 
                strlen(command_to_append.args[i])
                );
    }

    command->len += command_to_append.len;
    command->args[command->len] = NULL;
}

void command_execute(command_t command)
{
//    PRINT_CMD(command);
    pid_t pid = fork();
    int status = 0;
    assert(pid >= 0);

    // Parent waits for child
    if(pid == 0)
    {
        assert(execvp(command.command, command.args) != -1);
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    } 
    else{
        // TODO: check for error
        assert(waitpid(pid, &status, 0) >= 0);
    }
}

void wiz_build_init(int argc, char **argv)
{
    wiz_memory = malloc(MAX_MEMORY);
    max_alloc = 0;
//     TODO: Replace with own implementation
    assert(wiz_memory != NULL);
    memory_ptr = wiz_memory;

    file_t bin_file = file_init(argv[0]);
    file_t src_file = file_init(STRCAT(argv[0], ".c"));
    
    file_operations_t result = file_compare_date(&bin_file, &src_file);

    if(result == FILE_NEWER)
    {
        long last_modified = bin_file.file_info.st_mtim.tv_sec;

        LOG("Changes in " BLUE("%s") " detected, " YELLOW("RECOMPILING..."), src_file.file_path);
        CMD(BIN(COMPILER), "-g", src_file.file_path, "-o", argv[0]); 

        file_update_stat(&bin_file); // If bin file hasn't been modified, then we'll assume the compilation failed
        if(last_modified == bin_file.file_info.st_mtim.tv_sec)
        {
            LOG("Compilation of " BLUE("%s") " has " RED("FAILED"), src_file.file_path);
            wiz_build_deinit();
        }else
        {
            // TODO: Proper timer
            LOG("Compilation of " BLUE("%s") " has " GREEN("SUCCEEDED") " in " MAGENTA("%ld") "s", src_file.file_path, last_modified - bin_file.file_info.st_mtim.tv_sec);
            (void)execl(argv[0], argv[0]);
        }
        //            (void)execvp(argv[0], argv[0]);
    }
    wiz_alloc_reset();
    // TEST DIR
    dir_t dir_test = dir_init("./");

    LOG("File from dir reader test: " BLUE("%s") " found in dir", dir_test.files[0].file_path);
    LOG("File from dir reader test: " BLUE("%s") " found in dir", dir_test.files[1].file_path);
}

void wiz_build_deinit()
{
    size_t used_memory = (size_t)(memory_ptr - wiz_memory);
    free(wiz_memory);
    wiz_memory = NULL;

    LOG(YELLOW("%zu/%d") " peak bytes allocated", max_alloc, MAX_MEMORY);
    LOG(YELLOW("%zu/%d") " bytes allocated on exit", used_memory, MAX_MEMORY);
    exit(0);
}

#else /* RELEASE_BUILD */

// Macros are defined to have a blank output.
// This strips the program of any wiz_build function
// TODO: wrap wiz_build_init in a macro and add it here
// TODO: also add the other macros
#define MAKE_CMD(...)
#define CMD_APPEND(COMMAND, ...)
#define CMD(...)
#define PRINT_CMD(COMMAND)


#endif /* RELEASE_BUID */

#endif /* WIZARDRY_BUILD_H */
