#pragma once
// Disable warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <framework/opengl_includes.h>
#include <glm/mat3x3.hpp>
DISABLE_WARNINGS_POP()
#include <filesystem>

class Sprite {
public:
    Sprite(const std::filesystem::path& fileName);
    Sprite(const Sprite&) = delete;
    Sprite(Sprite&& other) noexcept;
    ~Sprite();

    Sprite& operator=(const Sprite&) = delete;
    Sprite& operator=(Sprite&&) noexcept;

    void draw(const glm::mat3& matrix, float depth) const;
    void draw(const glm::ivec2& position, float depth) const;
    void draw(int x, int y, float depth) const;

private:
    GLuint m_texture = (GLuint)-1;
};
