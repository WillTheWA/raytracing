# Exploration Overview
This is an exploration into raytracing using C. Raytracing has recently become a topic that I am very interested in, as its one of those topic that I never thought I would be able to understand. While the base implementation provided in this repo is limited to a 2d plane, I still want to figure out the optimal amount of rays and experiment with refraction. Feel free to test out the implementation provided and please note that a lot of this comes from `HirschDaniel`.

This includes some basic features such as 1000 rays from a small circle being emitted into a space with a larger circle which serves as the collision object. These rays also mimic light where it dissipates over time. The light rays also reflect off of the larger circle without overriding the previous light rays (which is pretty cool).

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

I'll add a makefile eventually lol.

## TODO:
- Add Ray Number Slider
- Add Other Shapes

## Credit
This exploration was inspired and built on a coding project done by `HirschDaniel`. [This](https://youtu.be/2BLRLuczykM?si=3woeIgQ-pk0VzeCp) is the link to the specific video that I followed.
