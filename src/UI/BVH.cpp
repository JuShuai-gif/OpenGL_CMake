#include<algorithm>
#include<cassert>
#include"BVH.h"

// BVH构造函数
BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    // 开始时间、结束时间
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;
    //构建BVH树
    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}
// 构建BVH树
BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    // 根节点
    BVHBuildNode* node = new BVHBuildNode();
    // 计算所有原始碰撞盒
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    // 物体数量为1的情况
    if (objects.size() == 1) {
        // 根节点的碰撞盒即为该物体的碰撞盒
        node->bounds = objects[0]->getBounds();   
        // 根节点的物体即为当前唯一的一个物体
        node->object = objects[0];
        // 左节点为空
        node->left = nullptr;
        // 右节点为空
        node->right = nullptr;
        // 面积
        node->area = objects[0]->getArea();
        return node;
    }
    // 物体数量为2的情况
    else if (objects.size() == 2) {
        // 左节点
        node->left = recursiveBuild(std::vector{objects[0]});
        // 右节点
        node->right = recursiveBuild(std::vector{objects[1]});
        // 将左右包围盒合成一个大的包围盒
        node->bounds = Union(node->left->bounds, node->right->bounds);
        // 左边面积 + 右边面积
        node->area = node->left->area + node->right->area;
        return node;
    }
    else {
        // 中心体包围盒
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds = Union(centroidBounds, objects[i]->getBounds().Centroid());
        // 获取所有物体在那个轴上跨度最大
        int dim = centroidBounds.maxExtent();
        // 0表示x轴，1表示y轴，2表示z轴
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                // 按x轴进行排序
                return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                // 按y轴进行排序
                return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                // 按z轴进行排序
                return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
    }

    return node;
}
// 射线求交
Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}
// 
Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // 求交信息
    Intersection isect;
    // 当射线没有打到该BVH
    if (!node->bounds.IntersectP(ray,ray.direction_inv,std::array<int,3>{ray.direction.x > 0,ray.direction.y > 0,ray.direction.z > 0}))
        return isect;
    // 当射线打到该BVH
    if (node->object != nullptr)
        return node->object->getIntersection(ray);

    Intersection isect_left,isect_right;
    isect_left = getIntersection(node->left,ray);
    isect_right = getIntersection(node->right,ray);
    return isect_left.distance <= isect_right.distance ? isect_left : isect_right;
}


void BVHAccel::getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf){
    if(node->left == nullptr || node->right == nullptr){
        node->object->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if(p < node->left->area) getSample(node->left, p, pos, pdf);
    else getSample(node->right, p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, float &pdf){
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root, p, pos, pdf);
    // pdf
    pdf /= root->area;
}
