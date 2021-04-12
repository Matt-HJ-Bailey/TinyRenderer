#include <cmath>
#include "geometry.h"

template <> template <> vec<3,int>  ::vec(const vec<3,double> &v) : x(std::lround(v.x)),y(std::lround(v.y)),z(std::lround(v.z)) {}
template <> template <> vec<3,double>::vec(const vec<3,int> &v)   : x(v.x),y(v.y),z(v.z) {}
template <> template <> vec<2,int>  ::vec(const vec<2,double> &v) : x(std::lround(v.x)),y(std::lround(v.y)) {}
template <> template <> vec<2,double>::vec(const vec<2,int> &v)   : x(v.x),y(v.y) {}