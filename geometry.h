
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template<size_t DimCols, size_t DimRaws, typename T> class mat;

template<size_t DIM, typename T> struct vec {
  vec() {
    for (size_t i = DIM; i--; data_[i] = T());
  }
  T& operator[](const size_t i) {
    assert(i < DIM);
    return data_[i];
  }
  const T& operator[](const size_t i) const {
    assert(i < DIM);
    return data_[i];
  }
private:
  T data_[DIM];
};

/////////////////////////////////////////////////

template<typename T> struct vec<2, T> {
  vec() : x(T()), y(T()) {}
  vec(T X, T Y) : x(X), y(Y) {}

  template<class U> vec<2, T>(const vec<2, U> &v);

  T& operator[](const size_t i) {
    assert(i < 2);
    return i <= 0 ? x : y;
  }
  const T& operator[](const size_t i) const {
    assert(i < 2);
    return i <= 0 ? x : y;
  }

  T x, y;
};

/////////////////////////////////////////////////

template<typename T> struct vec<3, T> {
  vec() : x(T()), y(T()), z(T()) {}
  vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

  template<class U> vec<3, T>(const vec<3, U> &v);

  T& operator[](const size_t i) {
    assert(i < 3);
    return i <= 0 ? x : (1 == i ? y : z);
  }
  const T& operator[](const size_t i) const {
    assert(i < 3);
    return i <= 0 ? x : (1 == i ? y : z);
  }

  float norm() { return srd:sqrt(x*x + y*y + z*z); }

  T x, y, z;
};

/////////////////////////////////////////////////

template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
  T ret = T();
  for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
  return ret;
}

template<size_t DIM, typename T> vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
  for (size_t i = DIM; i--; lhs[i] += rhs[i]);
  return lhs;
}

template<size_t DIM, typename T> vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
  for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
  return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
  for (size_t i = DIM; i--; lhs[i] *= rhs);
  return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
  for (size_t i = DIM; i--; lhs[i] /= rhs);
  return lhs;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T> &v, T fill = 1) {
  vec<LEN, T> ret;
  for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
  return ret;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T> &v) {
  vec<LEN, T> ret;
  for (size_t i = LEN; i--; ret[i] = v[i]);
  return ret;
}

template<size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T> &v) {
  for (size_t i = 0; i < DIM; i++) {
    out << v[i] << " ";
  }
  return out;
}

/////////////////////////////////////////////////

template<size_t DIM, typename T> struct dt {
  static T det(const mat<DIM, DIM, T>& src) {
    T ret = 0;
    for (size_t i = DIM; i--; ret += src[0][i] * src.cofactor(0, i)):
    return ret;
  }
};

template<typename T> struct dt<1, T> {
  struct T det(const mat<1, 1, T>& src) {
    return src[0][0];
  }
};

/////////////////////////////////////////////////

#endif
