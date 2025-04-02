#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define WIDTH 1000
#define HEIGHT 650
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define RAY_START 1000
#define RAY_MAX 10000
#define RAY_THICKNESS 3
#define RAY_COLOR 0xffd43b
#define OBJECT_SPEED 2
#define DARKEN_RATE 2
#define INITIAL_CAP 10

// Custom struct for a circle
struct Circle {
	double x;
	double y;
	double r;
};

struct Ray {
	double x_s;	// Starting x
	double y_s;	// Starting y
	double a;	// Angle
};

struct CollisionPoint {
	double x;
	double y;
};

struct RaySegment {
	SDL_Rect rect;
	Uint32 color;
};

// Dyn array vars
struct RaySegment *ray_segments = NULL;
size_t segment_count = 0;
size_t segment_capacity = 0;

/*
 * Function to draw circle on the SDL surface
 * 
 * This function draws a circle by first creating a box that is 
 * essentially 2r by 2r where r is the radius of the circle with the
 * center of the circle located at the center of the box. It then
 * checks within the box to see if certain pixels are withing the 
 * radius of the center point. If they are, then fill them in. If they
 * are not, then they are not filled in and will not be part of the
 * circle.
 *
 */
void FillCircle(SDL_Surface* surface, struct Circle circle, Uint32 color) {
	double radius_squared = pow(circle.r, 2);
	for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++) {
		for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++) {
			double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
			if (distance_squared < radius_squared) {
				SDL_Rect pixel = (SDL_Rect) {x, y, 1, 1};
				SDL_FillRect(surface, &pixel, color);
			}
		}
	}
}

// Create a dynamic array of the rays
struct Ray* create_dynarray(int size) {
	struct Ray* new_arr = malloc(size * sizeof(struct Ray));

	if (new_arr == NULL) {
		printf("Memory allocaition failure\n");
		return NULL;
	}

	return new_arr;
}

// Resize the array of rays (either increase or decrease
struct Ray* resize_dynarray(struct Ray* old_arr, int newsize) {
	struct Ray* new_arr = realloc(old_arr, newsize * sizeof(struct Ray));
	if (new_arr == NULL) {
		printf("Memory reallocation failed\n");
		return old_arr;
	}
	return new_arr;
}

// This is the method used to generate the array of rays for RAYS_NUMBER
// I want this to generate and return a dynarray to be used for the rays each time
void generate_rays(struct Circle circle, struct Ray* rays, int ray_count) {
	for (int i = 0; i < ray_count; i++) {
		double angle = ((double) i / ray_count) * 2 * M_PI;
		struct Ray ray = {circle.x, circle.y, angle};
		rays[i] = ray;
	}
}

// We want to change the color of the rays as they get further from the source
// The format for a hex color is 32-bit integer 0xRRGGBB
unsigned int darken_color(unsigned int color, int amount) {
	// Extract the color components by shifting bits
	unsigned int r = (color >> 16) & 0xFF;
	unsigned int g = (color >> 8) & 0xFF;
	unsigned int b = (color) & 0xFF;

	// Darken each RGB component
	// CLAMP TO PREVENT OVERFLOW
	r = (r > amount) ? (r - amount) : 0;
	g = (g > amount) ? (g - amount) : 0;
	b = (b > amount) ? (b - amount) : 0;
	
	// Combine values into color
	unsigned int new_color = (r << 16) | (g << 8) | b;	

	return new_color;
}

// Function for adding ray segment for drawing later
void add_segment(SDL_Rect rect, Uint32 color) {
	if (segment_count >= segment_capacity) {
		size_t new_capacity = (segment_capacity == 0) ? INITIAL_CAP : segment_capacity * 2;
		struct RaySegment *new_array = (struct RaySegment *)realloc(ray_segments, new_capacity * sizeof(struct RaySegment));

		if (!new_array) {
			printf("Error resizing array.\n");
			exit(1);
		}

		ray_segments = new_array;
		segment_capacity = new_capacity;
	}

	ray_segments[segment_count].rect = rect;
	ray_segments[segment_count].color = color;
	segment_count++;
}

