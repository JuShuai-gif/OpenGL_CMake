#pragma once
#include<glm/glm.hpp>

class Light
{
public:
    // 灯光位置
    glm::vec3 position;
    // 灯光强度
    glm::vec3 intensity;
    
public:
    // 对灯光属性进行赋值
    Light(const glm::vec3& p,const glm::vec3& i):position(p),intensity(i){}
    virtual ~Light() = default;
};
