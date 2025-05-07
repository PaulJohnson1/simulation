clang-15 \
    Main.c \
    -I. \
    -lSDL3 \
    -lm \
    -lGL \
    -O3 \
    -flto \
    -ffast-math \
    -DNDEBUG \
    -mtune=native \
    -march=native
    