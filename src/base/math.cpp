#include <assert.h>
#include "math.h"

const mat4& mat4::identity() {
  static mat4 mat;
  return mat;
}

float mat4::determinant() const {
  return data[0][0] * (
      data[1][1] * (data[2][2] * data[3][3] - data[2][3] * data[3][2]) -
          data[2][1] * (data[1][2] * data[3][3] - data[1][3] * data[3][2]) +
          data[3][1] * (data[1][2] * data[2][3] - data[1][3] * data[2][2]))

      - data[1][0] * (
          data[0][1] * (data[2][2] * data[3][3] - data[2][3] * data[3][2]) -
              data[2][1] * (data[0][2] * data[3][3] - data[0][3] * data[3][2]) +
              data[3][1] * (data[0][2] * data[2][3] - data[0][3] * data[2][2]))

      + data[2][0] * (
          data[0][1] * (data[1][2] * data[3][3] - data[1][3] * data[3][2]) -
              data[1][1] * (data[0][2] * data[3][3] - data[0][3] * data[3][2]) +
              data[3][1] * (data[0][2] * data[1][3] - data[0][3] * data[1][2]))

      - data[3][0] * (
          data[0][1] * (data[1][2] * data[2][3] - data[1][3] * data[2][2]) -
              data[1][1] * (data[0][2] * data[2][3] - data[0][3] * data[2][2]) +
              data[2][1] * (data[0][2] * data[1][3] - data[0][3] * data[1][2]));
}

vec3 mat4::origin() const {
  return {data[3][0], data[3][1], data[3][2]};
}

void mat4::set_origin(const vec3& vec) {
  data[3][0] = vec.x; data[3][1] = vec.y; data[3][2] = vec.z;
}

vec3 mat4::normalize_scale() {
  vec3 scale = {
      math::length((vec3) column(0)),
      math::length((vec3) column(1)),
      math::length((vec3) column(2))
  };

  if (!math::approximately(scale.x, 0.0f)) {
    float inv = 1 / scale.x;
    data[0][0] *= inv, data[0][1] *= inv, data[0][2] *= inv;
  }
  if (!math::approximately(scale.y, 0.0f)) {
    float inv = 1 / scale.y;
    data[1][0] *= inv, data[1][1] *= inv, data[1][2] *= inv;
  }
  if (!math::approximately(scale.z, 0.0f)) {
    float inv = 1 / scale.z;
    data[2][0] *= inv, data[2][1] *= inv, data[2][2] *= inv;
  }

  return scale;
}

vec4 mat4::row(uint32_t index) const {
  return {data[index][0], data[index][1], data[index][2], data[index][3] };
}

void mat4::set_row(uint32_t index, const vec3& vec) {
  data[index][0] = vec.x, data[index][1] = vec.y, data[index][2] = vec.z;
}

void mat4::set_row(uint32_t index, const vec4& vec) {
  data[index][0] = vec.x, data[index][1] = vec.y, data[index][2] = vec.z, data[index][3] = vec.w;
}

vec4 mat4::column(uint32_t index) const {
  return {data[0][index], data[1][index], data[2][index], data[3][index] };
}

void mat4::set_column(uint32_t index, const vec3& vec) {
  data[0][index] = vec.x, data[1][index] = vec.y, data[2][index] = vec.z;
}

void mat4::set_column(uint32_t index, const vec4& vec) {
  data[0][index] = vec.x, data[1][index] = vec.y, data[2][index] = vec.z, data[3][index] = vec.w;
}

mat4 mat4::trs(const vec3& origin, const quat& rot, const vec3& scale) {
  mat4 mat = mat4::from_quat(rot);
  mat.data[0][1] *= scale.x; mat.data[0][2] *= scale.x;
  mat.data[1][0] *= scale.y; mat.data[1][2] *= scale.y;
  mat.data[2][0] *= scale.z; mat.data[2][1] *= scale.z;
  mat.set_row(3, origin);
  return mat;
}

mat4 mat4::from_quat(const quat& q) {
  const float x2 = q.x + q.x;  const float y2 = q.y + q.y;  const float z2 = q.z + q.z;
  const float xx = q.x * x2; const float xy = q.x * y2; const float xz = q.x * z2;
  const float yy = q.y * y2; const float yz = q.y * z2; const float zz = q.z * z2;
  const float wx = q.w * x2; const float wy = q.w * y2; const float wz = q.w * z2;

  return {{
       { 1.0f - (yy + zz), xy + wz, xz - wy, 0.0f },
       { xy - wz, 1.0f - (xx + zz), yz + wx, 0.0f },
       { xz + wy, yz - wx, 1.0f - (xx + yy), 0.0f },
       { 0.0f, 0.0f, 0.0f, 1.0f }
  }};
}
mat4 &mat4::inverse() {
  *this = mat4::inverse(*this);
  return *this;
}

