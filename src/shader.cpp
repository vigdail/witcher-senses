#include "shader.h"

Shader::Shader(gl::program &&program) : program{std::move(program)} {}

void Shader::use() const { program.use(); }

int Shader::getLocation(const char *name) {
  if (auto it = uniforms.find(name); it != uniforms.end()) {
    return it->second;
  }
  const auto location = program.uniform_location(name);
  uniforms[name] = location;
  return location;
}