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
  Mesh();
  void load(const char* path);
  void bind();

 private:
  gl::vertex_array vao_;
  gl::buffer vertex_buffer_;
 public:
  uint32_t getIndexCount() const;
 private:
  gl::buffer index_buffer_;
  uint32_t index_count_;
};

