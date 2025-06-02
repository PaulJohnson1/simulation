#include <string.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "src/Utilities.h"

// #define DEBUG
#define RELEASE
#define BENCHMARK

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define CONCAT(x, y) x##y

// #define COLLISION_MANAGER BRUTE_FORCE
#define COLLISION_MANAGER HASH_GRID
// #define COLLISION_MANAGER NAIVE_HASH_GRID
// #define COLLISION_MANAGER QUADTREE

// clang-format off
#define BASE_FLAGS(X)                                                          \
  X("-Werror")                                                                   \
  X("-Wall")                                                                     \
  X("-Wextra")                                                                   \
  X("-Wpedantic")                                                                \
  X("-Wconversion")                                                              \
  X("-Wno-empty-translation-unit")                                               \
  X("-Wno-unused-function")                                                      \
  X("-Wno-extra-semi")                                                           \
  X("-Wno-gnu-empty-struct")                                                     \
  X("-Wno-strict-prototypes")                                                    \
  X("-Isrc")                                                                     \
  X("-DTMP_USE_" STRINGIFY(COLLISION_MANAGER))

#define BASE_CFLAGS(X)
#define BASE_CCFLAGS(X) X("-std=c++20")
// clang-format on
#define BASE_LFLAGS "-lSDL3", "-lm", "-lGL"

#define RELEASE_CFLAGS(X)                                                      \
  X("-O3")                                                                     \
  X("-ffast-math")                                                             \
  X("-DNDEBUG")

#define RELEASE_LFLAGS "-flto"

#define DEBUG_CFLAGS(X) X("-gdwarf-4") X("-DDEBUG")

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

char *str_replace(char *str, char find, char replace, uint64_t start_from) {
  for (; start_from[str]; str++)
    if (start_from[str] == find)
      start_from[str] = replace;
  return str;
}

int write_to_file(const char *file_name, const char *contents) {
  FILE *fp = fopen(file_name, "w");
  size_t len = strlen(contents);
  fwrite(contents, 1, len, fp);
  fclose(fp);

  return 0;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  char compile_commands_json[1000 * 1000] = {0};
  strcat(compile_commands_json, "[\n");

  if (!nob_mkdir_if_not_exists(BUILD_DIR))
    return 1;

  struct compilation_target {
    char const *compiler;
    char const *file;
    tmp_vector_declare(char const *, flags);
  };

  tmp_vector_declare_zero(struct compilation_target, targets);
  tmp_vector_declare_zero(char const *, objects);

#define append_flag(flag)                                                      \
  tmp_vector_grow(char const *, targets_end->flags);                           \
  *targets_end->flags_end++ = flag;

#define append_cc_src(src)                                                     \
  tmp_vector_grow(struct compilation_target, targets);                         \
  *targets_end = (struct compilation_target){.compiler = CXX, .file = (src)};  \
  BASE_FLAGS(append_flag);                                                     \
  BASE_CCFLAGS(append_flag);                                                   \
  targets_end++;
#define append_c_src(src)                                                      \
  tmp_vector_grow(struct compilation_target, targets);                         \
  *targets_end = (struct compilation_target){.compiler = CC, .file = (src)};   \
  BASE_FLAGS(append_flag);                                                     \
  BASE_CFLAGS(append_flag);                                                    \
  targets_end++;

  append_c_src("Collision/BruteForce.c");
  append_c_src("Collision/Quadtree.c");
  append_c_src("Collision/HashGrid.c");
  append_c_src("Collision/NaiveHashGrid.c");
  append_c_src("Ball.c");
  append_c_src("Simulation.c");
  append_c_src("Utilities.c");
  append_c_src("Vector.c");
  append_c_src("Window.c");

#ifdef BENCHMARK
  append_cc_src("Benchmark.cc");
#else
  append_c_src("Main.c");
#endif

#undef append_flag
#undef append_c_src
#undef append_cc_srcs

  for (struct compilation_target *t = targets; t < targets_end; t++) {
    Nob_Cmd cmd = {0};

    char source[1024] = SRC_DIR;
    char output[1024] = BUILD_DIR;

    strcat(source, t->file);
    strcat(output, t->file);
    strcat(output, ".o");
    str_replace(output, '/', '.', sizeof BUILD_DIR);

    nob_cmd_append(&cmd, t->compiler, "-c", "-o", output, source);
    for (char const **flag = t->flags; flag < t->flags_end; flag++)
      nob_cmd_append(&cmd, *flag);

#define append_flag(flag) nob_cmd_append(&cmd, flag);
    RELEASE_ONLY(RELEASE_CFLAGS(append_flag));
    DEBUG_ONLY(DEBUG_CFLAGS(append_flag));
#undef append_flag

    if (!nob_cmd_run_sync(cmd))
      return 1;

    tmp_vector_grow(char const *, objects);
    *objects_end++ = strdup(output);

    // compile_commands.json
    char entry[1000 * 100] = {0};
    strcat(entry, "    {\n"
                  "        \"directory\": "
                  "\"/home/paul/Documents/programming/physics\",\n"
                  "        \"arguments\": [");
    for (uint64_t j = 0; j < cmd.count; j++) {
      strcat(entry, "\"");
      strcat(entry, cmd.items[j]);
      strcat(entry, "\"");
      if (j != cmd.count - 1)
        strcat(entry, ", ");
    }

    strcat(entry, "],\n");
    strcat(entry, "        \"file\": \"");
    strcat(entry, "/home/paul/Documents/programming/physics/src/");
    strcat(entry, t->file);
    strcat(entry, "\",\n");

    strcat(entry, "        \"output\": \"");
    strcat(entry, "/home/paul/Documents/programming/physics/build/");
    strcat(entry, output);
    strcat(entry, "\"\n    }");
    if (t != targets_end - 1)
      strcat(entry, ",\n");
    strcat(compile_commands_json, entry);
  }
  strcat(compile_commands_json, "\n]");

  // link
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CXX);
  for (char const **i = objects; i < objects_end; i++)
    nob_cmd_append(&cmd, *i);
  nob_cmd_append(&cmd, "-o", "main",
                 BASE_LFLAGS RELEASE_ONLY(, RELEASE_LFLAGS));
  if (!nob_cmd_run_sync(cmd))
    return 1;

  write_to_file("compile_commands.json", compile_commands_json);
}
