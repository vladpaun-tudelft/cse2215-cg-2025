// ! You may copy this file's content to the solution.cpp file in the weblab environment.
#include "library.cpp"

// *** DO NOT CHANGE THIS FUNCTION ***
// draws one stone centered at the origin
void drawStone (const float w, const float h, const float d, const glm::vec3& color) {
    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (0.0f, 0.0f, -1.0f);
    glVertex3f (-w*0.5f,  -h*0.5f, -d*0.5f);
    glVertex3f ( w*0.5f,  -h*0.5f, -d*0.5f);
    glVertex3f (-w*0.5f,   h*0.5f, -d*0.5f);
    glVertex3f ( w*0.5f,   h*0.5f, -d*0.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (0.0f, 0.0f, 1.0f);
    glVertex3f (-w*0.5f,  -h*0.5f,  d*0.5f);
    glVertex3f ( w*0.5f,  -h*0.5f,  d*0.5f);
    glVertex3f (-w*0.5f,   h*0.5f,  d*0.5f);
    glVertex3f ( w*0.5f,   h*0.5f,  d*0.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (-1.0f, 0.0f, 0.0f);
    glVertex3f (-w*0.5f,  -h*0.5f,  -d*0.5f);
    glVertex3f (-w*0.5f,  -h*0.5f,   d*0.5f);
    glVertex3f (-w*0.5f,   h*0.5f,  -d*0.5f);
    glVertex3f (-w*0.5f,   h*0.5f,   d*0.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (1.0f, 0.0f, 0.0f);
    glVertex3f (w*0.5f,  -h*0.5f,  -d*0.5f);
    glVertex3f (w*0.5f,  -h*0.5f,   d*0.5f);
    glVertex3f (w*0.5f,   h*0.5f,  -d*0.5f);
    glVertex3f (w*0.5f,   h*0.5f,   d*0.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (0.0f, -1.0f, 0.0f);
    glVertex3f (-w*0.5f,  -h*0.5f,  -d*0.5f);
    glVertex3f (-w*0.5f,  -h*0.5f,   d*0.5f);
    glVertex3f ( w*0.5f,  -h*0.5f,  -d*0.5f);
    glVertex3f ( w*0.5f,  -h*0.5f,   d*0.5f);
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    glColor3fv (glm::value_ptr(color));
    glNormal3f (0.0f, 1.0f, 0.0f);
    glVertex3f (-w*0.5f,  h*0.5f,  -d*0.5f);
    glVertex3f (-w*0.5f,  h*0.5f,   d*0.5f);
    glVertex3f ( w*0.5f,  h*0.5f,  -d*0.5f);
    glVertex3f ( w*0.5f,  h*0.5f,   d*0.5f);
    glEnd();

}

// *** ONLY WHAT IS INSIDE the drawStonehenge FUNCTION IS EVALUATED ***
// Draws Stonehenge given a number of stading stones, a radius, and the stones' dimensions
// The stones should respect the given dimensions
// One standing stone should be placed along the z axis centered at x=0.
// Consider the floor at y=0, standing stones are standing on the floor
// The radius is measured on the xz-plane from the origin to the center of a standing stone
void drawStonehenge(const int n, const float r, const float w, const float h, const float d) {
    // a glm 4x4 identity matrix
    glm::mat4 transform (1.0f);

    // to reset, load or multiply a matrix to the GL_MODELVIEW stack use:
    // glMultMatrixf(glm::value_ptr(transform)); multiplies the current matrix by transform
    // glLoadIdentity(); // resets matrix on top of the stack to identity
    // glLoadMatrixf(glm::value_ptr(transform)); // loads a new matrix on top of the stack
    // !!NOTE!!: be careful if you use glLoadIdentity or glLoadMatrix, it will overwrite the current modelview matrix

    // you can push and pop from the stack with:
    // glPushMatrix();
    // glPopMatrix();

    // you can use the provided matrix functions in the Library Code. For example:
    //transform = translationMatrix (glm::vec(x,y,z));
    //transform = rotationMatrix{X,Y,Z} (angle_in_radians);

    // cos and sin functions are available in the glm namespace (glm:cos() and glm::sin())
    // Note that cos and sin expect an angle in radians! You can use glm::radians to convert from degrees to radians

    // here are two dummy transformation for reference, they do nothing
    transform = translationMatrix(glm::vec3(0.0f, 0.0f, 0.0f));
    transform = transform * rotationMatrixX(0.0f);

    const float pi = glm::pi<float>();
    glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.0f);    

    glPushMatrix();
    glMultMatrixf(glm::value_ptr(transform));
    drawStone(w, h, d, color);
    glPopMatrix();      
}
