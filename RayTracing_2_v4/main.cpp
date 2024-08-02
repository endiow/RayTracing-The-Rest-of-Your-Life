#include "header/rtweekend.h"
#include "header/scene.h"

#include "header/camera.h"
#include "header/hittable_list.h"


int main()
{
	camera cam;
	hittable_list world;

	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));

	world.add(make_shared<quad>(point3(555, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), green));
	world.add(make_shared<quad>(point3(0, 0, 0), vec3(0, 555, 0), vec3(0, 0, 555), red));
	world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), light));
	world.add(make_shared<quad>(point3(0, 0, 0), vec3(555, 0, 0), vec3(0, 0, 555), white));
	world.add(make_shared<quad>(point3(555, 555, 555), vec3(-555, 0, 0), vec3(0, 0, -555), white));
	world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 555, 0), white));

	shared_ptr<hittable> box1 = box(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));
	world.add(box1);

	shared_ptr<hittable> box2 = box(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));
	world.add(box2);

	// Light Sources
	auto empty_material = shared_ptr<material>();
	quad lights(point3(343, 554, 332), vec3(-130, 0, 0), vec3(0, 0, -105), empty_material);

	cam.aspect_ratio = 1.0;
	cam.image_width = 600;
	cam.samples_per_pixel = 10;
	cam.max_depth = 50;
	cam.background = color(0, 0, 0);

	cam.vfov = 40;
	cam.lookfrom = point3(278, 278, -800);
	cam.lookat = point3(278, 278, 0);
	cam.vup = vec3(0, 1, 0);

	cam.defocus_angle = 0;

	/*switch (10)
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
		default: final_scene(400, 250, 4, cam, world); break;
	}*/

	//Ԥ���������
	int WindowWidth;//���ڿ��
	int WindowHeight;//���ڸ߶�
	if (cam.image_width > cam.image_width / cam.aspect_ratio)  //�����Ⱦ��ͼƬΪ��棬���㴰�ڿ����߶�
	{
		WindowHeight = 600;
		WindowWidth = (int)((double)WindowHeight * (double)cam.image_width / (double)(cam.image_width / cam.aspect_ratio));
	}
	else //�����Ⱦ��ͼƬΪ���棬���㴰�ڿ����߶�
	{
		WindowWidth = 900;
		WindowHeight = (int)((double)WindowWidth * (double)(cam.image_width / cam.aspect_ratio) / (double)cam.image_width);
	}

	Mat RenderingImage(static_cast<int>(cam.image_width / cam.aspect_ratio), cam.image_width, CV_8UC3, Scalar(50, 50, 50)); //����һ��ͼƬ
	namedWindow("ͼ��Ԥ������Ⱦ�У�", WINDOW_NORMAL);//���ñ���
	moveWindow("ͼ��Ԥ������Ⱦ�У�", (int)((1920.0 - WindowWidth) / 2), (int)((1080.0 - WindowHeight) / 2) - 50);//���ô���λ��
	resizeWindow("ͼ��Ԥ������Ⱦ�У�", WindowWidth, WindowHeight);//���ô��ڴ�С

	cam.render0(world, lights);
	//cam.render0(world);

	imshow("ͼ��Ԥ������Ⱦ�У�", RenderingImage);
	waitKey(5000); //�ȴ�3000����
	destroyAllWindows();//�رմ���
	return 0;
}
