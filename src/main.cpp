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

constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;

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

struct Shader {
  gl::program program;
  std::unordered_map<std::string, int> uniforms{};

  explicit Shader(gl::program &&program) : program{std::move(program)} {}

  void use() const { program.use(); }

  template <class T> void setUniform(const char *name, const T &value) {
    const auto location = getLocation(name);
    if (location < 0) {
      std::cerr << "Bad uniform location: " << name << '\n';
    }
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

struct Move {};
struct Trace {};
struct Interesting {};

void spawnScene(World &world);
void moveSphereSystem(World &world) {
  world.view<Transform, Move>().each([&](Transform &transform) {
    transform.translation.x = 5.0f * float(sin(glfwGetTime() / 1.14));
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

struct Intensity {
  gl::framebuffer framebuffer;
  gl::texture_2d color;
  Shader shader;
};

void createIntensityFramebuffer(World &world, int width, int height) {
  gl::texture_2d color;
  color.set_min_filter(GL_LINEAR);
  color.set_mag_filter(GL_LINEAR);
  color.set_wrap_s(GL_REPEAT);
  color.set_wrap_t(GL_REPEAT);
  color.set_storage(1, GL_R11F_G11F_B10F, width, height);

  gl::framebuffer framebuffer;
  framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, color, 0);
  framebuffer.set_draw_buffer(GL_COLOR_ATTACHMENT0);

  gl::shader vertex(GL_VERTEX_SHADER);
  vertex.load_source("../assets/intensity.vert");
  gl::shader fragment(GL_FRAGMENT_SHADER);
  fragment.load_source("../assets/intensity.frag");

  gl::program program;
  program.attach_shader(vertex);
  program.attach_shader(fragment);
  if (!program.link()) {
    std::cerr << "Not linked: " << program.info_log() << '\n';
    exit(EXIT_FAILURE);
  }

  world.ctx().emplace<Intensity>(std::move(framebuffer), std::move(color),
                                 Shader(std::move(program)));
}

class PingPong {
public:
  PingPong(std::array<gl::texture_2d, 2> &&textures)
      : textures_{std::move(textures)} {}

  void swap() { current_index_ = 1 - current_index_; }

  const gl::texture_2d &current() const { return textures_[current_index_]; }

  const gl::texture_2d &next() const { return textures_[1 - current_index_]; }

private:
  std::array<gl::texture_2d, 2> textures_;
  uint32_t current_index_ = 0;
};

struct Outline {
  gl::framebuffer framebuffer;
  PingPong textures;
  Shader shader;
};

void createOutline(World &world) {
  gl::texture_2d color_1;
  color_1.set_min_filter(GL_LINEAR);
  color_1.set_mag_filter(GL_LINEAR);
  color_1.set_wrap_s(GL_REPEAT);
  color_1.set_wrap_t(GL_REPEAT);
  color_1.set_storage(1, GL_RG16F, 512, 512);

  gl::texture_2d color_2;
  color_2.set_min_filter(GL_LINEAR);
  color_2.set_mag_filter(GL_LINEAR);
  color_2.set_wrap_s(GL_REPEAT);
  color_2.set_wrap_t(GL_REPEAT);
  color_2.set_storage(1, GL_RG16F, 512, 512);

  gl::framebuffer framebuffer;
  framebuffer.attach_texture(GL_COLOR_ATTACHMENT0, color_1, 0);
  framebuffer.set_draw_buffer(GL_COLOR_ATTACHMENT0);

  gl::shader vertex(GL_VERTEX_SHADER);
  vertex.load_source("../assets/outline.vert");
  gl::shader fragment(GL_FRAGMENT_SHADER);
  fragment.load_source("../assets/outline.frag");

  gl::program program;
  program.attach_shader(vertex);
  program.attach_shader(fragment);
  if (!program.link()) {
    std::cerr << "Not linked: " << program.info_log() << '\n';
    exit(EXIT_FAILURE);
  }

  Shader shader(std::move(program));
  shader.setUniform("intensity_map", 0);
  shader.setUniform("outline_map", 1);

  world.ctx().emplace<Outline>(
      std::move(framebuffer),
      PingPong({std::move(color_1), std::move(color_2)}), std::move(shader));
}

struct Input {
  float horizontal{};
  float vertical{};
  glm::vec2 mouse_pos{};
  glm::vec2 mouse_delta{};
  bool left_mouse{};
  bool right_mouse{};
};

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
  auto *world = static_cast<World *>(glfwGetWindowUserPointer(window));
  auto &input = world->ctx().at<Input>();
  auto &last_pos = input.mouse_pos;
  glm::vec2 new_pos((float)xpos, (float)ypos);
  input.mouse_delta = new_pos - last_pos;
  input.mouse_pos = new_pos;
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mode) {
  auto *world = static_cast<World *>(glfwGetWindowUserPointer(window));
  auto &input = world->ctx().at<Input>();
  if (action == GLFW_PRESS) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      input.left_mouse = true;
    } else if (button == GLFW_MOUSE_BUTTON_2) {
      input.right_mouse = true;
    }
  } else if (action == GLFW_RELEASE) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      input.left_mouse = false;
    } else if (button == GLFW_MOUSE_BUTTON_2) {
      input.right_mouse = false;
    }
  }
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action,
                      int mods) {
  auto *world = static_cast<World *>(glfwGetWindowUserPointer(window));
  auto &input = world->ctx().at<Input>();
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_W:
      input.vertical += 1.0f;
      break;
    case GLFW_KEY_S:
      input.vertical -= 1.0f;
      break;
    case GLFW_KEY_A:
      input.horizontal -= 1.0f;
      break;
    case GLFW_KEY_D:
      input.horizontal += 1.0f;
      break;
    default:
      break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
    case GLFW_KEY_W:
      input.vertical -= 1.0f;
      break;
    case GLFW_KEY_S:
      input.vertical += 1.0f;
      break;
    case GLFW_KEY_A:
      input.horizontal += 1.0f;
      break;
    case GLFW_KEY_D:
      input.horizontal -= 1.0f;
      break;
    default:
      break;
    }
  }
}