mat4 mat4::inverse(const mat4& mat) {
  vec3 x = (vec3) mat.row(0);
  vec3 y = (vec3) mat.row(1);
  vec3 z = (vec3) mat.row(2);
  vec3 w = (vec3) -mat.row(3);

  return {{
      { mat.data[0][0], mat.data[1][0], mat.data[2][0], 0.0f },
      { mat.data[0][1], mat.data[1][1], mat.data[2][1], 0.0f },
      { mat.data[0][2], mat.data[1][2], mat.data[2][2], 0.0f },
      { w | x,          w | y,          w | z,          1.0f }
  }};
}
mat4 mat4::translation(const vec3 &trans) {
  return {{
              {1.0f, 0.0f, 0.0f, 0.0f},
              {0.0f, 1.0f, 0.0f, 0.0f},
              {0.0f, 0.0f, 1.0f, 0.0f},
              {trans.x, trans.y, trans.z, 1.0f}
      }};
}
mat4 mat4::scale(float scale) {
  return {{
              {scale, 0.0f, 0.0f, 0.0f},
              {0.0f, scale, 0.0f, 0.0f},
              {0.0f, 0.0f, scale, 0.0f},
              {0.0f, 0.0f, 0.0f, 1.0f},
  }};
}
mat4 mat4::scale(const vec3 &scale) {
  return {{
              {scale.x, 0.0f, 0.0f, 0.0f},
              {0.0f, scale.y, 0.0f, 0.0f},
              {0.0f, 0.0f, scale.z, 0.0f},
              {0.0f, 0.0f, 0.0f, 1.0f},
          }};
}
mat4 mat4::ortho(float x, float width, float y, float height, float minZ, float maxZ) {
  float scale = 1.0f / (maxZ - minZ);
  return {{
      {2.0f / width, 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / height, 0.0f, 0.0f},
      {0.0f, 0.0f, scale, 0.0f},
      {-(2 * x + width) / width, -(2 * y + height) / height, -(minZ + maxZ) * scale, 1.0f}
  }};
}
mat4 mat4::ortho(float width, float height, float minZ, float maxZ) {
  float scale = 1.0f / (maxZ - minZ);
  return {{
      {2.0f / width, 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / height, 0.0f, 0.0f},
      {0.0f, 0.0f, scale, 0.0f},
      {0.0f, 0.0f, -(minZ + maxZ) * scale, 1.0f}
  }};
}
mat4 mat4::perspective(float fov, float ratio, float minZ, float maxZ) {
  assert(math::approximately(0.0f, 0.0f));
  const float cot = 1.0f / std::tan(fov * 0.5f);
  const float scale = 1.0f / (maxZ - minZ);
  return {{
      {cot / ratio, 0.0f, 0.0f, 0.0f},
      {0.0f, cot, 0.0f, 0.0f},
      {0.0f, 0.0f, maxZ * scale, 1.0f},
      {0.0f, 0.0f, -minZ * maxZ * scale, 0.0f}
    }};
}

