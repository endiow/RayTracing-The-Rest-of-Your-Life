#pragma once

#include "rtweekend.h"
#include "aabb.h"

class material;

class hit_record 
{
public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    bool front_face;
    double u;
    double v;

    void set_face_normal(const ray& r, const vec3& outward_normal) 
    {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable 
{
public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

    virtual aabb bounding_box() const = 0;

    virtual double pdf_value(const point3& origin, const vec3& direction) const 
    {
        return 0.0;
    }

    virtual vec3 random(const point3& origin) const 
    {
        return vec3(1, 0, 0);
    }
};


class translate : public hittable 
{
public:
    translate(shared_ptr<hittable> object, const vec3& offset)
        : object(object), offset(offset)
    {
        bbox = object->bounding_box() + offset;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        // 将射线的原点反向移动偏移量
        ray offset_r(r.origin() - offset, r.direction(), r.time());

        // 判断添加偏移后的射线是否与物体存在交点（如果存在，判断在何处）
        if (!object->hit(offset_r, ray_t, rec))
            return false;

        // 给交点的位置增加偏移量
        rec.p += offset;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> object;
    vec3 offset;
    aabb bbox;
};



class rotate_y : public hittable 
{
public:
    rotate_y(shared_ptr<hittable> object, double angle) : object(object) 
    {
        auto radians = degrees_to_radians(angle);
        sin_theta = std::sin(radians);
        cos_theta = std::cos(radians);
        bbox = object->bounding_box();

        point3 min(infinity, infinity, infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++) 
        {
            for (int j = 0; j < 2; j++) 
            {
                for (int k = 0; k < 2; k++) 
                {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto newx = cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; c++) 
                    {
                        min[c] = std::fmin(min[c], tester[c]);
                        max[c] = std::fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bbox = aabb(min, max);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        // 将光线从世界坐标系转换到物体局部坐标系
        auto origin = r.origin();
        auto direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        // 判断是否有交点以及交点在何处
        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        // 将交点从物体局部坐标系转换到世界坐标系
        auto p = rec.p;
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        // 将交点处的法线从物体局部坐标系转换到世界坐标系
        auto normal = rec.normal;
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    aabb bbox;
};