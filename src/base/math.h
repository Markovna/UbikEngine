#pragma once

struct vec2i {
  int x;
  int y;
};

struct vec3i {
  int x;
  int y;
  int z;
};

struct vec4i {
  int x;
  int y;
  int z;
  int w;
};

bool operator==(vec2i lhs, vec2i rhs);
bool operator!=(vec2i lhs, vec2i rhs);

bool operator==(vec3i lhs, vec3i rhs);
bool operator!=(vec3i lhs, vec3i rhs);

bool operator==(vec4i lhs, vec4i rhs);
bool operator!=(vec4i lhs, vec4i rhs);

struct vec2 {
  float x, y;
};

struct vec3 {
  float x, y, z;
};

struct vec4 {
  float x, y, z, w;
};

struct quat {
  float x, y, z, w;
};

struct mat4 {
  float data[4][4] = {
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 1.0f }
  };

  static const mat4& identity();
};

