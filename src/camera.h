#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
public:
  Camera(float aspect, float fov, float near, float far)
      : projection_{glm::perspective(fov, aspect, near, far)},
        view_{glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f))} {}

  void updateView(glm::mat4 transform) {
    glm::vec3 forward(0.0f, 0.0f, 1.0f);
    forward = transform * glm::vec4(forward, 1.0);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    up = transform * glm::vec4(up, 0.0);
    glm::vec3 origin = transform[3];
    view_ = glm::lookAt(origin, forward, up);
  }

  const glm::mat4 &getProjection() const { return projection_; }
  const glm::mat4 &getView() const { return view_; }

private:
  glm::mat4 projection_;
  glm::mat4 view_;
};
