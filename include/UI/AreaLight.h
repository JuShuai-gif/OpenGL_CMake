#pragma once
#include <glm/glm.hpp>
#include "Light.h"
#include "global.h"

class AreaLight : public Light
{
public:
    // 长度
    float length;
    // 法线
    glm::vec3 normal;
    // 水平方向坐标
    glm::vec3 u;
    // 竖直方向坐标
    glm::vec3 v;
    
public:
    // 对面光源属性进行赋值
    AreaLight(const glm::vec3& p,const glm::vec3& i):Light(p,i){
        normal = glm::vec3(0,-1,0);
        u = glm::vec3(1,0,0);
        v = glm::vec3(0,0,1);
        length = 100;
    }
    // 获取采样点
    glm::vec3 SamplePoint()const{
        auto random_u = get_random_float();
        auto random_v = get_random_float();
        return position + random_u * u + random_v * v;
    }
};

