#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "BVH.h"
#include "Intersection.h"
#include "Material.h"
#include "OBJ_Loader.hpp"
#include "Object.h"
#include "Triangle.h"
#include "shader.h"
#include <cassert>
#include <array>

bool rayTriangleIntersect(const glm::vec3& v0, const glm::vec3& v1,
                          const glm::vec3& v2, const glm::vec3& orig,
                          const glm::vec3& dir, float& tnear, float& u, float& v)
{
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 pvec = glm::cross(dir, edge2);
    float det = glm::dot(edge1, pvec);
    if (det == 0 || det < 0)
        return false;

    glm::vec3 tvec = orig - v0;
    u = glm::dot(tvec, pvec);
    if (u < 0 || u > det)
        return false;

    glm::vec3 qvec = glm::cross(tvec, edge1);
    v = glm::dot(dir, qvec);
    if (v < 0 || u + v > det)
        return false;

    float invDet = 1 / det;

    tnear = glm::dot(edge2, qvec) * invDet;
    u *= invDet;
    v *= invDet;

    return true;
}

class Triangle : public Object
{
public:
    glm::vec3 v0, v1, v2; // vertices A, B ,C , counter-clockwise order
    glm::vec3 e1, e2;     // 2 edges v1-v0, v2-v0;
    glm::vec3 t0, t1, t2; // texture coords
    glm::vec3 normal;
    float area;
    Material* m;

    Triangle(glm::vec3 _v0, glm::vec3 _v1, glm::vec3 _v2, Material* _m = nullptr)
        : v0(_v0), v1(_v1), v2(_v2), m(_m)
    {
        e1 = v1 - v0;
        e2 = v2 - v0;
        normal = glm::normalize(glm::cross(e1, e2));
        area = glm::cross(e1, e2).length()*0.5f;
    }

    bool intersect(const Ray& ray) override;
    bool intersect(const Ray& ray, float& tnear,
                   uint32_t& index) const override;
    Intersection getIntersection(Ray ray) override;
    void getSurfaceProperties(const glm::vec3& P, const glm::vec3& I,
                              const uint32_t& index, const glm::vec2& uv,
                              glm::vec3& N, glm::vec2& st) const override
    {
        N = normal;
        //        throw std::runtime_error("triangle::getSurfaceProperties not
        //        implemented.");
    }
    glm::vec3 evalDiffuseColor(const glm::vec2&) const override;
    Bounds3 getBounds() override;
    void Sample(Intersection &pos, float &pdf){
        float x = std::sqrt(get_random_float()), y = get_random_float();
        pos.coords = v0 * (1.0f - x) + v1 * (x * (1.0f - y)) + v2 * (x * y);
        pos.normal = this->normal;
        pdf = 1.0f / area;
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }
};

class MeshTriangle : public Object
{
public:
    MeshTriangle(const std::string& filename, Material *mt = new Material())
    {
        objl::Loader loader;
        loader.LoadFile(filename);
        area = 0;
        m = mt;
        assert(loader.LoadedMeshes.size() == 1);
        auto mesh = loader.LoadedMeshes[0];

        glm::vec3 min_vert = glm::vec3{std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity(),
                                     std::numeric_limits<float>::infinity()};
        glm::vec3 max_vert = glm::vec3{-std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity(),
                                     -std::numeric_limits<float>::infinity()};
        
        // 经过验证，读取的顶点已经是按照EBO排序
        std::cout<< mesh.Vertices.size()<<std::endl;
        std::cout<< mesh.Indices.size()<<std::endl;

        for (int i = 0; i < mesh.Vertices.size(); i += 3) {
            std::array<glm::vec3, 3> face_vertices;

            for (int j = 0; j < 3; j++) {
                auto vert = glm::vec3(mesh.Vertices[i + j].Position.X,
                                     mesh.Vertices[i + j].Position.Y,
                                     mesh.Vertices[i + j].Position.Z);
                face_vertices[j] = vert;

                min_vert = glm::vec3(std::min(min_vert.x, vert.x),
                                    std::min(min_vert.y, vert.y),
                                    std::min(min_vert.z, vert.z));
                max_vert = glm::vec3(std::max(max_vert.x, vert.x),
                                    std::max(max_vert.y, vert.y),
                                    std::max(max_vert.z, vert.z));
            }

            triangles.emplace_back(face_vertices[0], face_vertices[1],
                                   face_vertices[2], mt);
        }

        bounding_box = Bounds3(min_vert, max_vert);

        std::vector<Object*> ptrs;
        for (auto& tri : triangles){
            ptrs.push_back(&tri);
            // 物体的面积是由每个三角形的面积共同组成
            area += tri.area;
        }
        bvh = new BVHAccel(ptrs);
    }

    bool intersect(const Ray& ray) { return true; }

    bool intersect(const Ray& ray, float& tnear, uint32_t& index) const
    {
        bool intersect = false;
        for (uint32_t k = 0; k < numTriangles; ++k) {
            const glm::vec3& v0 = vertices[vertexIndex[k * 3]];
            const glm::vec3& v1 = vertices[vertexIndex[k * 3 + 1]];
            const glm::vec3& v2 = vertices[vertexIndex[k * 3 + 2]];
            float t, u, v;
            if (rayTriangleIntersect(v0, v1, v2, ray.origin, ray.direction, t,
                                     u, v) &&
                t < tnear) {
                tnear = t;
                index = k;
                intersect |= true;
            }
        }

        return intersect;
    }

