#pragma once

#include <cmath>
#include <limits>

#include "core/serialization.h"

struct vec2;
struct vec3;
struct vec4;
struct quat;

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
  float x = 0, y = 0;

  static const vec2& zero();
};

struct vec3 {
  float x = 0, y = 0, z = 0;

  explicit operator vec2() const;

  static vec3 normalized(const vec3&);
  static void normalize(vec3&);
  static float length(const vec3&);
  static float sqr_length(const vec3&);

  static const vec3& up();
  static const vec3& forward();
  static const vec3& right();
  static const vec3& zero();
  static const vec3& one();
};

struct vec4 {
  float x = 0, y = 0, z = 0, w = 0;

  explicit operator vec3() const;
};

bool operator==(vec2 lhs, vec2 rhs);
bool operator!=(vec2 lhs, vec2 rhs);

struct mat4 {
  float data[4][4] = {
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 1.0f }
  };

  [[nodiscard]] float determinant() const;
  vec3 normalize_scale();

  mat4& inverse();

  [[nodiscard]] vec3 origin() const;
  void set_origin(const vec3&);

  [[nodiscard]] vec4 row(uint32_t) const;
  void set_row(uint32_t, const vec3&);
  void set_row(uint32_t, const vec4&);

  [[nodiscard]] vec4 column(uint32_t) const;
  void set_column(uint32_t, const vec3&);
  void set_column(uint32_t, const vec4&);

  static mat4 inverse(const mat4&);
  static mat4 translation(const vec3& trans);
  static mat4 scale(float scale);
  static mat4 scale(const vec3& scale);
  static mat4 ortho(float x, float width, float y, float height, float minZ, float maxZ);
  static mat4 ortho(float width, float height, float minZ, float maxZ);
  static mat4 perspective(float fov, float ratio, float minZ, float maxZ);
  static mat4 look_at(const vec3& position, const vec3& target);
  static mat4 look_at(const vec3& position, const vec3& target, const vec3& up);
  static mat4 from_quat(const quat&);
  static mat4 trs(const vec3&, const quat&, const vec3&);
  static const mat4& identity();
};

struct quat {
  float x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f;

  quat& normalize();

  quat& operator*=(const quat& rhs);

  static const quat& identity();
  static quat axis(const vec3&, float rad);
  static quat angle(const vec3 &from, const vec3 &to);
  static quat inverse(const quat&);
  static quat from_matrix(const mat4&);
  static quat look_at(const vec3& direction, const vec3& up);
  static quat basis(const vec3 &right, const vec3 &up, const vec3 &forward);
};

quat operator*(const quat& lhs, const quat& rhs);
vec3 operator*(const vec3& lhs, const vec3& rhs);
vec3 operator*(float lhs, const vec3& rhs);

vec3 operator+(const vec3& lhs, const vec3& rhs);

vec3 operator-(const vec3& lhs, const vec3& rhs);
vec3 operator-(const vec3& rhs);
vec4 operator-(const vec4& rhs);

vec3 operator/(const vec3& lhs, float rhs);
vec3 operator/(float rhs, const vec3& lhs);

vec3 operator^(const vec3& lhs, const vec3& rhs);
float operator|(const vec3& lhs, const vec3& rhs);

vec3 operator*(const quat& lhs, const vec3& rhs);

class transform {
 public:
  transform() noexcept = default;

  transform(const transform& other) = default;
  transform& operator=(const transform& other) = default;

  [[nodiscard]] vec3 transform_coord(const vec3& value) const;
  [[nodiscard]] vec3 transform_vector(const vec3& value) const;
  [[nodiscard]] vec3 transform_direction(const vec3& value) const;

  [[nodiscard]] vec3 up() const;
  [[nodiscard]] vec3 forward() const;
  [[nodiscard]] vec3 right() const;

  void set_from_matrix(const mat4& matrix);

  explicit operator mat4() const;

  static const transform& identity();

  static transform inverse(const transform& transform);
  static transform from_matrix(const mat4& matrix);

 public:
  vec3 position = vec3::zero();
  quat rotation = quat::identity();
  vec3 scale = vec3::one();
};

transform operator*(const transform& lhs, const transform& rhs);

namespace math {

constexpr float PI = M_PI;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 1.0f / DEG_TO_RAD;

template <class T>
bool is_nan(const T& value) { return isnan(value); }

float length(const vec3& vec);

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
inline approximately(T x, T y, int ulp = 1) {
  T diff = x - y;
  return std::fabs(diff) <= std::numeric_limits<T>::epsilon() * std::fabs(x+y) * ulp
      // unless the result is subnormal
      || std::fabs(diff) < std::numeric_limits<T>::min();
}

}

template<>
struct serializer<vec4> {
  static void from_asset(assets::provider*, const asset&, vec4&);
  static void to_asset(asset&, const vec4&);
};

template<>
struct serializer<vec3> {
  static void from_asset(assets::provider*, const asset&, vec3&);
  static void to_asset(asset&, const vec3&);
};

template<>
struct serializer<quat> {
  static void from_asset(assets::provider*, const asset&, quat&);
  static void to_asset(asset&, const quat&);
};

template<>
struct serializer<transform> {
  static void from_asset(assets::provider*, const asset&, transform&);
  static void to_asset(asset&, const transform&);
};
