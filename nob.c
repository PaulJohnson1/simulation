#include "cjson.h"
#include <stdlib.h>
#include <string.h>
#define NOB_IMPLEMENTATION
#include "cjson.c"
#include "nob.h"

// #define DEBUG
#define RELEASE
// #define PROFILE

// #define CLANG
#define GCC

#define PGO
#define BENCHMARK

// #define COLLISION_MANAGER BRUTE_FORCE
#define COLLISION_MANAGER HASH_GRID
// #define COLLISION_MANAGER NAIVE_HASH_GRID
// #define COLLISION_MANAGER QUADTREE

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define BASE_FLAGS(X)                                                          \
    X("-Werror")                                                               \
    X("-Wall")                                                                 \
    X("-Wextra")                                                               \
    X("-Wconversion")                                                          \
    X("-Wno-empty-translation-unit")                                           \
    X("-Wno-unused-function")                                                  \
    X("-Wno-gnu-empty-struct")                                                 \
    X("-Isrc")                                                                 \
    X("-DTMP_USE_" STRINGIFY(COLLISION_MANAGER))

#define BASE_CFLAGS(X)
#define BASE_CCFLAGS(X) X("-std=c++20")
#define BASE_LFLAGS(X)                                                         \
    X("-lSDL3")                                                                \
    X("-lm")                                                                   \
    X("-lGL")

#define RELEASE_CFLAGS(X)                                                      \
    X("-O3")                                                                   \
    X("-march=native")                                                         \
    X("-mtune=native")                                                         \
    X("-DNDEBUG")

#define RELEASE_LFLAGS(X) X("-flto")

#ifdef GCC
#define CC "gcc"
#define CXX "g++"
#define PROFILE_GENERATE_LFLAGS(X) X("-fprofile-generate")
#define PROFILE_GENERATE_CFLAGS(X) X("-fprofile-generate")
#define PROFILE_USE_LFLAGS(X) X("-fprofile-use")
#define PROFILE_USE_CFLAGS(X) X("-fprofile-use")
#endif

#ifdef CLANG
#define CC "clang"
#define CXX "clang++"
#define PROFILE_GENERATE_LFLAGS(X) X("-fprofile-instr-generate")
#define PROFILE_GENERATE_CFLAGS(X) X("-fprofile-instr-generate")
#define PROFILE_USE_LFLAGS(X) X("-fprofile-instr-use")
#define PROFILE_USE_CFLAGS(X) X("-fprofile-instr-use")
#endif

#define DEBUG_CFLAGS(X)                                                        \
    X("-g")                                                                    \
    X("-DDEBUG")

#define PROFILE_CFLAGS(X)                                                      \
    X("-gline-tables-only")                                                    \
    X("-fno-inline")                                                           \
    X("-O3")                                                                   \
    X("-DNDEBUG")

#define CLANG_ONLY(...)
#define GCC_ONLY(...)
#define DEBUG_ONLY(...)
#define RELEASE_ONLY(...)
#define PGO_ONLY(...)
#define PROFILE_ONLY(...)

#ifdef CLANG
#undef CLANG_ONLY
#define CLANG_ONLY(...) __VA_ARGS__
#endif
#ifdef GCC
#undef GCC_ONLY
#define GCC_ONLY(...) __VA_ARGS__
#endif
#ifdef PGO
#undef PGO_ONLY
#define PGO_ONLY(...) __VA_ARGS__
#endif

#ifdef DEBUG
#undef DEBUG_ONLY
#define DEBUG_ONLY(...) __VA_ARGS__
#endif

#ifdef RELEASE
#undef RELEASE_ONLY
#define RELEASE_ONLY(...) __VA_ARGS__
#endif

#ifdef PROFILE
#undef PROFILE_ONLY
#define PROFILE_ONLY(...) __VA_ARGS__
#endif

struct compilation_target_file
{
    char const *name;
    char const *suffix;
};

struct compilation_target_file files[] = {
    {"Collision/BruteForce", "c"},
    {"Collision/Quadtree", "c"},
    {"Collision/HashGrid", "c"},
    {"Collision/NaiveHashGrid", "c"},
    {"Ball", "c"},
    {"Simulation", "c"},
    {"Utilities", "c"},
    {"Vector", "c"},
    {"Window", "c"},
#ifdef BENCHMARK
    {"Benchmark", "cc"},
#else
    {"Main", "c"},
#endif
};

