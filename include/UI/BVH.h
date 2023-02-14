#pragma once
#include<atomic>
#include<memory>
#include<vector>
#include<ctime>
#include <glm/glm.hpp>
#include "Object.h"
#include "Ray.h"
#include "Bounds3.h"
#include "Intersection.h"

// BVH 节点
struct BVHBuildNode;
struct BVHPrimitiveInfo;

inline int leafNodes,totalLeafNodes,totalPrimitives,interiorNodes;

/**************************************/
class BVHAccel
{
public:
    // BVH 创建类型
    enum class SplitMethod{NAIVE, SAH};

    BVHAccel(std::vector<Object*> p,int maxPrimsInNode = 1,SplitMethod splitMethod = SplitMethod::NAIVE);
    Bounds3 WorldBound()const;
    ~BVHAccel();

    Intersection Intersect(const Ray& ray)const;
    Intersection getIntersection(BVHBuildNode* node,const Ray& ray)const;
    bool IntersectP(const Ray& ray)const;
    BVHBuildNode* root;

    BVHBuildNode* recursiveBuild(std::vector<Object*> objects);

    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<Object*> primitives;

    void getSample(BVHBuildNode* node,float p,Intersection& pos,float& pdf);
    void Sample(Intersection& pos,float& pdf);
};

struct BVHBuildNode
{
    Bounds3 bounds;
    BVHBuildNode* left;
    BVHBuildNode* right;
    Object* object;
    float area;
public:
    int splitAxis = 0,firstPrimOffset = 0,nPrimitives = 0;
    BVHBuildNode(){
        bounds = Bounds3();
        left = nullptr;
        right = nullptr;
        object = nullptr;
    }
};

