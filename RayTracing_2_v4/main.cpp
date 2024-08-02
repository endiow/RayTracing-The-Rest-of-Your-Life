#include "header/rtweekend.h"
#include "header/scene.h"

#include "header/camera.h"
#include "header/hittable_list.h"


int main()
{
	camera cam;
	hittable_list world;
	// Light Sources
	auto empty_material = shared_ptr<material>();
	hittable_list lights;
	lights.add(make_shared<quad>(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), empty_material));
	lights.add(make_shared<sphere>(point3(190, 90, 190), 90, empty_material));
	

	switch (11)
	{
		case 1: bouncing_spheres(cam, world);  break;
		case 2: checkered_spheres(cam, world); break;
		case 3:  earth(cam, world);             break;
		case 4:  perlin_spheres(cam, world);     break;
		case 5:  quads(cam, world);              break;
		case 6:  simple_light(cam, world);       break;
		case 7:  cornell_box(cam, world);        break;
		case 8:  cornell_smoke(cam, world);      break;
		case 9:  final_scene(800, 10000, 40, cam, world); break;
		case 10:  mc_cornell_box(cam, world, lights);        break;
		case 11:  mc_cornell_box_sphere(cam, world, lights);        break;
		default: final_scene(400, 250, 4, cam, world); break;
	}

	//预览窗口相关
	int WindowWidth;//窗口宽度
	int WindowHeight;//窗口高度
	if (cam.image_width > cam.image_width / cam.aspect_ratio)  //如果渲染的图片为横版，计算窗口宽度与高度
	{
		WindowHeight = 600;
		WindowWidth = (int)((double)WindowHeight * (double)cam.image_width / (double)(cam.image_width / cam.aspect_ratio));
	}
	else //如果渲染的图片为竖版，计算窗口宽度与高度
	{
		WindowWidth = 900;
		WindowHeight = (int)((double)WindowWidth * (double)(cam.image_width / cam.aspect_ratio) / (double)cam.image_width);
	}

	Mat RenderingImage(static_cast<int>(cam.image_width / cam.aspect_ratio), cam.image_width, CV_8UC3, Scalar(50, 50, 50)); //创建一张图片
	namedWindow("图像预览（渲染中）", WINDOW_NORMAL);//设置标题
	moveWindow("图像预览（渲染中）", (int)((1920.0 - WindowWidth) / 2), (int)((1080.0 - WindowHeight) / 2) - 50);//设置窗口位置
	resizeWindow("图像预览（渲染中）", WindowWidth, WindowHeight);//设置窗口大小

	cam.render1(RenderingImage, world, lights);
	//cam.render0(world);

	imshow("图像预览（渲染中）", RenderingImage);
	waitKey(5000); //等待3000毫秒
	destroyAllWindows();//关闭窗口
	return 0;
}
