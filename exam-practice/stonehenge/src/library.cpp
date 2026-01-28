// ! DO NOT CHANGE THIS FILE!
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
DISABLE_WARNINGS_POP()

#include <framework/trackball.h>
#include <framework/window.h>
#include <framework/opengl_includes.h>
#include <framework/image.h>
#include <framework/window.h>

#include <iostream>


//////Predefined global variables

bool show_imgui = true;
constexpr glm::ivec2 resolution { 800, 800 };
std::unique_ptr<Window> pWindow;
std::unique_ptr<Trackball> pTrackball;

void drawStonehenge(const int n, const float r, const float w, const float h, const float d);


glm::mat4 translationMatrix (const glm::vec3 t) {
        // 3d translation
        return glm::mat4 (1.0, 0.0, 0.0, 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          t[0], t[1], t[2], 1.0);

}

glm::mat4 rotationMatrixX (float angle) {
        // X rotation
        return glm::mat4 (1.0, 0.0, 0.0, 0.0,
                          0.0, cos(angle), sin(angle), 0.0,
                          0.0, -sin(angle), cos(angle), 0.0,
                          0.0, 0.0, 0.0, 1.0 );
}

glm::mat4 rotationMatrixY (float angle) {
        // Y rotation
        return glm::mat4 (cos(angle), 0.0, -sin(angle), 0.0,
                          0.0, 1.0, 0.0, 0.0,
                          sin(angle), 0.0, cos(angle), 0.0,
                          0.0, 0.0, 0.0, 1.0 );
}

glm::mat4 rotationMatrixZ (float angle) {

        // Z rotation
        return glm::mat4 (cos(angle), sin(angle), 0.0, 0.0,
                          -sin(angle), cos(angle), 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.0, 0.0, 0.0, 1.0 );

}

glm::mat4 scaleMatrix (const glm::vec3 s) {
        // 3d scale
        return glm::mat4 (s[0],  0.0,  0.0, 0.0,
                           0.0, s[1],  0.0, 0.0,
                           0.0,  0.0, s[2], 0.0,
                           0.0,  0.0,  0.0, 1.0);

}

glm::mat4 scaleMatrix (const float s) {
        // 3d isotropic scale
        return glm::mat4 (  s, 0.0, 0.0, 0.0,
                          0.0,   s, 0.0, 0.0,
                          0.0, 0.0,   s, 0.0,
                          0.0, 0.0, 0.0, 1.0);
}

void initializeGL (Window &window) {

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        std::cerr << "Could not initialize GLEW" << std::endl;
        exit(1);
    }

    window.updateInput();
    
    glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

}


void setupLight(const glm::vec3 lightpos) {
    glEnable (GL_LIGHTING) ;
    glEnable (GL_LIGHT0);

    glm::vec4 lightkd (1.0, 1.0, 1.0, 1.0);
    glm::vec4 lightka (0.3, 0.3, 0.3, 1.0);
    glm::vec4 lightks (1.0, 1.0, 1.0, 1.0);

    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(lightka));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(lightkd));
    glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(lightks));
    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(lightpos));
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void clearScreen (Window &window) {
    // Clear screen.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void loadMatrices (Window &window) {
    glMatrixMode(GL_PROJECTION);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 
                        (float)window.getWindowSize().x/(float)window.getWindowSize().y, 0.1f, 300.0f);
    glLoadMatrixf(glm::value_ptr(projectionMatrix));
    
    glMatrixMode(GL_MODELVIEW);
}

void saveResult (Window &window, const char* filename ) {

    window.renderToImage(filename, true);
    window.swapBuffers();

    auto size = std::filesystem::file_size(filename);

    if (size > 1048576) {
        std::cerr << "Warning: output file larger than 1MB." << std::endl;
        std::cerr << "Warning: The image may not appear in Weblab. Decrease the resolution in the main function if this is the case." << std::endl;
    }
}

////////// Draw Functions

// function to draw coordinate axes with a certain length (1 as a default)
void drawCoordSystem(const float length = 1)
{
    // draw simply colored axes

    // remember all states of the GPU
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    // deactivate the lighting state
    glDisable(GL_LIGHTING);
    // draw axes
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

    // reset to previous state
    glPopAttrib();
}

/**
 * Objects to be rendered
 */
void display()
{
    setupLight(glm::vec3(0.0f, 3.0f, 5.0f));

    // draws the x (red), y (green) and z (blue) axes
    drawCoordSystem(3.0f);

    // you may change these values to test different combinations, your solution should work for any given parameters
    // and not only the default ones below
    int num_stones = 20;
    float stone_w = 2.5f;
    float stone_h = 4.5f;
    float stone_d = 1.0f;
    float radius = 18;

    // note that the drawStonehenge function is called between Push and Pop
    glPushMatrix();
    drawStonehenge(num_stones, radius, stone_w, stone_h, stone_d);    
    glPopMatrix();
}

void displayInternal(void);
void reshape(const glm::ivec2&);
void init()
{    
    pTrackball->setCamera(
        glm::vec3(0.0f, 0.0f, 0.0f), // lookAt
        glm::vec3(glm::radians(30.0f), 0.0f, 0.0f), // rotations
        50.0f                         // distance
    );

    // Initialize viewpoint
    pTrackball->printHelp();
    reshape(resolution);

    glDisable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    // Enable Depth test
    glEnable(GL_DEPTH_TEST);

    // Draw frontfacing polygons as filled.
    glPolygonMode(GL_FRONT, GL_FILL);
    // Draw backfacing polygons as outlined.
    glPolygonMode(GL_BACK, GL_LINE);
    glShadeModel(GL_SMOOTH);
}

// Program entry point. Everything starts here.
int main(int /* argc */, char** argv)
{
    pWindow = std::make_unique<Window>(argv[0], resolution, OpenGLVersion::GL2);
    pTrackball = std::make_unique<Trackball>(pWindow.get(), glm::radians(50.0f));
    pWindow->registerWindowResizeCallback(reshape);

    init();

    while (!pWindow->shouldClose()) {
        pWindow->updateInput();

        displayInternal();

        pWindow->swapBuffers();
    }
}

// OpenGL helpers. You don't need to touch these.
void displayInternal(void)
{
    // Clear screen
    glViewport(0, 0, pWindow->getWindowSize().x, pWindow->getWindowSize().y);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Load identity matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Load up view transforms
    const glm::mat4 viewTransform = pTrackball->viewMatrix();
    glMultMatrixf(glm::value_ptr(viewTransform));

    // Your rendering function
    display();
}
void reshape(const glm::ivec2& size)
{
    // Called when the window is resized.
    // Update the viewport and projection matrix.
    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const glm::mat4 perspectiveMatrix = pTrackball->projectionMatrix();
    glLoadMatrixf(glm::value_ptr(perspectiveMatrix));
    glMatrixMode(GL_MODELVIEW);
}
