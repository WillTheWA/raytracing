#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define RAYS_NUMBER 1000
#define RAY_THICKNESS 3
#define RAY_COLOR 0xffd43b
#define OBJECT_SPEED 2

// Custom struct for a circle
struct Circle {
	double x;
	double y;
	double r;
};

struct Ray {
	// Starting pos
	double x_s;
	double y_s;
	
	// Angle
	double a;
};

struct CollisionPoint {
	double x;
	double y;
};

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

// This is the method used to generate the array of rays for RAYS_NUMBER
void generate_rays(struct Circle circle, struct Ray rays[RAYS_NUMBER]) {
	for (int i = 0; i < RAYS_NUMBER; i++) {
		double angle = ((double) i / RAYS_NUMBER) * 2 * M_PI;
		struct Ray ray = {circle.x, circle.y, angle};
		rays[i] = ray;
	}
}

// Given a ray that collided with an object, draw that ray as a reflection
void reflect_ray(SDL_Surface* surface, struct Ray ray, struct CollisionPoint collision, Uint32 color_r, struct Circle circle) {
	// Set boundary flags
	int end_of_screen = 0;
		
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
	while (!end_of_screen) {
		// Ray direction + step	
		x_draw += step * cos(reflect_a);
		y_draw += step * sin(reflect_a);

		SDL_Rect ray_point = (SDL_Rect) {x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};	
		SDL_FillRect(surface, &ray_point, color_r);
			
		// Check screen boundary
		if (x_draw < 0 || x_draw > WIDTH) {
			end_of_screen = 1;
		}
		if (y_draw < 0 || y_draw > HEIGHT) {
			end_of_screen = 1;
		}
	}
}

// This method draw the rays based on their projection and angle
void FillRays(SDL_Surface* surface, struct Ray rays[RAYS_NUMBER], Uint32 color, struct Circle object) {	
	double radius_squared = pow(object.r, 2);
	for (int i = 0; i < RAYS_NUMBER; i++) {
		struct Ray ray = rays[i];
		
		// Set boundary flags
		int end_of_screen = 0;
		int object_hit = 0;
		
		// Draw the ray
		double step = 1;
		double x_draw = ray.x_s;
		double y_draw = ray.y_s;
		while (!end_of_screen && !object_hit) {
			
			x_draw += step * cos(ray.a);
			y_draw += step * sin(ray.a);
			
			SDL_Rect ray_point = (SDL_Rect) {x_draw, y_draw, RAY_THICKNESS, RAY_THICKNESS};
			SDL_FillRect(surface, &ray_point, color);
			
			// Check screen boundary
			if (x_draw < 0 || x_draw > WIDTH) {
				end_of_screen = 1;
			}
			if (y_draw < 0 || y_draw > HEIGHT) {
				end_of_screen = 1;
			}
			
			// Check for object collision
			double distance_squared = pow(x_draw - object.x, 2) + pow(y_draw - object.y, 2);
			if (distance_squared < radius_squared) {
				// Pass collision point and reflect if ray
				struct CollisionPoint collision = {x_draw, y_draw};
				//printf("Collided: x=%f y=%f", collision.x, collision.y);
				reflect_ray(surface, ray, collision, RAY_COLOR, object);
				break;
			}
		}
	}
}

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

	// Generate the rays
	struct Ray rays[RAYS_NUMBER];
	generate_rays(circle1, rays);

	// Animation Loop
	double object_speed_y = OBJECT_SPEED;
	int simulation_running = 1;
	SDL_Event event;
	while(simulation_running) {
		// Check for window closed
		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				simulation_running = 0;
			}
			if (event.type == SDL_MOUSEMOTION && event.motion.state != 0) {
				circle1.x = event.motion.x;
				circle1.y = event.motion.y;
				generate_rays(circle1, rays);
			}
		}
		// Redraw a black screen to avoid mulitple circles when moving
		SDL_FillRect(surface, &erase_rect, COLOR_BLACK);
		
		// Draw the rays projected from circle1
		// Pass circle2 as obstacle
		FillRays(surface, rays, RAY_COLOR, circle2);
		
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
		SDL_Delay(10);
	}
}