// Function to clear the array of rays
void clear_rays() {
	free(ray_segments);
	ray_segments = NULL;
	segment_count = 0;
	segment_capacity = 0;
}

// Given a ray that collided with an object, draw that ray as a reflection
void reflect_ray(SDL_Surface* surface, struct Ray ray, struct CollisionPoint collision, Uint32 color_r, struct Circle circle) {
	// Set draw flags
	int end_of_screen = 0;
	int dead_ray = 0;
		
	// Use the object as a new ray caster for reflection
	double step = 1;
	double x_draw = collision.x;
	double y_draw = collision.y;
	
	// Compute incident ray direction
	double ray_x = cos(ray.a);
	double ray_y = sin(ray.a);

	// Compute normal vector at collision point
	double norm_x = collision.x - circle.x;
	double norm_y = collision.y - circle.y;
	double norm_mag = sqrt(pow(norm_x, 2) + pow(norm_y, 2));

	norm_x = norm_x / norm_mag;
	norm_y = norm_y / norm_mag;

	// Compute dot product
	double dot_prod = (ray_x * norm_x) + (ray_y * norm_y);

	// Reflection direction
	double reflect_x = ray_x - 2 * dot_prod * norm_x; 
	double reflect_y = ray_y - 2 * dot_prod * norm_y;
       	double reflect_a = atan2(reflect_y, reflect_x);

	// Only test edge of screen
	// Include darkening effect
	Uint32 new_color = color_r;
	int darken_counter = 0;
	while (!end_of_screen && !dead_ray) {
		// Darkening code
		if (darken_counter < DARKEN_RATE)
			darken_counter += 1;
		else {
			new_color = darken_color(new_color, 1);
			darken_counter = 0;
		}

		// Ray direction + step	
		x_draw += step * cos(reflect_a);
		y_draw += step * sin(reflect_a);

		SDL_Rect ray_point = (SDL_Rect) {x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};	
		SDL_FillRect(surface, &ray_point, new_color);
			
		// Check screen boundary
		if (x_draw < 0 || x_draw > WIDTH) {
			end_of_screen = 1;
		}
		if (y_draw < 0 || y_draw > HEIGHT) {
			end_of_screen = 1;
		}

		// Check if a ray doesn't need to be rendered
		if (new_color == 0x000000) {
			dead_ray = 1;
		}
	}
}

// Create and draw the rays
void FillRays(SDL_Surface* surface, struct Ray* rays, Uint32 color, struct Circle object, int ray_count) {	
	// If there are no rays, then don't fill or generate any
	if (ray_count == 0)
		return;

	double radius_squared = pow(object.r, 2);
	int status;
	
	// Fork child to create ray
	for (int i = 0; i < ray_count; i++) {
		pid_t pid = fork();

		if (pid < 0) {
			printf("Fork Failed\n");
			exit(1);  // Exit on fork failure
		}

		if (pid == 0) { // Child process
			struct Ray ray = rays[i];
		
			// Set draw flags
			int end_of_screen = 0;
			int object_hit = 0;
			int dead_ray = 0;

			// Draw the ray
			double step = 1;
			double x_draw = ray.x_s;
			double y_draw = ray.y_s;
			Uint32 new_color = color;
			int darken_counter = 0;

			while (!end_of_screen && !object_hit && !dead_ray) {	
				// Darkening color code
				if (darken_counter < DARKEN_RATE)
					darken_counter += 1;
				else {
					new_color = darken_color(new_color, 1);
					darken_counter = 0;
				}

				x_draw += step * cos(ray.a);
				y_draw += step * sin(ray.a);
		
				SDL_Rect ray_point = (SDL_Rect) {x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};
				add_segment(ray_point, new_color);

				// Check screen boundary
				if (x_draw < 0 || x_draw > WIDTH || y_draw < 0 || y_draw > HEIGHT) {
					end_of_screen = 1;
				}

				// Check if a ray doesn't need to be rendered
				if (new_color == 0x000000) {
					dead_ray = 1;
				}

				// Check for object collision
				double distance_squared = pow(x_draw - object.x, 2) + pow(y_draw - object.y, 2);
				if (distance_squared < radius_squared) {
					// Pass collision point and reflect if ray
					struct CollisionPoint collision = {x_draw, y_draw};
					reflect_ray(surface, ray, collision, new_color, object);
					break;
				}
			}
			_exit(0);  // Proper exit for child
		}
	}

	// Parent waits for all child processes
	for (int i = 0; i < ray_count; i++) {
		wait(&status);
	}

	// Parent draws segments after all children complete
	for (size_t i = 0; i < segment_count; i++) {
		SDL_FillRect(surface, &ray_segments[i].rect, ray_segments[i].color);
	}
}

