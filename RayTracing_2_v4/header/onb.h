#pragma once
#include "rtweekend.h"

class onb 
{
public:
    onb(const vec3& n)  //生成局部坐标系
    {
        axis[2] = unit_vector(n);
        vec3 a = (std::fabs(axis[2].x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        axis[1] = unit_vector(cross(axis[2], a));
        axis[0] = cross(axis[2], axis[1]);
    }

    const vec3& u() const { return axis[0]; }
    const vec3& v() const { return axis[1]; }
    const vec3& w() const { return axis[2]; }

    vec3 transform(const vec3& v) const 
    {
        // 局部坐标系转换为世界坐标系
        return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
    }

private:
    vec3 axis[3];
};