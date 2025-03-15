# Exploration Overview
This is an exploration into raytracing using C.

## Compiling
First ensure that both `GCC` and `SDL` are installed.

Navigate to the directory where `raytracing.c` is stored.

Run the following command to compile the file:

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

## Credit
This exploration was inspired and built on a coding project done by `HirschDaniel`. [This](https://youtu.be/2BLRLuczykM?si=3woeIgQ-pk0VzeCp) is the link to the specific video that I followed.
