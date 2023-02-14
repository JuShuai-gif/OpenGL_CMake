#include "Scene.h"

// 构建BVH
void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}
// 
Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        glm::vec2 uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
glm::vec3 Scene::castRay(const Ray &ray, int depth) const
{
    glm::vec3 L_dir,L_indir; //直接光和间接光

    Intersection inter_p = intersect(ray);

    if (!inter_p.happened){
        return glm::vec3();
    }
    if (inter_p.m->hasEmission()){
        return inter_p.m->getEmission();
    }

    glm::vec3& p = inter_p.coords;
    glm::vec3& N = inter_p.normal;
    glm::vec3 wo = glm::normalize(ray.origin - p);
    Material* m = inter_p.m;

    Intersection inter;
    float pdf_light;
    sampleLight(inter,pdf_light);

    glm::vec3& x = inter.coords;
    glm::vec3& NN = inter.normal;
    glm::vec3& emit = inter.emit;
    glm::vec3 ws = glm::normalize(x - p);
    float d = (x - p).length();
    
    Ray r(p,ws);
    Intersection i = intersect(r);

    if (i.distance - d > -0.001){
        L_dir = emit * m->eval(wo,ws,N) * glm::dot(ws,N) * glm::dot(-ws,NN) / (d*d*pdf_light);
    }

    float f = get_random_float();
    if (f<RussianRoulette){
        glm::vec3 wi =glm::normalize(m->sample(wo,N));
        Ray r(p,wi);
        Intersection i = intersect(r);

        if (i.happened && !i.m->hasEmission()){
            L_indir = castRay(r,depth + 1) * m->eval(wo,wi,N)*glm::dot(wi,N)/m->pdf(wo,wi,N)/RussianRoulette;
        }
    }
    return L_dir + L_indir;
}