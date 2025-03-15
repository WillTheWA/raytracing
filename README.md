# Exploration Overview
This is an exploration into raytracing using C.

## Compiling
First ensure that both GCC and SDL are installed.

Navigate to the directory where raytracing.c is stored.

Run the following command to compile:

```
gcc raytracing.c -o raytracing `sdl2-config --cflags --libs` -lm
```

This should generate a raytracing executeable file.

The program can be launched using the following command:

```
./raytracing
```

## TODO:
- Optimize Rays
- Add 'Light' Refraction
- Improve Visualization