// Program entrypoint :)
int main() {
	// Initialize the SDL and create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

	// Create a surface for the window to render the objects
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	SDL_Rect erase_rect = (SDL_Rect) {0, 0, WIDTH, HEIGHT};

	// Create two circles using the struct
	struct Circle circle1 = {200, 200, 30};
	struct Circle circle2 = {650, 200, 100};

	// Set var for variable number of rays start at RAYS_NUMBER;
	int ray_count = RAY_START;
	struct Ray* rays = create_dynarray(ray_count);
	generate_rays(circle1, rays, ray_count);

	// Animation Loop
	double object_speed_y = OBJECT_SPEED;
	int simulation_running = 1;
	SDL_Event event;

	Uint32 lastTime = SDL_GetTicks();
	Uint32 frameCount = 0;

	while(simulation_running) {	
		Uint32 startTime = SDL_GetTicks();
		
		// Check for window closed
		while(SDL_PollEvent(&event)) {
			// Check if quit
			if (event.type == SDL_QUIT) {
				simulation_running = 0;
				clear_rays();
				free(rays);
			}
			// Check if mouse moved
			if (event.type == SDL_MOUSEMOTION && event.motion.state != 0) {
				circle1.x = event.motion.x;
				circle1.y = event.motion.y;
				generate_rays(circle1, rays, ray_count);
			}
			// Check if ray number altered and keep within bounds
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_x ) {
				if (ray_count + 10 <= RAY_MAX) {
					ray_count += 10;
					rays = resize_dynarray(rays, ray_count);
					generate_rays(circle1, rays, ray_count);
				}
			}
			// REALLOC SIZE CANNOT BE 0
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_z ) {
				if (ray_count - 10 >= 10) {
					ray_count -= 10;
					rays = resize_dynarray(rays, ray_count);
					generate_rays(circle1, rays, ray_count);
				} else if (ray_count - 10 >= 0) {
					ray_count -= 10;
					generate_rays(circle1, rays, ray_count);
				}
			}
		}
		// Redraw a black screen to avoid mulitple circles when moving
		SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
		
		// Draw the rays projected from circle1
		// Pass circle2 as obstacle
		FillRays(surface, rays, RAY_COLOR, circle2, ray_count);
		
		// Redraw both circles above the rays
		FillCircle(surface, circle1, COLOR_WHITE);
		FillCircle(surface, circle2, COLOR_WHITE);
		
		circle2.y += object_speed_y;
		if (circle2.y - circle2.r < 0) {
			object_speed_y = -object_speed_y;
		}
		if (circle2.y + circle2.r > HEIGHT) {
			object_speed_y = -object_speed_y;
		}

		SDL_UpdateWindowSurface(window);

		// Update framecount
		frameCount++;
		Uint32 currentTime = SDL_GetTicks();
    		if (currentTime - lastTime >= 1000) {
        		printf("Ray Count: %d     FPS: %d\n", ray_count, frameCount);
        		frameCount = 0;
        		lastTime = currentTime;
    		}

    		// Cap framerate to 60 fps
    		Uint32 frameTime = SDL_GetTicks() - startTime;
    		
		// 16 ms per frame (1000ms / 60fps)
		if (frameTime < 16) {
        		SDL_Delay(16 - frameTime);
    		}

		// Clear the rays
		clear_rays();
	}
}
