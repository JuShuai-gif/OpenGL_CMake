#pragma once
#include<glm/glm.hpp>
#include "Material.h"
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false;
        coords=glm::vec3();
        normal=glm::vec3();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
    }
    bool happened;      // 是否发生碰撞
    glm::vec3 coords;   // 碰撞坐标
    glm::vec3 tcoords;  // 碰撞贴图坐标
    glm::vec3 normal;   // 碰撞法线
    glm::vec3 emit;     // 
    double distance;    // 碰撞距离
    Object* obj;        // 碰撞物体
    Material* m;        // 碰撞物体材质
};