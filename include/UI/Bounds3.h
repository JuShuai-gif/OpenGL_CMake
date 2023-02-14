#pragma once
#include "Ray.h"
#include "glm/glm.hpp"
#include <limits>
#include <array>

class Bounds3
{
private:
    
public:
    // 包围盒左下角(pMin),右上角(pMax)
    glm::vec3 pMin,pMax;

    Bounds3(){
        // 获取double中的最大值和最小值
        double minNum = std::numeric_limits<double>::lowest();
        double maxNum = std::numeric_limits<double>::max();
        // 构建pMax,pMin
        pMax = glm::vec3(minNum,minNum,minNum);
        pMin = glm::vec3(maxNum,maxNum,maxNum);
    }

    Bounds3(const glm::vec3 p):pMin(p),pMax(p){}
    
    Bounds3(const glm::vec3 p1,const glm::vec3 p2){
        pMin = glm::vec3(std::min(p1.x,p2.x),std::min(p1.y,p2.y),std::min(p1.z,p2.z));
        pMax = glm::vec3(std::max(p1.x,p2.x),std::max(p1.y,p2.y),std::max(p1.z,p2.z));
    }

    // 对角线向量
    glm::vec3 Diagonal()const{
        return pMax - pMin;
    }
    // 获取最长的坐标轴
    int maxExtent()const{
        glm::vec3 d = Diagonal();
        // x 坐标最大
        if (d.x > d.y && d.x > d.z){
            return 0;
        }
        // y 坐标最大
        else if(d.y > d.z)
            return 1;
        // z 坐标最大
        else
            return 2;
    }
    // 包围盒的面积
    double SurfaceArea()const{
        glm::vec3 d = Diagonal();
        return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }
    // 中心点坐标
    glm::vec3 Centroid(){
        return 0.5f * pMin + 0.5f * pMax;
    }
    // 两个包围盒合并一个
    Bounds3 Intersect(const Bounds3& b){
        return Bounds3(glm::vec3(std::max(pMin.x, b.pMin.x),std::max(pMin.y, b.pMin.y),
                                    std::max(pMin.z, b.pMin.z)),
                        glm::vec3(std::min(pMax.x, b.pMax.x),std::fmin(pMax.y, b.pMax.y),
                                    std::fmin(pMax.z, b.pMax.z)));
    }
    // 
    glm::vec3 Offset(const glm::vec3& p)const{
        glm::vec3 o = p - pMin;
        if(pMax.x > pMin.x)
            o.x /= pMax.x - pMin.x;
        if(pMax.y > pMin.y)
            o.y /= pMax.y - pMin.y;
        if(pMax.z > pMin.z)
            o.z /= pMax.z - pMin.z;
        return o;
    }

    // 判断包围盒是否相交
    bool Overlaps(const Bounds3& b1,const Bounds3& b2){
        bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
        bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
        bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
        return (x && y && z);
    }

    //判断点是否在包围盒中
    bool Inside(const glm::vec3& p,const Bounds3& b){
        return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                    p.y <= b.pMax.y && p.z >=b.pMin.z && p.z <= b.pMax.z);
    }

    inline const glm::vec3& operator[](int i)const{
        return (i == 0) ? pMin : pMax; 
    }

    inline bool IntersectP(const Ray& ray,const glm::vec3& invDir,
                            const std::array<int,3>& dirisNeg)const;

};

// 射线与包围盒求交
inline bool Bounds3::IntersectP(const Ray& ray,const glm::vec3& invDir,
                                const std::array<int,3>& dirisNeg)const
{
    float txmin,txmax,tymin,tymax,tzmin,tzmax;
    // 下面分别求x、y、z方向上的投影（这样计算比较简便）
    txmin = (pMin.x - ray.origin.x) * invDir.x;
    txmax = (pMax.x - ray.origin.x) * invDir.x;

    tymin = (pMin.y - ray.origin.y) * invDir.y;
    tymax = (pMax.y - ray.origin.y) * invDir.y;

    tzmin = (pMin.z - ray.origin.z) * invDir.z;
    tzmax = (pMax.z - ray.origin.z) * invDir.z;
    // 若x坐标为负，则对换x时间
    if(dirisNeg.at(0) < 0)
        std::swap(txmin,txmax);
    // 若y坐标为负，则对换y时间
    if(dirisNeg.at(1) < 0)
        std::swap(tymin,tymax);
    // 若z坐标为负，则对换z时间
    if(dirisNeg.at(2) < 0)
        std::swap(tzmin,tzmax);
    float tEnter = std::min(txmin,std::min(tymin,tzmin));
    float tExit = std::max(txmax,std::max(tymax,tzmax));

    if(tEnter < tExit && tExit >=0)
        return true;
    else 
        return false;
}
// 两个包围盒合并为一个大的包围盒
inline Bounds3 Union(const Bounds3& b1,const Bounds3& b2){
    Bounds3 ret;
    if (b1.pMin.x < b2.pMin.x && b1.pMin.y < b2.pMin.y && b1.pMin.z < b2.pMin.z)
        ret.pMin = b1.pMin;
    else
        ret.pMin = b2.pMin;
    
    if (b1.pMax.x < b2.pMax.x && b1.pMax.y < b2.pMax.y && b1.pMax.z < b2.pMax.z)
        ret.pMax = b1.pMax;
    else
        ret.pMax = b2.pMax;
    
    return ret;
}
// 一个包围盒和一个向量合并为一个大的包围盒
inline Bounds3 Union(const Bounds3& b,const glm::vec3& p){
    Bounds3 ret;
    if (b.pMin.x < p.x && b.pMin.y < p.y && b.pMin.z < p.z)
        ret.pMin = b.pMin;
    else
        ret.pMin = p;
    
    if (b.pMax.x < p.x && b.pMax.y < p.y && b.pMax.z < p.z)
        ret.pMax = b.pMax;
    else
        ret.pMax = p;
    
    return ret;
}