struct compilation_target
{
    struct compilation_target_file file;
    char const *compiler;
    cjson_vector_declare(char const *, flags);
};

char *str_replace(char *str, char find, char replace, uint64_t start_from)
{
    for (; start_from[str]; str++)
        if (start_from[str] == find)
            start_from[str] = replace;
    return str;
}

int write_to_file(const char *file_name, const char *contents)
{
    FILE *fp = fopen(file_name, "w");
    size_t len = strlen(contents);
    fwrite(contents, 1, len, fp);
    fclose(fp);

    return 0;
}

struct make_executable_options
{
    uint8_t profile_generate;
    uint8_t profile_use;
    uint8_t release;
    uint8_t debug;
    uint8_t profile;
    uint8_t populate_compile_commands;
};

struct cjson_value compile_commands = {.type = cjson_value_type_array};

int make_object(struct compilation_target *target)
{
    Nob_Cmd cmd = {0};
    char source[1024] = SRC_DIR;
    char object[1024] = BUILD_DIR;

    strcat(source, target->file.name);
    strcat(source, ".");
    strcat(source, target->file.suffix);
    strcat(object, target->file.name);
    strcat(object, ".o");
    str_replace(object, '/', '.', sizeof BUILD_DIR);

    nob_cmd_append(&cmd, target->compiler);
    nob_cmd_append(&cmd, source, "-o", object, "-c");

    for (char const **i = target->flags; i < target->flags_end; i++)
        nob_cmd_append(&cmd, *i);
    if (!nob_cmd_run_sync(cmd))
        return 1;
    return 0;
}

// this is so ugly
void append_object_to_compile_commands(struct compilation_target *target)
{
#define DIRECTORY_PATH "/home/paul/Documents/programming/physics"
#define DIRECTORY DIRECTORY_PATH "/"
    struct cjson_value *entry = calloc(1, sizeof *entry);
    entry->type = cjson_value_type_object;

    // directory
    {
        cjson_vector_grow(struct cjson_object_entry, entry->object.values);
        memset(entry->object.values_end, 0, sizeof *entry->object.values);
        entry->object.values_end->name = strdup("directory");
        struct cjson_value *directory_value =
            calloc(1, sizeof *directory_value);
        directory_value->type = cjson_value_type_string;
        directory_value->string.value = strdup(DIRECTORY_PATH);
        entry->object.values_end++->value = directory_value;
    }

    // file
    {
        cjson_vector_grow(struct cjson_object_entry, entry->object.values);
        memset(entry->object.values_end, 0, sizeof *entry->object.values);
        entry->object.values_end->name = strdup("file");
        struct cjson_value *file_value = calloc(1, sizeof *file_value);
        file_value->type = cjson_value_type_string;

        cjson_vector_declare_zero(char, file_name);
        cjson_vector_reserve(char, strlen(DIRECTORY), file_name);
        memcpy(file_name_end, DIRECTORY, strlen(DIRECTORY));
        file_name_end += strlen(DIRECTORY);

        uint64_t file_name_len = strlen(target->file.name);
        cjson_vector_reserve(char, file_name_len, file_name);
        memcpy(file_name_end, target->file.name, file_name_len);
        file_name_end += file_name_len;

        cjson_vector_grow(char, file_name);
        *file_name_end++ = '.';

        uint64_t file_suffix_len = strlen(target->file.suffix);
        cjson_vector_reserve(char, file_suffix_len, file_name);
        memcpy(file_name_end, target->file.suffix, file_suffix_len);
        file_name_end += file_suffix_len;

        cjson_vector_grow(char, file_name);
        *file_name_end++ = 0;

        file_value->string.value = file_name;
        entry->object.values_end++->value = file_value;
    }

    // output
    {
        cjson_vector_grow(struct cjson_object_entry, entry->object.values);
        memset(entry->object.values_end, 0, sizeof *entry->object.values);
        entry->object.values_end->name = strdup("output");
        struct cjson_value *output_value = calloc(1, sizeof *output_value);
        output_value->type = cjson_value_type_string;

        cjson_vector_declare_zero(char, output_name);
        cjson_vector_reserve(char, strlen(DIRECTORY), output_name);
        memcpy(output_name, DIRECTORY, strlen(DIRECTORY));
        output_name_end += strlen(DIRECTORY);

        uint64_t file_name_len = strlen(target->file.name);
        cjson_vector_reserve(char, file_name_len, output_name);
        memcpy(output_name_end, target->file.name, file_name_len);
        output_name_end += file_name_len;

        cjson_vector_grow(char, output_name);
        *output_name_end++ = '.';
        cjson_vector_grow(char, output_name);
        *output_name_end++ = 'o';
        cjson_vector_grow(char, output_name);
        *output_name_end++ = 0;

        str_replace(output_name, '/', '.', sizeof DIRECTORY BUILD_DIR);

        output_value->string.value = output_name;
        entry->object.values_end++->value = output_value;
    }

    // arguments
    {
        cjson_vector_grow(struct cjson_object_entry, entry->object.values);
        memset(entry->object.values_end, 0, sizeof *entry->object.values);
        entry->object.values_end->name = strdup("arguments");
        struct cjson_value *arguments_value =
            calloc(1, sizeof *arguments_value);
        arguments_value->type = cjson_value_type_array;

        for (char const **i = target->flags; i < target->flags_end; i++)
        {
            struct cjson_value *argument_value =
                calloc(1, sizeof *argument_value);
            argument_value->type = cjson_value_type_string;
            argument_value->string.value = *i;
            cjson_vector_grow(struct cjson_value *,
                              arguments_value->array.values);
            *arguments_value->array.values_end++ = argument_value;
        }
        entry->object.values_end++->value = arguments_value;
    }
#undef DIRECTORY
#undef DIRECTORY_SLASH

    cjson_vector_grow(struct cjson_value *, compile_commands.array.values);
    *compile_commands.array.values_end++ = entry;
}

