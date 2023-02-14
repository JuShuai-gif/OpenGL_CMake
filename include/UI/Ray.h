#pragma once

#include<glm/glm.hpp>
#include<iostream>

struct Ray{
    //Destination = origin + t*direction
    // 源点
    glm::vec3 origin;
    // 方向、方向分量倒数
    glm::vec3 direction, direction_inv;
    // 位移时间t
    double t;
    // 时间最小值、最大值
    double t_min, t_max;

    Ray(const glm::vec3& ori, const glm::vec3& dir, const double _t = 0.0): origin(ori), direction(dir),t(_t) {
        direction_inv = glm::vec3(1./direction.x, 1./direction.y, 1./direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    glm::vec3 operator()(float t) const{return origin + direction * t;}

    friend std::ostream &operator<<(std::ostream& os, const Ray& r){
        os<<"[origin:="<<r.origin.x<<","<<r.origin.y<<","<<r.origin.z<<" "
                <<"direction="<<r.direction.x<<","<<r.direction.y<<","<<r.direction.z<<" "
                <<"time="<< r.t<<"]\n";
        return os;
    }
};