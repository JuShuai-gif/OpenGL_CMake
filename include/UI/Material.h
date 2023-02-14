#pragma once
#include<glm/glm.hpp>
#include "global.h"
enum MaterialType{DIFFUSE};

class Material
{
private:
    //计算反射
    // 注意这个方向，wi和wo都是朝外的
    glm::vec3 reflect(const glm::vec3& I,const glm::vec3& N){
        return I - 2 * glm::dot(I,N) * N;
    }
    // 计算折射
    glm::vec3 refract(const glm::vec3& I,const glm::vec3& N,const float& ior)const{
        float cosi = clamp(-1,1,glm::dot(I,N));
        float etai = 1,etat = ior;
        glm::vec3 n = N;
        if(cosi < 0){
            cosi = -cosi;
        }else{
            std::swap(etai,etat);
            n = -N;
        }
        float eta = etai/etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? glm::vec3(0) :eta * I + (eta * cosi - sqrtf(k)) * n;
    }
    //菲涅尔反射
    void fresnel(const glm::vec3& I,const glm::vec3& N,const float& ior,float& kr)const{
        float cosi = clamp(-1,1,glm::dot(I,N));
        float etai = 1,etat = ior;
        if(cosi > 0){std::swap(etai,etat);}
        float sint = etai / etat * sqrtf(std::max(0.f,1 - cosi *cosi));
        if(sint >= 1){
            kr = 1;
        }
        else{
            float cost = sqrtf(std::max(0.f,1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs +Rp * Rp) / 2;
        }
    }

    glm::vec3 toWorld(const glm::vec3& a,const glm::vec3& N){
        glm::vec3 B,C;
        if(std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = glm::vec3(N.z * invLen, 0.0f, -N.x * invLen);
        }        
        else{
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = glm::vec3(0.0f,N.z * invLen,-N.y * invLen);
        }
        B = glm::cross(C,N);
        return a.x * B + a.y * C + a.z * N;
    }
public:
    MaterialType m_type;        // 材质类型
    glm::vec3 m_emission;       // 自发光颜色
    float ior;                  // 折射率
    glm::vec3 Kd,Ks;            // 漫反射和高光
    float specularExponent;     // 高光系数

    inline Material(MaterialType t = DIFFUSE,glm::vec3 e = glm::vec3(0.0f));
    inline MaterialType getType();
    inline glm::vec3 getColorAt(float u,float v);
    inline glm::vec3 getEmission();
    inline bool hasEmission();

    inline glm::vec3 sample(const glm::vec3& wi,const glm::vec3& N);
    inline float pdf(const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& N);
    inline glm::vec3 eval(const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& N);
};

Material::Material(MaterialType t,glm::vec3 e){
    m_type = t;
    m_emission = e;
}

MaterialType Material::getType(){
    return m_type;
}

glm::vec3 Material::getEmission(){
    return m_emission;
}

bool Material::hasEmission(){
    if(glm::length(m_emission) > EPSILON) return true;
    else return false;
}

glm::vec3 Material::getColorAt(float u,float v){
    return glm::vec3(0);
}

glm::vec3 Material::sample(const glm::vec3& wi,const glm::vec3& N){
    switch (m_type)
    {
        case DIFFUSE:
        {
            float x_1 = get_random_float(),x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z),phi = 2 * M_PI * x_2;
            glm::vec3 localRay(r * std::cos(phi),r * std::sin(phi),z);
            return toWorld(localRay,N);
            break;
        }
    }
}

float Material::pdf(const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& N){
    switch (m_type)
    {
        case DIFFUSE:
        {
            if(glm::dot(wo,N) > 0.0f)
                return 0.5f / M_PI;
            else
                return 0.0f;
            break;
        }
    }
}

glm::vec3 Material::eval(const glm::vec3& wi,const glm::vec3& wo,const glm::vec3& N){
    switch (m_type)
    {
        case DIFFUSE:
        {
            float cosalpha = glm::dot(N,wo);
            if(cosalpha > 0.0f){
                glm::vec3 diffuse = Kd / M_PI;
                return diffuse;
            }
            else
                return glm::vec3(0.0f);
            break;
        }
    }
}