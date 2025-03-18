TARGET = raytracing

all:
	gcc raytracing.c -o $(TARGET) `sdl2-config --cflags --libs` -lm

clean:
	rm -f $(TARGET)