void resetMouseDelta(World &world) {
  auto &input = world.ctx().at<Input>();
  input.mouse_delta = glm::vec2(0.0f);
}

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
  world.ctx().emplace<Input>();

  glfwSetWindowUserPointer(window, &world);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetKeyCallback(window, keyboardCallback);

  auto create_shader = [&](const char *vert_path, const char *frag_path) {
    gl::shader vert_shader(GL_VERTEX_SHADER);
    vert_shader.load_source(vert_path);

    gl::shader frag_shader(GL_FRAGMENT_SHADER);
    frag_shader.load_source(frag_path);

    gl::program program;
    program.attach_shader(vert_shader);
    program.attach_shader(frag_shader);
    if (!program.link()) {
      std::cout << program.info_log() << '\n';
    }
    return Shader(std::move(program));
  };
  Shader object_shader =
      create_shader("../assets/object.vert", "../assets/object.frag");
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
  createIntensityFramebuffer(world, WINDOW_WIDTH, WINDOW_HEIGHT);
  createOutline(world);

  glm::vec2 texture_size((float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
  Shader compose_shader =
      create_shader("../assets/compose.vert", "../assets/compose.frag");
  compose_shader.use();
  compose_shader.setUniform("color_map", 0);
  compose_shader.setUniform("outline_map", 1);
  compose_shader.setUniform("intensity_map", 2);
  compose_shader.setUniform("texture_size", texture_size);

  gl::vertex_array quad_vao;

  while (!glfwWindowShouldClose(window)) {
    moveSphereSystem(world);

    const auto &offscreen = world.ctx().at<const Offscreen>();
    offscreen.framebuffer.bind();
    gl::set_viewport({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT});
    gl::set_clear_color({0.3, 0.3, 0.3, 1.0});
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
              GL_STENCIL_BUFFER_BIT);

    gl::set_depth_test_enabled(true);
    gl::set_stencil_test_enabled(true);

    gl::set_stencil_mask(0xff);
    gl::set_stencil_operation(GL_KEEP, GL_KEEP, GL_REPLACE);
    object_shader.use();

    gl::set_stencil_function(GL_ALWAYS, 0x00, 0xff);
    auto traces = world.view<Trace>();
    auto interesting = world.view<Interesting>();
    world.view<Transform, MeshHandle, Color>().each(
        [&](auto entity, const auto &transform, const auto &mesh,
            const auto &color) {
          object_shader.setUniform("color", color.color);
          object_shader.setUniform("model", transform.transform());
          mesh->bind();
          if (traces.contains(entity)) {
            gl::set_stencil_function(GL_ALWAYS, 0x8, 0xff);
            gl::set_stencil_mask(0xff);
          } else if (interesting.contains(entity)) {
            gl::set_stencil_function(GL_ALWAYS, 0x4, 0xff);
            gl::set_stencil_mask(0xff);
          } else {
            gl::set_stencil_mask(0x00);
          }
          gl::draw_elements(GL_TRIANGLES, mesh->getIndexCount(),
                            GL_UNSIGNED_INT, nullptr);
        });

    gl::set_depth_test_enabled(false);
    gl::set_stencil_operation(GL_KEEP, GL_KEEP, GL_KEEP);
    auto &intensity = world.ctx().at<Intensity>();
    intensity.framebuffer.bind();
    intensity.framebuffer.attach_texture(GL_DEPTH_STENCIL_ATTACHMENT,
                                         offscreen.depth_stencil, 0);

    gl::set_clear_color({0.0, 0.0, 0.0, 1.0});
    gl::clear(GL_COLOR_BUFFER_BIT);
    intensity.shader.use();
    intensity.shader.setUniform("color", glm::vec3(1.0, 0.0, 0.0));
    gl::set_stencil_mask(0xFF);
    gl::set_stencil_function(GL_LESS, 0x00, 0x04);
    quad_vao.bind();
    gl::draw_arrays(GL_TRIANGLES, 0, 6);
    intensity.shader.setUniform("color", glm::vec3(0.0, 1.0, 0.0));
    gl::set_stencil_function(GL_LESS, 0x00, 0x08);
    gl::draw_arrays(GL_TRIANGLES, 0, 6);

    gl::set_stencil_test_enabled(false);
    auto &outline = world.ctx().at<Outline>();
    outline.framebuffer.bind();
    outline.framebuffer.attach_texture(GL_COLOR_ATTACHMENT0,
                                       outline.textures.current());
    outline.shader.use();
    outline.shader.setUniform("time", (float)glfwGetTime());
    gl::set_viewport({0, 0}, {512, 512});
    intensity.color.bind_unit(0);
    outline.textures.next().bind_unit(1);
    gl::clear(GL_COLOR_BUFFER_BIT);
    gl::draw_arrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl::set_viewport({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT});
    compose_shader.use();
    compose_shader.setUniform("time", (float)glfwGetTime());
    offscreen.color.bind_unit(0);
    outline.textures.current().bind_unit(1);
    intensity.color.bind_unit(2);
    gl::clear(GL_COLOR_BUFFER_BIT);
    gl::draw_arrays(GL_TRIANGLES, 0, 6);

    outline.textures.swap();

    glfwSwapBuffers(window);
    glfwPollEvents();

    resetMouseDelta(world);
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
  world.emplace<Trace>(sphere);

  auto sphere_2 = world.create();
  world.emplace<MeshHandle>(sphere_2, sphere_mesh);
  world.emplace<Transform>(sphere_2, Transform({-1.0f, 1.0f, 2.0}));
  world.emplace<Color>(sphere_2, Color{glm::vec3(0.2f, 0.2f, 0.4f)});

  auto cube_mesh = std::make_shared<Mesh>();
  cube_mesh->load("../assets/cube.gltf");
  auto cube = world.create();
  world.emplace<MeshHandle>(cube, cube_mesh);
  world.emplace<Transform>(cube, Transform({-2.0f, 1.0f, -2.0}));
  world.emplace<Color>(cube, Color{glm::vec3(1.0f, 0.6f, 0.5f)});
  world.emplace<Interesting>(cube);

  auto plane_mesh = std::make_shared<Mesh>();
  plane_mesh->load("../assets/plane.gltf");
  auto plane = world.create();
  Transform plane_transform{};
  plane_transform.scale = glm::vec3(2.0f, 1.0f, 2.0f);
  world.emplace<MeshHandle>(plane, plane_mesh);
  world.emplace<Transform>(plane, plane_transform);
  world.emplace<Color>(plane, Color{glm::vec3(0.3f, 0.6f, 0.7f)});
}
