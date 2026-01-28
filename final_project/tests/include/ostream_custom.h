#ifndef COMPUTERGRAPHICS_OSTREAM_CUSTOM_H
#define COMPUTERGRAPHICS_OSTREAM_CUSTOM_H

// #include <framework/glm_ostream.h>
#include <ostream>
#include "scene.h"
#include "framework/ray.h"
#include <shading.h>
#include <glm/gtx/string_cast.hpp>

namespace glm {

inline std::ostream& operator<<(std::ostream& stream, const glm::vec2& obj)
{
    return stream << glm::to_string(obj);
}

inline std::ostream& operator<<(std::ostream& stream, const glm::vec3& obj)
{
    return stream << glm::to_string(obj);
}

inline std::ostream& operator<<(std::ostream& stream, const glm::uvec3& obj)
{
    return stream << glm::to_string(obj);
}

inline std::ostream& operator<<(std::ostream& stream, const glm::uvec4& obj)
{
    return stream << glm::to_string(obj);
}

inline std::ostream& operator<<(std::ostream& stream, const glm::vec4& obj)
{
    return stream << glm::to_string(obj);
}

inline std::ostream& operator<<(std::ostream& stream, const glm::mat4x4& obj)
{
    return stream << glm::to_string(obj);
}

}

inline std::ostream& operator<<(std::ostream &stream, const Vertex &v)
{
    stream << "Vertex {";
    stream << "position: " << v.position << ", ";
    stream << "normal: " << v.normal << ", ";
    stream << "texCoord: " << v.texCoord;
    stream << "}";
    return stream;
}

inline std::ostream& operator<<(std::ostream &stream, const Material &m)
{
    stream << "Material {";
    stream << "kd: " << m.kd << ", ";
    stream << "ks: " << m.ks << ", ";
    stream << "shininess: " << m.shininess << ", ";
    stream << "transparency: " << m.transparency << ", ";
    stream << "...";
    stream << "}";
    return stream;
}

inline std::ostream& operator<< (std::ostream &stream, const Mesh &mesh)
{
    stream << "Mesh {";
    stream << "vertices: [";
    int i;
    for (i = 0; i < mesh.vertices.size() && i < 50; i++) { // max 50 vertices
        if (i > 0) stream << ", ";
        stream << mesh.vertices[i];
    }
    if (i + 1 < mesh.vertices.size()) stream << ", ...";
    stream << "]";
    stream << ", triangles: [";
    for (i = 0; i < mesh.triangles.size() && i < 50; i++) { // max 50 triangles
        if (i > 0) stream << ", ";
        glm::uvec3 triangle = mesh.triangles[i];
        operator<<(stream, triangle);
        // stream << mesh.triangles[i];
    }
    if (i + 1 < mesh.triangles.size()) stream << ", ...";
    stream << "]";
    stream << ", material: ";
    stream << mesh.material;
    stream << "}";
    return stream;
}

inline std::ostream& operator<< (std::ostream &stream, const Scene &scene)
{
    stream << "Scene {";
    stream << "meshes: [";
    int i;
    for (i = 0; i < scene.meshes.size() && i < 10; i++) { // max 100 meshes
        if (i > 0) stream << ", ";
        stream << scene.meshes[i];
    }
    if (i + 1 < scene.meshes.size()) {
        stream << ", ...";
    }
    stream << "], ... }";
    return stream;
}

inline std::ostream& operator<< (std::ostream &stream, const Ray &ray)
{
    stream << "Ray {";
    stream << "origin: " << ray.origin << ", ";
    stream << "direction: " << ray.direction << ", ";
    stream << "t: " << ray.t;
    stream << "}";
    return stream;
}

inline std::ostream& operator<< (std::ostream &stream, const LinearGradient::Component &comp)
{
    stream << "Component {";
    stream << "color: " << comp.color;
    stream << ", t: " << comp.t;
    stream << "}";
    return stream;
}

inline std::ostream& operator<< (std::ostream &stream, const LinearGradient &lg)
{
    stream << "LinearGradient {";
    stream << "components: [";
    int i;
    for (i = 0; i < lg.components.size() && i < 50; i++) {  // max 50 components
        if (i > 0) stream << ", ";
        stream << lg.components[i];
    }
    if (i+1 < lg.components.size()) stream << ", ...";
    stream << "]";
    stream << "}";
    return stream;
}


#endif //COMPUTERGRAPHICS_OSTREAM_CUSTOM_H
