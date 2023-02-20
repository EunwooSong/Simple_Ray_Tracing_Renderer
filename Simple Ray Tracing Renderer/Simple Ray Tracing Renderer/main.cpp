#include "common.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

real hit_sphere(const point3& center, real radius, const ray& r) {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant < 0)
		return -1.0;
	else 
		return (-half_b - sqrt(discriminant)) / a;
}

color ray_color(const ray& r, const hittable* world, int depth) {
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (world->hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}

	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

void render_block(hittable_list* world, camera* cam,
	int max_depth, int samples_per_pixel,
	int image_height, int image_width,
	int start_width, int end_width, std::vector<std::vector<color>>* result) {

	for (int j = image_height - 1; j >= 0; j--) {
		std::cerr << "Renderer : " << j << ", w : " << start_width
			<< " to " << end_width << std::endl;

		result->push_back(std::vector<color>());
		for (int i = start_width; i < end_width; i++) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; s++) {
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam->get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			(*result)[image_height-(j+1)].push_back(pixel_color);
		} 
	}
}

int main() {
	// Image 
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = WIDTH;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	// Multi Thread Renderer!
	std::vector<color> data(image_width * image_height);
	std::vector<std::vector<std::vector<color>>> partial_data(RENDER_THREAD);
	std::vector<thread> renderers;

	// World
	hittable_list world;
	
	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8));
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2));

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

	// Camera
	camera cam;

	// Init & Start Renderer
	for (int i = 0; i < RENDER_THREAD; i++) {
		renderers.push_back(thread(
			render_block, &world, &cam, max_depth, samples_per_pixel,
			image_height, image_width,
			i * image_width/RENDER_THREAD, (i+1) * image_width/RENDER_THREAD, 
			&partial_data[i]));
	}
	
	for (int i = 0; i < RENDER_THREAD; i++) {
		renderers[i].join();
	}

	// Write
	std::cerr << "\n Print Render Result \n" << std::endl;
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	
	for (int j = 0; j < image_height; j++) {
		for (int i = 0; i < RENDER_THREAD; i++) {
			for (int k = 0; k < image_width / RENDER_THREAD; k++) {

				write_color(std::cout, partial_data[i][j][k], samples_per_pixel);
			}
		}
		
	}
	std::cerr << "\nDone.\n" << std::endl;
	return 0;
}