    Bounds3 getBounds() { return bounding_box; }

    void getSurfaceProperties(const glm::vec3& P, const glm::vec3& I,
                              const uint32_t& index, const glm::vec2& uv,
                              glm::vec3& N, glm::vec2& st) const
    {
        const glm::vec3& v0 = vertices[vertexIndex[index * 3]];
        const glm::vec3& v1 = vertices[vertexIndex[index * 3 + 1]];
        const glm::vec3& v2 = vertices[vertexIndex[index * 3 + 2]];
        glm::vec3 e0 = normalize(v1 - v0);
        glm::vec3 e1 = normalize(v2 - v1);
        N = glm::normalize(glm::cross(e0, e1));
        const glm::vec2& st0 = stCoordinates[vertexIndex[index * 3]];
        const glm::vec2& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
        const glm::vec2& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
        st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
    }

    glm::vec3 evalDiffuseColor(const glm::vec2& st) const
    {
        float scale = 5;
        float pattern =
            (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
        return lerp(glm::vec3(0.815, 0.235, 0.031),
                    glm::vec3(0.937, 0.937, 0.231), pattern);
    }

    Intersection getIntersection(Ray ray)
    {
        Intersection intersec;

        if (bvh) {
            intersec = bvh->Intersect(ray);
        }

        return intersec;
    }
    
    void Sample(Intersection &pos, float &pdf){
        bvh->Sample(pos, pdf);
        pos.emit = m->getEmission();
    }
    float getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }

    void SetDataOpenGL(){
        std::cout<< "总共三角形个数：" << triangles.size() <<std::endl;

        for (auto& tri : triangles){
            positions.push_back(tri.v0);
            positions.push_back(tri.v1);
            positions.push_back(tri.v2);
            normals.push_back(tri.normal);
            normals.push_back(tri.normal);
            normals.push_back(tri.normal);
        }
        glGenBuffers(1,&VBO);
        glBindBuffer(GL_ARRAY_BUFFER,VBO);
        GLuint dataSize = sizeof(glm::vec3) * positions.size() + sizeof(glm::vec3) * normals.size();
        glBufferData(GL_ARRAY_BUFFER,dataSize,NULL,GL_STATIC_DRAW);

        GLuint pointDataOffset = 0;
        GLuint normalsDataOffset = sizeof(glm::vec3) * positions.size();
        glBufferSubData(GL_ARRAY_BUFFER,pointDataOffset,sizeof(glm::vec3) * positions.size(),&positions[0]);
        glBufferSubData(GL_ARRAY_BUFFER, normalsDataOffset, sizeof(glm::vec3) * normals.size(), &normals[0]);

        // 生成vao对象并且绑定vao
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);  
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)(sizeof(glm::vec3) * positions.size())); 
        glEnableVertexAttribArray(1);
    }

    void Draw(Shader& shader){
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, positions.size());
    }
public:
    Bounds3 bounding_box;
    std::unique_ptr<glm::vec3[]> vertices;
    uint32_t numTriangles;
    std::unique_ptr<uint32_t[]> vertexIndex;
    std::unique_ptr<glm::vec2[]> stCoordinates;
    std::vector<glm::vec3> positions,normals;
    std::vector<Triangle> triangles;

    BVHAccel* bvh;
    float area;

    Material* m;

    GLuint VAO,VBO;
};

inline bool Triangle::intersect(const Ray& ray) { return true; }
inline bool Triangle::intersect(const Ray& ray, float& tnear,
                                uint32_t& index) const
{
    return false;
}

inline Bounds3 Triangle::getBounds() { return Union(Bounds3(v0, v1), v2); }

inline Intersection Triangle::getIntersection(Ray ray)
{
    Intersection inter;

    if (glm::dot(ray.direction, normal) > 0)
        return inter;
    double u, v, t_tmp = 0;
    glm::vec3 pvec = glm::cross(ray.direction, e2);
    double det = glm::dot(e1, pvec);
    if (fabs(det) < EPSILON)
        return inter;

    double det_inv = 1. / det;
    glm::vec3 tvec = ray.origin - v0;
    u = glm::dot(tvec, pvec) * det_inv;
    if (u < 0 || u > 1)
        return inter;
    glm::vec3 qvec = glm::cross(tvec, e1);
    v = glm::dot(ray.direction, qvec) * det_inv;
    if (v < 0 || u + v > 1)
        return inter;
    t_tmp = glm::dot(e2, qvec) * det_inv;

    // TODO find ray triangle intersection
    if (t_tmp < 0)
        return inter;
    inter.happened = true;
    inter.coords = ray(t_tmp);
    inter.normal = normal;
    inter.distance = t_tmp;
    inter.obj = this;
    inter.m = m;
    
    return inter;
}

inline glm::vec3 Triangle::evalDiffuseColor(const glm::vec2&) const
{
    return glm::vec3(0.5, 0.5, 0.5);
}
