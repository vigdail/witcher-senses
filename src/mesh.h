#pragma once

#include <glm/glm.hpp>
#include <gl/all.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
  glm::vec2 uv;
};

class Mesh {
public:
  void load(const char *path);
  void bind();
  uint32_t getIndexCount() const;

private:
  gl::vertex_array vao_{};
  gl::buffer vertex_buffer_{};
  gl::buffer index_buffer_{};
  uint32_t index_count_{0};
};

