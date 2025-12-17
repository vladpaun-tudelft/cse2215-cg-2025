#include "draw.h"
#include <framework/opengl_includes.h>
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#ifdef WIN32
// Windows.h includes a ton of stuff we don't need, this macro tells it to include less junk.
#define WIN32_LEAN_AND_MEAN
// Disable legacy macro of min/max which breaks completely valid C++ code (std::min/std::max won't work).
#define NOMINMAX
// GLU requires Windows.h on Windows :-(.
#include <Windows.h>
#endif
#include <GL/glu.h>
#endif
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <algorithm>

void drawCoordSystem(const float length, std::array<glm::vec3, 3> colors)
{
    glLineWidth(3.0f);
    // Draw axes.
    glBegin(GL_LINES);
    glColor3fv(glm::value_ptr(colors[0]));
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);

    glColor3fv(glm::value_ptr(colors[1]));
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    glColor3fv(glm::value_ptr(colors[2]));
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, length);
    glEnd();
}

void drawSphere(glm::vec3 center, float radius, glm::vec3 color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    const glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), center);
    glMultMatrixf(glm::value_ptr(transform));

    glColor3fv(glm::value_ptr(color));
    auto quadric = gluNewQuadric();
    gluSphere(quadric, static_cast<GLdouble>(radius), 400, 200);
    gluDeleteQuadric(quadric);

    glPopMatrix();
    glPopAttrib();
}

void drawRay(const Ray& ray, std::array<glm::vec3, 3> colors)
{
    const glm::vec3 hitPoint = ray.origin + std::clamp(ray.t, 0.0f, 100.0f) * ray.direction;
    bool hit = ray.t < std::numeric_limits<float>::max();
    glm::vec3 color;
    if (hit) {
        if (ray.t <= glm::sqrt(2.0f)) {
            color = {0.8f, 0.2f, 0.2f};
            color = colors[0];
        }
        else {
            color = {0.8f, 0.8f, 0.2f};
            color = colors[1];
        }
    } else {
        color = {0.2f, 0.8f, 0.2f};
        color=colors[2];
    }

    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3fv(glm::value_ptr(color));
    glVertex3fv(glm::value_ptr(ray.origin));
    glVertex3fv(glm::value_ptr(hitPoint));
    glEnd();

    if (hit) {
        drawSphere(hitPoint, 0.05f, color);
    }
}
