# Exploration Overview
This is an exploration into raytracing using C. Raytracing has recently become a topic that I am very interested in, as its one of those topic that I never thought I would be able to understand. While the base implementation provided in this repo is limited to a 2d plane, I still want to figure out the optimal amount of rays and experiment with refraction. Feel free to test out the implementation provided and please note that a lot of this comes from `HirschDaniel` and IS NOT original content.

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
