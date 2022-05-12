#include "mesh.h"

#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <gl/all.hpp>
#include <gl/auxiliary/glm_uniforms.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <memory>
#include <optional>
#include <vector>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

void debugMessageCallback(const gl::debug_log &log) {
  std::cerr << log.message << std::endl;
}

struct Transform {
  glm::vec3 translation{};
  glm::vec3 rotation{};
  glm::vec3 scale{1.0f};

  Transform() = default;
  Transform(const Transform &) = default;

  explicit Transform(const glm::vec3 &translation) : translation{translation} {}

  glm::mat4 transform() const {
    glm::mat4 rotation_matrix = glm::toMat4(glm::quat(rotation));

    return glm::translate(glm::mat4(1.0f), translation) * rotation_matrix *
           glm::scale(glm::mat4(1.0f), scale);
  }
};

struct Color {
  glm::vec3 color;
};

struct DirectionalLight {
  glm::vec3 direction;
  float intensity;
  glm::vec3 color;
};

using MeshHandle = std::shared_ptr<Mesh>;
using World = entt::registry;

struct Move {
  bool placeholder{};
};

struct Stencil {
  int value;
};

void spawnScene(World &world);
void moveSphereSystem(World &world) {
  world.view<Transform, Move>().each([&](Transform &transform, const Move &m) {
    transform.translation.x = 5.0f * float(sin(glfwGetTime()));
  });
}

struct Offscreen {
  gl::framebuffer framebuffer;
  gl::texture_2d color;
  gl::texture_2d depth_stencil;
};

void createOffscreenFramebuffer(World &world, int width, int height) {
  gl::texture_2d color;
  color.set_min_filter(GL_LINEAR);
  color.set_mag_filter(GL_LINEAR);
  color.set_wrap_s(GL_REPEAT);
  color.set_wrap_t(GL_REPEAT);
  color.set_storage(1, GL_RGBA8, width, height);

  gl::texture_2d depth_stencil;
  depth_stencil.set_min_filter(GL_LINEAR);
  depth_stencil.set_mag_filter(GL_LINEAR);
  depth_stencil.set_wrap_s(GL_REPEAT);
  depth_stencil.set_wrap_t(GL_REPEAT);
  depth_stencil.set_storage(1, GL_DEPTH24_STENCIL8, width, height);

  gl::framebuffer framebuffer;
  framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, color, 0);
  framebuffer.attach_texture(GL_DEPTH_STENCIL_ATTACHMENT, depth_stencil, 0);
  framebuffer.set_draw_buffer(GL_COLOR_ATTACHMENT0);

  world.ctx().emplace<Offscreen>(std::move(framebuffer), std::move(color),
                                 std::move(depth_stencil));
}

struct Shader {
  gl::program program;
  std::unordered_map<std::string, int> uniforms{};

  explicit Shader(gl::program &&program) : program{std::move(program)} {}

  void use() const { program.use(); }

  template <class T> void setUniform(const char *name, const T &value) {
    const auto location = getLocation(name);
    program.set_uniform(location, value);
  }

  int getLocation(const char *name) {
    if (auto it = uniforms.find(name); it != uniforms.end()) {
      return it->second;
    }
    const auto location = program.uniform_location(name);
    uniforms[name] = location;
    return location;
  }
};

int main() {
  glfwSetErrorCallback([](int error, const char *description) {
    std::cerr << "Error: " << description << '\n';
  });

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        "Witcher Senses", nullptr, nullptr);
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

  auto create_object_shader = [&]() {
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
    return Shader(std::move(program));
  };
  Shader object_shader = create_object_shader();
  object_shader.use();

  spawnScene(world);

  const glm::vec3 camera_origin(3.0f, 3.0f, -10.0f);
  glm::mat4 view =
      glm::lookAt(camera_origin, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0));
  object_shader.setUniform("view", view);

  glm::mat4 proj = glm::perspective(45.0f, (float)WINDOW_WIDTH / WINDOW_HEIGHT,
                                    0.01f, 1000.0f);
  object_shader.setUniform("proj", proj);

  DirectionalLight directional_light{
      .direction = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)),
      .intensity = 1.0f,
      .color = glm::vec3(1.0f, 1.0f, 1.0f),
  };

  gl::buffer light_ubo;
  light_ubo.set_data(sizeof(DirectionalLight), &directional_light);
  light_ubo.bind_base(GL_UNIFORM_BUFFER, 0);

  createOffscreenFramebuffer(world, WINDOW_WIDTH, WINDOW_HEIGHT);

  while (!glfwWindowShouldClose(window)) {
    const auto &offscreen = world.ctx().at<const Offscreen>();
    offscreen.framebuffer.bind();
    gl::set_clear_color({0.3, 0.3, 0.3, 1.0});
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
              GL_STENCIL_BUFFER_BIT);

    gl::set_depth_test_enabled(true);
    gl::set_stencil_test_enabled(true);

    gl::set_stencil_mask(0xff);
    gl::set_stencil_operation(GL_KEEP, GL_KEEP, GL_REPLACE);

    world.view<Transform, MeshHandle, Color>().each(
        [&](auto entity, const auto &transform, const auto &mesh,
            const auto &color) {
          object_shader.setUniform("color", color.color);
          object_shader.setUniform("model", transform.transform());
          mesh->bind();
          if (auto *s = world.try_get<Stencil>(entity); s) {
            gl::set_stencil_function(GL_ALWAYS, s->value, 0xff);
            gl::set_stencil_mask(0xff);
          } else {
            gl::set_stencil_mask(0x00);
          }
          gl::draw_elements(GL_TRIANGLES, mesh->getIndexCount(),
                            GL_UNSIGNED_INT, nullptr);
        });
    moveSphereSystem(world);

    offscreen.framebuffer.bind(GL_READ_FRAMEBUFFER);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitNamedFramebuffer(offscreen.framebuffer.id(), 0, 0, 0, WINDOW_WIDTH,
                           WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void spawnScene(World &world) {
  auto sphere_mesh = std::make_shared<Mesh>();
  sphere_mesh->load("../assets/sphere.gltf");
  auto sphere = world.create();
  world.emplace<MeshHandle>(sphere, sphere_mesh);
  world.emplace<Transform>(sphere, Transform({0.0f, 1.0f, 0.0}));
  world.emplace<Color>(sphere, Color{glm::vec3(0.5f, 0.6f, 0.3f)});
  world.emplace<Move>(sphere, Move{});
  world.emplace<Stencil>(sphere, Stencil{4});

  auto cube_mesh = std::make_shared<Mesh>();
  cube_mesh->load("../assets/cube.gltf");
  auto cube = world.create();
  world.emplace<MeshHandle>(cube, cube_mesh);
  world.emplace<Transform>(cube, Transform({-2.0f, 1.0f, -2.0}));
  world.emplace<Color>(cube, Color{glm::vec3(1.0f, 0.6f, 0.5f)});
  world.emplace<Stencil>(cube, Stencil{8});

  auto plane_mesh = std::make_shared<Mesh>();
  plane_mesh->load("../assets/plane.gltf");
  auto plane = world.create();
  world.emplace<MeshHandle>(plane, plane_mesh);
  world.emplace<Transform>(plane, Transform{});
  world.emplace<Color>(plane, Color{glm::vec3(0.3f, 0.6f, 0.7f)});
}
