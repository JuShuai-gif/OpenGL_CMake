#pragma once
#include<glm/glm.hpp>
#include"global.h"
#include "Bounds3.h"
#include "Ray.h"
#include "Intersection.h"

class Object
{
public:
    Object() {}
    virtual ~Object() {}
    virtual bool intersect(const Ray& ray) = 0;
    virtual bool intersect(const Ray& ray, float &, uint32_t &) const = 0;
    virtual Intersection getIntersection(Ray _ray) = 0;
    virtual void getSurfaceProperties(const glm::vec3 &, const glm::vec3 &, const uint32_t &, const glm::vec2 &, glm::vec3 &, glm::vec2 &) const = 0;
    virtual glm::vec3 evalDiffuseColor(const glm::vec2 &) const =0;
    virtual Bounds3 getBounds()=0;
    virtual float getArea()=0;
    virtual void Sample(Intersection &pos, float &pdf)=0;
    virtual bool hasEmit()=0;
};