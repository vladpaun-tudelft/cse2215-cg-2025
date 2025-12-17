#include "draw.h"
#include <framework/opengl_includes.h>
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>

void drawTriangle(const std::array<glm::vec3, 3>& points)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glBegin(GL_TRIANGLES);

    glColor3f(0.4f, 0.8f, 0.4f);

    glVertex3f(points[0].x, points[0].y, points[0].z);
    glVertex3f(points[1].x, points[1].y, points[1].z);
    glVertex3f(points[2].x, points[2].y, points[2].z);

    glEnd();

    glLineWidth(3.0);
    glBegin(GL_LINES);
    glColor3f(0.8f, 0.8f, 0.8f);

    glVertex3f(points[0].x, points[0].y, points[0].z);
    glVertex3f(points[1].x, points[1].y, points[1].z);
    glVertex3f(points[2].x, points[2].y, points[2].z);
    glVertex3f(points[0].x, points[0].y, points[0].z);

    glEnd();
    glLineWidth(1.0);

    glPopMatrix();
}

void drawRectangle(const std::array<glm::vec3, 4>& points, const Plane& plane)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glBegin(GL_QUADS);

    glColor3f(0.4f, 0.8f, 0.4f);
    
    glVertex3f(points[0].x, points[0].y, points[0].z);
    glVertex3f(points[1].x, points[1].y, points[1].z);
    glVertex3f(points[2].x, points[2].y, points[2].z);
    glVertex3f(points[3].x, points[3].y, points[3].z);

    glEnd();

    glLineWidth(3.0);
    glBegin(GL_LINES);
    glColor3f(0.8f, 0.8f, 0.8f);

    glVertex3f(plane.p.x, plane.p.y, plane.p.z);
    glVertex3f(plane.p.x + plane.n.x, plane.p.y + plane.n.y, plane.p.z + plane.n.z);
    glEnd();
    glLineWidth(1.0);

    glPopMatrix();
}

void drawSphere(const glm::vec3& position, float radius, float red, float green, float blue)
{
    glColor3f(red, green, blue);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    const auto translate = glm::translate(glm::identity<glm::mat4>(), position);
    glMultMatrixf(glm::value_ptr(translate));

    auto sphere = gluNewQuadric();
    gluQuadricNormals(sphere, GLU_SMOOTH);
    gluSphere(sphere, radius, 20, 10);
    gluDeleteQuadric(sphere);

    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void drawCoordSystem(const float length)
{
    // Remember all states of the GPU.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    // Deactivate the lighting state.
    glDisable(GL_LIGHTING);

    // Draw axes.
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(length, 0, 0);

    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, length, 0);

    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, length);
    glEnd();

    // Restore previous GPU state.
    glPopAttrib();
}
