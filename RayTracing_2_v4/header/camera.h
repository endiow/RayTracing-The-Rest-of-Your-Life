﻿#pragma once

#include "rtweekend.h"
#include "pdf.h"
#include "material.h"

class camera 
{
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth = 10;   // Maximum number of ray bounces into scene
    color  background;               // Scene background color

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, 0);   // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3   vup = vec3(0, 1, 0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render0(const hittable& world, const hittable& lights)
    {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) 
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) 
            {
                color pixel_color(0, 0, 0);
                for (int s_j = 0; s_j < sqrt_spp; s_j++) 
                {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) 
                    {
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world, lights);
                    }
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

    void render1(Mat& rendering, const hittable& world, const hittable& lights)
    {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = image_height - 1; j > 0; j--)
        {
            std::clog << "\rScanlines remaining: " << j - 1 << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) 
            {
                color pixel_color(0, 0, 0);
                for (int s_j = 0; s_j < sqrt_spp; s_j++)
                {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++)
                    {
                        ray r = get_ray(i, (image_height - j - 1), s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world, lights);
                    }
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);

                auto r = pixel_samples_scale * pixel_color.x();
                auto g = pixel_samples_scale * pixel_color.y();
                auto b = pixel_samples_scale * pixel_color.z();

                // Replace NaN components with zero.
                /*if (r != r) r = 0.0;
                if (g != g) g = 0.0;
                if (b != b) b = 0.0;*/

                // Apply a linear to gamma transform for gamma 2
                r = linear_to_gamma(r);
                g = linear_to_gamma(g);
                b = linear_to_gamma(b);

                // Translate the [0,1] component values to the byte range [0,255].
                static const interval intensity(0.000, 0.999);
                int rbyte = int(256 * intensity.clamp(r));
                int gbyte = int(256 * intensity.clamp(g));
                int bbyte = int(256 * intensity.clamp(b));

                //实时显示渲染帧
                rendering.at<cv::Vec3b>(image_height - 1 - j, i)[0] = bbyte;
                rendering.at<cv::Vec3b>(image_height - 1 - j, i)[1] = gbyte;
                rendering.at<cv::Vec3b>(image_height - 1 - j, i)[2] = rbyte;
            }

            //每行计算完以后刷新预览窗口
            if (!(j % (image_height / 100))) //每渲染image_height/100行后更新预览窗口图片
            {
                imshow("图像预览（渲染中）", rendering);
                waitKey(1);//等待1毫秒事件让窗口刷新完毕
            }
        }
        std::clog << "\rDone.                 \n";
    }

private:
    int    image_height;   // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    int    sqrt_spp;             // Square root of number of samples per pixel
    double recip_sqrt_spp;       // 1 / sqrt_spp
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius

    void initialize() 
    {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        sqrt_spp = int(std::sqrt(samples_per_pixel));
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
        recip_sqrt_spp = 1.0 / sqrt_spp;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // 光圈的u,v向量
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j, int s_i, int s_j) const
    {
        // 构建一条源自光圈并随机指向的相机光线
        // 对于分层样本正方形si，sj，像素位置i，j周围的采样点

        auto offset = sample_square_stratified(s_i, s_j);

        auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square_stratified(int s_i, int s_j) const 
    {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return vec3(px, py, 0);
    }

    vec3 sample_square() const 
    {
        // 在像素周围[-.5，-.5]-[+.5，+.5]单位平方内随机采样
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const 
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    //color=∫attenuation∗s(direction)color(direction)=attenuation∗s(direction)∗color(direction)/p(direction)
    //颜色 == ∫衰减 * 本色 * 半球内每条光线的权重（s(direction)）/ 光线的分布情况（p(direction)/pdf）
    color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const
    {
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;

        // If the ray hits nothing, return the background color.
        if (!world.hit(r, interval(0.001, infinity), rec))
            return background;

        scatter_record srec;
        color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, srec))
            return color_from_emission;

        if (srec.skip_pdf) 
        {
            return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);
        }

        auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf p(light_ptr, srec.pdf_ptr);

        ray scattered = ray(rec.p, p.generate(), r.time());
        auto pdf_value = p.value(scattered.direction());

        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

        color sample_color = ray_color(scattered, depth - 1, world, lights);
        color color_from_scatter = (srec.attenuation * scattering_pdf * sample_color) / pdf_value;

        return color_from_emission + color_from_scatter;
    }
};