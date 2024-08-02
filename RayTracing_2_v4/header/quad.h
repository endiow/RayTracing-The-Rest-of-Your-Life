#pragma once
#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"

class quad : public hittable 
{
public:
    quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
        : Q(Q), u(u), v(v), mat(mat)
    {
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);

        area = n.length();

        set_bounding_box();
    }

    virtual void set_bounding_box() 
    {
        // Compute the bounding box of all four vertices.
        auto bbox_diagonal1 = aabb(Q, Q + u + v);
        auto bbox_diagonal2 = aabb(Q + u, Q + v);
        bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        auto denom = dot(normal, r.direction());

        // 光线与平面平行
        if (std::fabs(denom) < 1e-8)
            return false;

        // 交点在射线范围之外(平面也有包围盒)
        auto t = (D - dot(normal, r.origin())) / denom;
        if (!ray_t.contains(t))
            return false;

        auto intersection = r.at(t);    //交点
        vec3 planar_hitpt_vector = intersection - Q;
        auto alpha = dot(w, cross(planar_hitpt_vector, v));     //算出点相对于平面Q的偏移
        auto beta = dot(w, cross(u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, rec))     //alpha,beta都在[0,1]即在quad内
            return false;

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    //判断光线与平面的交点是否在平面内
    virtual bool is_interior(double a, double b, hit_record& rec) const 
    {
        interval unit_interval = interval(0, 1);    
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;      //设置纹理坐标值
        rec.v = b;
        return true;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override 
    {
        hit_record rec;
        if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        //dA = rdθ∗rsinθdφ= r^2 sinθdθdφ
        //立体角 = dA / r^2 = sinθdθdφ
        
        //对于光源A，在半球上的投影立体角 dω = (A * cosa)/ d^2
        //其中，a为光源法向量与光线的夹角
        //则， 概率 = p(ω) * dω = p(q) * dA      p(q)为球心p到光源上的点q的概率密度函数   dω为立体角
        //对光源均匀采样，则p(q) = 1/A
        //有，(p(ω) * dA * cos(θ)) / distance^2(p,q) = dA / A  =>  p(ω) = distance^2 * (p,q) / cos(θ) * A 
        auto distance_squared = rec.t * rec.t * direction.length_squared();
        auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);      //返回光源的概率密度函数
    }

    vec3 random(const point3& origin) const override 
    {
        auto p = Q + (random_double() * u) + (random_double() * v);
        return p - origin;
    }

private:
    point3 Q;   //基点
    vec3 u, v;  //两个边向量
    vec3 w;     
    shared_ptr<material> mat;
    aabb bbox;
    vec3 normal;
    double D;
    double area;    //光源的A
};


inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<hittable_list>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3(std::fmin(a.x(), b.x()), std::fmin(a.y(), b.y()), std::fmin(a.z(), b.z()));
    auto max = point3(std::fmax(a.x(), b.x()), std::fmax(a.y(), b.y()), std::fmax(a.z(), b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()), dx, dy, mat)); // front
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz, dy, mat)); // right
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx, dy, mat)); // back
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dz, dy, mat)); // left
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()), dx, -dz, mat)); // top
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dx, dz, mat)); // bottom

    return sides;
}