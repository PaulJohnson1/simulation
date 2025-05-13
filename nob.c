#include <string.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "src/Utilities.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define COLLISION_MANAGER HASH_GRID
#define COLLISION_MANAGER_STR STRINGIFY(COLLISION_MANAGER)

#define BASE_CFLAGS                                                            \
    "-Wall", "-Wextra", "-Wpedantic", "-Isrc",                                 \
        "-DTMP_USE_" COLLISION_MANAGER_STR
#define BASE_LFLAGS "-lSDL3", "-lm", "-lGL"

#define RELEASE_CFLAGS "-O3 -ffast-math -DNDEBUG"
#define RELEASE_LFLAGS "-flto"

#define DEBUG_CFLAGS "-gdwarf-4 -DDEBUG"

#define PROFILE_CFLAGS "-gline-tables-only -fno-inline -O3 -DNDEBUG"

#define CC "clang"
#define CXX "clang++"

#define DEBUG_ONLY(...)
#define RELEASE_ONLY(...)
#define PROFILE_ONLY(...)

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
    if (!fp)
    {
        // fopen failed: errno is set
        return -1;
    }

    size_t len = strlen(contents);
    size_t written = fwrite(contents, 1, len, fp);
    if (written < len)
    {
        // a write error occurred
        fclose(fp);
        return -1;
    }

    if (fclose(fp) != 0)
    {
        // error closing file
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    char compile_commands_json[1000 * 1000] = {0};
    strcat(compile_commands_json, "[\n");

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    struct
    {
        tmp_vector_declare(char const *, srcs)
    } c = {0};
    struct
    {
        tmp_vector_declare(char const *, objs)
    } os = {0};

#define append_c_src(src)                                                      \
    tmp_vector_grow(char const *, c.srcs);                                     \
    *c.srcs_end++ = (src);
    append_c_src("Collision/BruteForce.c");
    append_c_src("Collision/Quadtree.c");
    append_c_src("Collision/SpatialHash.c");
    append_c_src("Ball.c");
    append_c_src("Main.c");
    append_c_src("Simulation.c");
    append_c_src("Utilities.c");
    append_c_src("Vector.c");
    append_c_src("Window.c");
#undef append_c_src

    for (char const **i = c.srcs; i < c.srcs_end; i++)
    {
        Nob_Cmd cmd = {0};

        char source[1024] = SRC_DIR;
        char output[1024] = BUILD_DIR;

        strcat(source, *i);
        strcat(output, *i);
        strcat(output, ".o");
        str_replace(output, '/', '.', sizeof BUILD_DIR);

        nob_cmd_append(&cmd, CC, "-c", "-o", output, source, BASE_CFLAGS,
                       RELEASE_ONLY(, RELEASE_CFLAGS) DEBUG_ONLY(, DEBUG_CFLAGS)
                           PROFILE_ONLY(, PROFILE_CFLAGS));
        if (!nob_cmd_run_sync(cmd))
            return 1;

        tmp_vector_grow(char const *, os.objs);
        *os.objs_end++ = strdup(output);

        // compile_commands.json
        char entry[1000 * 100] = {0};
        strcat(entry, "    {\n"
                      "        \"directory\": "
                      "\"/home/paul/Documents/programming/physics\",\n"
                      "        \"arguments\": [");
        for (uint64_t j = 0; j < cmd.count; j++)
        {
            strcat(entry, "\"");
            strcat(entry, cmd.items[j]);
            strcat(entry, "\"");
            if (j != cmd.count - 1)
                strcat(entry, ", ");
        }

        strcat(entry, "],\n");
        strcat(entry, "        \"file\": \"");
        strcat(entry, "/home/paul/Documents/programming/physics/src/");
        strcat(entry, *i);
        strcat(entry, "\",\n");

        strcat(entry, "        \"output\": \"");
        strcat(entry, "/home/paul/Documents/programming/physics/build/");
        strcat(entry, output);
        strcat(entry, "\"\n    }");
        if (i != c.srcs_end - 1)
            strcat(entry, ",\n");
        strcat(compile_commands_json, entry);
    }
    strcat(compile_commands_json, "\n]");

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CC);
    for (char const **i = os.objs; i < os.objs_end; i++)
        nob_cmd_append(&cmd, *i);
    nob_cmd_append(&cmd, "-o", "main",
                   BASE_LFLAGS RELEASE_ONLY(, RELEASE_LFLAGS));
    if (!nob_cmd_run_sync(cmd))
        return 1;

    write_to_file("compile_commands.json", compile_commands_json);
}