int make_executable(struct make_executable_options *options)
{
    for (uint64_t i = 0; i < NOB_ARRAY_LEN(files); i++)
    {
        struct compilation_target target = {0};
        target.compiler = CC;
        target.file = files[i];
#define append_flag(flag)                                                      \
    cjson_vector_grow(char const *, target.flags);                             \
    *target.flags_end++ = flag;

        BASE_FLAGS(append_flag);
        if (options->release)
        {
            RELEASE_CFLAGS(append_flag);
        }
        if (options->debug)
        {
            DEBUG_CFLAGS(append_flag);
        }
        if (options->profile_use)
        {
            PROFILE_USE_CFLAGS(append_flag);
        }
        if (options->profile_generate)
        {
            PROFILE_GENERATE_CFLAGS(append_flag);
        }
        if (options->profile_use)
        {
            PROFILE_USE_CFLAGS(append_flag);
        }
        if (options->profile)
        {
            PROFILE_CFLAGS(append_flag);
        }
#undef append_flag
        if (options->populate_compile_commands)
            append_object_to_compile_commands(&target);
        if (make_object(&target))
            return 1;
    }

    if (options->populate_compile_commands)
        write_to_file("compile_commands.json",
                      cjson_value_stringify(&compile_commands));

    // link
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CXX, "-o", "main");

    for (uint64_t i = 0; i < NOB_ARRAY_LEN(files); i++)
    {
        char object[1024] = BUILD_DIR;
        strcat(object, files[i].name);
        strcat(object, ".o");
        str_replace(object, '/', '.', sizeof BUILD_DIR);
        nob_cmd_append(&cmd, strdup(object));
    }

#define append_flag(flag) nob_cmd_append(&cmd, flag);

    BASE_LFLAGS(append_flag);
    if (options->release)
    {
        RELEASE_LFLAGS(append_flag);
    }
    if (options->profile_generate)
    {
        PROFILE_GENERATE_LFLAGS(append_flag);
    }
    if (options->profile_use)
    {
        PROFILE_USE_LFLAGS(append_flag);
    }
#undef append_flag
    if (!nob_cmd_run_sync(cmd))
        return 1;
    return 0;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    struct make_executable_options options = {0};
    options.populate_compile_commands = 1;
    DEBUG_ONLY(options.debug = 1);
    RELEASE_ONLY(options.release = 1);
    PGO_ONLY(options.profile_generate = 1);
    PROFILE_ONLY(options.profile = 1);

    if (make_executable(&options))
        return 1;

    if (options.profile_generate)
    {
        system("./main");
        CLANG_ONLY(system(
            "llvm-profdata merge -output=default.profdata default.profraw"));
        options.populate_compile_commands = 0;
        options.profile_generate = 0;
        options.profile_use = 1;
        if (make_executable(&options))
            return 1;
    }
}
