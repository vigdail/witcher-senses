#pragma once

#include <gl/program.hpp>

#include <unordered_map>

class Shader {
public:
  explicit Shader(gl::program &&program);

  template <class T> void setUniform(const char *name, const T &value) {
    const auto location = getLocation(name);
    if (location < 0) {
      std::cerr << "Bad uniform location: " << name << '\n';
    }
    program.set_uniform(location, value);
  }

  void use() const;
  int getLocation(const char *name);

private:
  gl::program program;
  std::unordered_map<std::string, int> uniforms{};
};
