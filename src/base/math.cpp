#include "math.h"

const mat4& mat4::identity() {
  static mat4 mat;
  return mat;
}

bool operator==(vec4i lhs, vec4i rhs) {
  return lhs.x == rhs.x
      && lhs.y == rhs.y
      && lhs.z == rhs.z
      && lhs.w == rhs.w;
}
bool operator==(vec3i lhs, vec3i rhs) {
  return lhs.x == rhs.x
      && lhs.y == rhs.y
      && lhs.z == rhs.z;
}
bool operator!=(vec3i lhs, vec3i rhs) {
  return !(lhs == rhs);
}
bool operator!=(vec4i lhs, vec4i rhs) {
  return !(lhs == rhs);
}
bool operator!=(vec2i lhs, vec2i rhs) {
  return !(lhs == rhs);
}
bool operator==(vec2i lhs, vec2i rhs) {
  return lhs.x == rhs.x
      && lhs.y == rhs.y;
}