mat4 mat4::look_at(const vec3 &position, const vec3 &target) {
  vec3 fwd = vec3::normalized(target - position);
  vec3 right = vec3::up() ^ fwd;
  vec3 up = fwd ^ right;
  return {{
              { right.x, right.y, right.z, 0.0f },
              { up.x, up.y, up.z, 0.0f},
              { fwd.x, fwd.y, fwd.z, 0.0f },
              { position.x, position.y, position.z, 1.0f}
    }};
}
mat4 mat4::look_at(const vec3 &position, const vec3 &target, const vec3 &up) {
  vec3 fwd = vec3::normalized(target - position);
  vec3 right = vec3::normalized(up ^ fwd);
  vec3 up0 = fwd ^ right;
  return {{
              { right.x, right.y, right.z, 0.0f },
              { up0.x, up0.y, up0.z, 0.0f},
              { fwd.x, fwd.y, fwd.z, 0.0f },
              { position.x, position.y, position.z, 1.0f}
          }};
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

const vec3 &vec3::up() {
  static vec3 inst { 0.0f, 1.0f, 0.0f };
  return inst;
}

const vec3 &vec3::forward() {
  static vec3 inst { 0.0f, 0.0f, 1.0f };
  return inst;
}

const vec3 &vec3::right() {
  static vec3 inst { 1.0f, 0.0f, 0.0f };
  return inst;
}

const vec3 &vec3::zero() {
  static vec3 inst { 0.0f, 0.0f, 0.0f };
  return inst;
}

const vec3 &vec3::one() {
  static vec3 inst { 1.0f, 1.0f, 1.0f };
  return inst;
}

vec3::operator vec2() const { 
  return { x, y }; 
}

vec3 vec3::normalized(const vec3& vec) {
  return vec / std::sqrt(vec | vec);
}

vec4::operator vec3() const { 
  return { x, y, z }; 
}

vec3 transform::transform_coord(const vec3 &value) const {
  return position + (rotation * value) * scale;
}

vec3 transform::transform_vector(const vec3 &value) const {
  return (rotation * value) * scale;
}

vec3 transform::transform_direction(const vec3 &value) const {
  return rotation * value;
}

vec3 transform::up() const {
  return transform_direction(vec3::up());
}

vec3 transform::forward() const {
  return transform_direction(vec3::forward());
}

vec3 transform::right() const {
  return transform_direction(vec3::right());
}

void transform::set_from_matrix(const mat4 &matrix) {
  mat4 tmp = matrix;

  scale = tmp.normalize_scale();
  if (matrix.determinant() < 0.f) {
    scale.x *= -1.f;
    tmp.set_row(0, -tmp.row(0));
  }

  position = matrix.origin();
  rotation = quat::from_matrix(tmp).normalize();
}

transform::operator mat4() const {
  return mat4::trs(position, rotation, scale);
}

transform transform::inverse(const transform &transform) {
  quat rotation = quat::inverse(transform.rotation);
  vec3 scale = 1 / transform.scale;
  vec3 position = rotation * (scale * -transform.position);
  return { position, rotation, scale };
}

const transform &transform::identity() {
  static transform identity;
  return identity;
}

transform transform::from_matrix(const mat4 &matrix) {
  transform tr;
  tr.set_from_matrix(matrix);
  return tr;
}
quat &quat::normalize() {
  float l = 1.0f / std::sqrtf(x * x + y * y + z * z + w * w);
  x *= l; y *= l; z *= l; w *= l;
  return *this;
}

const quat &quat::identity() {
  static quat identity;
  return identity;
}

quat quat::inverse(const quat& q) {
  return { -q.x, -q.y, -q.z, q.w };
}

quat quat::from_matrix(const mat4& mat) {
  float s;
  const float tr = mat.data[0][0] + mat.data[1][1] + mat.data[2][2];

  quat q;
  if (tr > 0.0f) {
    float t = tr + 1.0f;
    s = 1.0f / std::sqrtf(t) * 0.5f;

    q.x = (mat.data[1][2] - mat.data[2][1]) * s;
    q.y = (mat.data[2][0] - mat.data[0][2]) * s;
    q.z = (mat.data[0][1] - mat.data[1][0]) * s;
    q.w = s * t;
  }
  else {
    int32_t i = 0;

    if (mat.data[1][1] > mat.data[0][0]) i = 1;
    if (mat.data[2][2] > mat.data[i][i]) i = 2;

    static const int32_t next[3] = { 1, 2, 0 };
    const int32_t j = next[i];
    const int32_t k = next[j];

    s = mat.data[i][i] - mat.data[j][j] - mat.data[k][k] + 1.0f;

    float sqrt_s = std::sqrtf(s);

    float qt[4];
    qt[i] = 0.5f * sqrt_s;

    s = 0.5f / sqrt_s;

    qt[3] = (mat.data[j][k] - mat.data[k][j]) * s;
    qt[j] = (mat.data[i][j] + mat.data[j][i]) * s;
    qt[k] = (mat.data[i][k] + mat.data[k][i]) * s;

    q.x = qt[0]; q.y = qt[1]; q.z = qt[2]; q.w = qt[3];
  }
  return q;
}

quat operator*(const quat& lhs, const quat& rhs) {
  return {
      lhs.w * rhs.x + rhs.w * lhs.x + lhs.y * rhs.z - rhs.y * lhs.z,
      lhs.w * rhs.y + rhs.w * lhs.y + lhs.z * rhs.x - rhs.z * lhs.x,
      lhs.w * rhs.z + rhs.w * lhs.z + lhs.x * rhs.y - rhs.x * lhs.y,
      lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
  };
}

vec3 operator*(const vec3& lhs, const vec3& rhs) {
  return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

vec3 operator+(const vec3& lhs, const vec3& rhs) {
  return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

vec3 operator-(const vec3& lhs, const vec3& rhs) {
  return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

vec3 operator-(const vec3& rhs) {
  return { -rhs.x, -rhs.y, -rhs.z };
}

vec4 operator-(const vec4& rhs) {
  return { -rhs.x, -rhs.y, -rhs.z, -rhs.w };
}

vec3 operator*(float lhs, const vec3& rhs) {
  return { lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
}

vec3 operator/(const vec3& lhs, float rhs) {
  const float scale = 1.0f / rhs;
  return scale * lhs;
}

vec3 operator/(float rhs, const vec3& lhs) {
  return { rhs / lhs.x, rhs / lhs.y, rhs / lhs.z };
}

vec3 operator^(const vec3& lhs, const vec3& rhs) {
  return {
      lhs.y * rhs.z - lhs.z * rhs.y,
      lhs.z * rhs.x - lhs.x * rhs.z,
      lhs.x * rhs.y - lhs.y * rhs.x
  };
}

float operator|(const vec3& lhs, const vec3& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

vec3 operator*(const quat& lhs, const vec3& rhs) {
  const vec3 q { lhs.x, lhs.y, lhs.z};
  const vec3 t = 2.f * (q ^ rhs);
  return rhs + (lhs.w * t) + (q ^ t);
}

transform operator*(const transform& lhs, const transform& rhs) {
  return { lhs.rotation * (rhs.position * lhs.scale) + lhs.position, lhs.rotation * rhs.rotation, lhs.scale * rhs.scale};
}

float math::length(const vec3 &vec) {
  return std::sqrtf(vec | vec);
}
