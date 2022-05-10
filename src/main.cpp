#include "mesh.h"

#include <gl/all.hpp>
#include <gl/auxiliary/glm_uniforms.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <entt/entt.hpp>

#include <iostream>
#include <vector>
#include <memory>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

void debugMessageCallback(gl::debug_log log) {
  std::cerr << log.message << std::endl;
}

struct Transform {
  glm::vec3 translation{};
  glm::vec3 rotation{};
  glm::vec3 scale{1.0f};

  Transform() = default;
  Transform(const Transform&) = default;

  Transform(const glm::vec3& translation) : translation{translation} {}

  glm::mat4 transform() const {
    glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));

    return glm::translate(glm::mat4(1.0f), translation)
        * rotation_matrix
        * glm::scale(glm::mat4(1.0f), scale);
  }
};

struct Color {
  glm::vec3 color;
};

using MeshHandle = std::shared_ptr<Mesh>;
using World = entt::registry;

struct Move {
  bool placeholder{};
};

void moveSphereSystem(World& world) {
  world.view<Transform, Move>().each([&](Transform& transform, const Move& m) {
    transform.translation.x = 5.0 * sin(glfwGetTime());
  });
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

  entt::registry world;

  gl::set_depth_test_enabled(true);

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

  auto sphere_mesh = std::make_shared<Mesh>();
  sphere_mesh->load("../assets/sphere.gltf");
  auto sphere = world.create();
  world.emplace<MeshHandle>(sphere, sphere_mesh);
  world.emplace<Transform>(sphere, Transform({0.0f, 1.0f, 0.0}));
  world.emplace<Color>(sphere, Color{glm::vec3(1.0f, 0.0f, 0.0f)});
  world.emplace<Move>(sphere, Move{});

  auto plane_mesh = std::make_shared<Mesh>();
  plane_mesh->load("../assets/plane.gltf");
  auto plane = world.create();
  world.emplace<MeshHandle>(plane, plane_mesh);
  world.emplace<Transform>(plane, Transform{});
  world.emplace<Color>(plane, Color{glm::vec3(0.0f, 1.0f, 0.0f)});

  const glm::vec3 camera_origin(3.0f, 3.0f, -10.0f);
  glm::mat4 view = glm::lookAt(camera_origin, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
  auto view_location = program.uniform_location("view");
  program.set_uniform(view_location, view);

  glm::mat4 proj = glm::perspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.01f, 1000.0f);
  auto proj_location = program.uniform_location("proj");
  program.set_uniform(proj_location, proj);

  auto color_location = program.uniform_location("color");
  auto model_location = program.uniform_location("model");

  while (!glfwWindowShouldClose(window)) {
    gl::set_clear_color({0.3, 0.3, 0.3, 1.0});
    gl::clear();

    world.view<Transform, MeshHandle, Color>().each([&](const auto& transform, const auto& mesh, const auto& color) {
      program.set_uniform(color_location, color.color);
      program.set_uniform(model_location, transform.transform());
      mesh->bind();
      gl::draw_elements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    });
    moveSphereSystem(world);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
