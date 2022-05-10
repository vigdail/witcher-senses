//#include <glad/gl.h>
#include <gl/all.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

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
  vert_shader.load_source("../assets/triangle.vert");

  gl::shader frag_shader(GL_FRAGMENT_SHADER);
  frag_shader.load_source("../assets/triangle.frag");

  gl::program program;
  program.attach_shader(vert_shader);
  program.attach_shader(frag_shader);
  if (!program.link()) {
    std::cout << program.info_log() << '\n';
  }
  program.use();

  gl::vertex_array vao;
  vao.bind();

  while (!glfwWindowShouldClose(window)) {
    gl::set_clear_color({0.3, 0.3, 0.3, 1.0});
    gl::clear();

    gl::draw_arrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
