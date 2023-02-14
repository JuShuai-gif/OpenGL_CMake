#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Object.h"
#include "Light.h"
#include "AreaLight.h"
#include "BVH.h"
#include "Ray.h"

class Scene
{
public:
    int width = 1280;       // 宽
    int height = 960;       // 高
    double fov = 40;        // 视口比例
    // 背景颜色
    glm::vec3 backgroundColor = glm::vec3(0.235294, 0.67451, 0.843137);
    // 最大深度
    int maxDepth = 1;
    // RR系数
    float RussianRoulette = 0.8;

public:
    Scene(int w, int h) : width(w), height(h)
    {}
    // 增加物体
    void Add(Object *object) { 
        objects.push_back(object); 
    }
    // 增加灯光
    void Add(std::unique_ptr<Light> light) { 
        lights.push_back(std::move(light)); 
    }
    // 获取物体
    const std::vector<Object*>& get_objects() const {
        return objects; 
    }
    // 获取灯光
    const std::vector<std::unique_ptr<Light> >&  get_lights() const { 
        return lights; 
    }
    
public:
    BVHAccel *bvh;
    Intersection intersect(const Ray& ray) const;
    void buildBVH();
    glm::vec3 castRay(const Ray &ray, int depth) const;
    void sampleLight(Intersection &pos, float &pdf) const;
    bool trace(const Ray &ray, const std::vector<Object*> &objects, float &tNear, uint32_t &index, Object **hitObject);
    std::tuple<glm::vec3, glm::vec3> HandleAreaLight(const AreaLight &light, const glm::vec3 &hitPoint, const glm::vec3 &N,
                                                   const glm::vec3 &shadowPointOrig,
                                                   const std::vector<Object *> &objects, uint32_t &index,
                                                   const glm::vec3 &dir, float specularExponent);
public:
    // 物体
    std::vector<Object* > objects;
    // 灯光
    std::vector<std::unique_ptr<Light> > lights;
    // 计算反射
    glm::vec3 reflect(const glm::vec3 &I, const glm::vec3 &N) const
    {
        return I - 2 * glm::dot(I, N) * N;
    }
    // 计算折射
    glm::vec3 refract(const glm::vec3 &I, const glm::vec3 &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, glm::dot(I, N));
        float etai = 1, etat = ior;
        glm::vec3 n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? glm::vec3(0) : eta * I + (eta * cosi - sqrtf(k)) * n;
    }
    // 计算菲涅尔反射
    void fresnel(const glm::vec3 &I, const glm::vec3 &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, glm::dot(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
    }
};
