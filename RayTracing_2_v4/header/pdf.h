#pragma once
#include "rtweekend.h"

#include "onb.h"
#include "hittable_list.h"


class pdf 
{
public:
    virtual ~pdf() {}

    virtual double value(const vec3& direction) const = 0;      //返回概率密度函数
    virtual vec3 generate() const = 0;
};


class sphere_pdf : public pdf 
{
public:
    sphere_pdf() {}

    double value(const vec3& direction) const override 
    {
        return 1 / (4 * pi);        //球随机分布
    }

    vec3 generate() const override 
    {
        return random_unit_vector();
    }
};


class cosine_pdf : public pdf 
{
public:
    cosine_pdf(const vec3& w) : uvw(w) {}

    double value(const vec3& direction) const override 
    {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta / pi);     //越靠近法线概率越大
    }

    vec3 generate() const override 
    {
        return uvw.transform(random_cosine_direction());
    }

private:
    onb uvw;
};


class mixture_pdf : public pdf 
{
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) 
    {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const vec3& direction) const override 
    {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override 
    {
        if (random_double() < 0.5)        //来自散射和光源的权重
            return p[0]->generate();
        else
            return p[1]->generate();
    }

private:
    shared_ptr<pdf> p[2];
};


class hittable_pdf : public pdf
{
public:
    hittable_pdf(const hittable& objects, const point3& origin)
        : objects(objects), origin(origin)
    {}

    double value(const vec3& direction) const override
    {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override
    {
        return objects.random(origin);
    }

private:
    const hittable& objects;
    point3 origin;
};