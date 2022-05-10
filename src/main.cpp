#include "mesh.h"

#include <gl/all.hpp>
#include <gl/auxiliary/glm_uniforms.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

void debugMessageCallback(gl::debug_log log) {
  std::cerr << log.message << std::endl;
}

int main() {
  glfwSetErrorCallback([](int error, const char* description) {
    std::cerr << "Error: " << description << '\n';
  });

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Witcher Senses", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  gl::initialize();

#ifndef NDEBUG
  gl::set_debug_output_enabled(true);
  gl::set_synchronous_debug_output_enabled(true);
  gl::set_debug_log_callback(debugMessageCallback);
  gl::set_debug_log_filters(GL_DONT_CARE, GL_DONT_CARE, {},
                            GL_DEBUG_SEVERITY_NOTIFICATION, false);
#endif
  gl::shader vert_shader(GL_VERTEX_SHADER);
  vert_shader.load_source("../assets/object.vert");

  gl::shader frag_shader(GL_FRAGMENT_SHADER);
  frag_shader.load_source("../assets/object.frag");

  gl::program program;
  program.attach_shader(vert_shader);
  program.attach_shader(frag_shader);
  if (!program.link()) {
    std::cout << program.info_log() << '\n';
  }
  program.use();

  Mesh mesh;
  mesh.load("../assets/sphere.gltf");
  mesh.bind();

  const glm::vec3 camera_origin(3.0f, 3.0f, -10.0f);
  glm::mat4 view = glm::lookAt(camera_origin, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
  auto view_location = program.uniform_location("view");
  program.set_uniform(view_location, view);

  glm::mat4 proj = glm::perspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 1000.0f);
  auto proj_location = program.uniform_location("proj");
  program.set_uniform(proj_location, proj);

  while (!glfwWindowShouldClose(window)) {
    gl::set_clear_color({0.3, 0.3, 0.3, 1.0});
    gl::clear();

    gl::draw_elements(GL_TRIANGLES, mesh.getIndexCount(), GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
