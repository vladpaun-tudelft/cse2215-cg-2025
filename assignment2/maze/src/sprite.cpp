#include "sprite.h"
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/vector_relational.hpp>
#include <stb/stb_image.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <cassert>
#include <exception>
#include <filesystem>
#include <iostream>

static const std::filesystem::path dataDirPath { DATA_DIR };

GLuint loadTexture(const std::filesystem::path& fileName)
{
    if (!std::filesystem::exists(fileName)) {
        std::cerr << "Texture file does not exists: " << fileName << std::endl;
        throw std::exception();
    }

    int width, height, numChannels;
    const auto fileNameStr = fileName.string(); // Create l-value so c_str() is safe.
    stbi_uc* pixels = stbi_load(fileNameStr.c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

    if (!pixels) {
        std::cerr << "Failed to read texture " << fileName << " using stb_image.h" << std::endl;
        throw std::exception();
    }

    glEnable(GL_TEXTURE_2D);
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels); // Free CPU data when the texture has been copied to the GPU.
    return texID;
}

Sprite::Sprite(Sprite&& other) noexcept
{
    other.m_texture = (GLuint)-1;
}

Sprite::Sprite(const std::filesystem::path& fileName)
{
    const auto imageFilePath = dataDirPath / fileName;
    assert(std::filesystem::exists(imageFilePath));

    m_texture = loadTexture(imageFilePath);
}

Sprite::~Sprite()
{
    if (m_texture != (GLuint)-1)
        glDeleteTextures(1, &m_texture);
}

Sprite& Sprite::operator=(Sprite&& other) noexcept
{
    m_texture = other.m_texture;
    other.m_texture = (GLuint)-1;
    return *this;
}

void Sprite::draw(const glm::mat3& matrix, float depth) const
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    const std::array vertices {
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
    };
    glBegin(GL_QUADS);
    for (const auto& vertex : vertices) {
        const glm::vec2 transformedVertex = matrix * glm::vec3(vertex, 1.0f);
        const glm::vec3 vertex3D = glm::vec3(transformedVertex, depth);
        glTexCoord2d(vertex.x, 1 - vertex.y);
        glVertex3fv(glm::value_ptr(vertex3D));
    }
    glEnd();

    glPopAttrib();
}

void Sprite::draw(const glm::ivec2& position, float depth) const
{
    return draw(position.x, position.y, depth);
}

void Sprite::draw(int x, int y, float depth) const
{
    auto transform = glm::identity<glm::mat3>();
    transform = glm::translate(transform, glm::vec2(x, y));
    draw(transform, depth);
}
