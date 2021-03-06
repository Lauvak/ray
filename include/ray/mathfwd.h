// +----------------------------------------------------------------------
// | Project : ray.
// | All rights reserved.
// +----------------------------------------------------------------------
// | Copyright (c) 2013-2017.
// +----------------------------------------------------------------------
// | * Redistribution and use of this software in source and binary forms,
// |   with or without modification, are permitted provided that the following
// |   conditions are met:
// |
// | * Redistributions of source code must retain the above
// |   copyright notice, this list of conditions and the
// |   following disclaimer.
// |
// | * Redistributions in binary form must reproduce the above
// |   copyright notice, this list of conditions and the
// |   following disclaimer in the documentation and/or other
// |   materials provided with the distribution.
// |
// | * Neither the name of the ray team, nor the names of its
// |   contributors may be used to endorse or promote products
// |   derived from this software without specific prior
// |   written permission of the ray team.
// |
// | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// | "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// | LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// | A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// | OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// | SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// | LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// | DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// | THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// | (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// | OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// +----------------------------------------------------------------------
#ifndef _H_MATHFWD_H_
#define _H_MATHFWD_H_

#include <ray/mathutil.h>

_NAME_BEGIN

template<typename T = int>
class Sizet;

template<typename T = int>
class Pointt;

template<typename T = int>
class Rectt;

template<typename T = std::uint32_t>
class Binaryt;

template<typename T = float>
class Viewportt;

template<typename T = int>
class Scissort;

template<typename T = float>
class Vector2t;

template<typename T = float>
class Vector3t;

template<typename T = float>
class Vector4t;

template<typename T = float>
class Color3t;

template<typename T = float>
class Color4t;

template<typename T = float>
class RGBt;

template<typename T = float>
class RGBAt;

template<typename T = float>
class BGRt;

template<typename T = float>
class BGRAt;

template<typename T = float>
class Matrix2x2t;

template<typename T = float>
class Matrix3x3t;

template<typename T = float>
class Matrix4x4t;

template<typename T = float>
class Raycast2t;

template<typename T = float>
class Raycast3t;

template<typename T = float>
class Triangle3t;

template<typename T = float>
class Quaterniont;

template<typename T = float>
class Line2t;

template<typename T = float>
class Line3t;

template<typename T = float>
class Box2t;

template<typename T = float>
class AABBt;

template<typename T = float>
class OBBt;

template<typename T = float>
class BoundingBoxt;

template<typename T = float>
class Frustumt;

template<typename T = float>
class Plane2t;

template<typename T = float>
class Plane3t;

template<typename T = float>
class Rect3t;

template<typename T = float>
class Spheret;

// default
typedef Sizet<> Size;
typedef Pointt<> Point;
typedef Rectt<> Rect;
typedef Binaryt<> Binary;
typedef Viewportt<> Viewport;
typedef Scissort<> Scissor;
typedef Vector2t<> Vector2;
typedef Vector3t<> Vector3;
typedef Vector4t<> Vector4;
typedef Vector2t<> Texcoord;
typedef Matrix2x2t<> Matrix2x2;
typedef Matrix3x3t<> Matrix3x3;
typedef Matrix4x4t<> Matrix4x4;
typedef Raycast3t<> Raycast3;
typedef Triangle3t<> Triangle3;
typedef Color3t<> Color3;
typedef Color4t<> Color4;
typedef Quaterniont<> Quaternion;
typedef Line3t<> Line3;
typedef Box2t<> Box2;
typedef AABBt<> AABB;
typedef BoundingBoxt<> BoundingBox;
typedef Frustumt<> Frustum;
typedef OBBt<> OBB;
typedef Plane3t<> Plane3;
typedef Rect3t<> Rect3;
typedef Spheret<> Sphere;

// float
typedef Sizet<float> Sizef;
typedef Pointt<float> Pointf;
typedef Rectt<float> Rectf;
typedef Viewportt<float> Viewportf;
typedef Vector2t<float> Vector2f;
typedef Vector3t<float> Vector3f;
typedef Vector4t<float> Vector4f;
typedef Vector2t<float> Texcoordf;
typedef Vector3t<float> Normalf;
typedef Matrix2x2t<float> Matrix2x2f;
typedef Matrix3x3t<float> Matrix3x3f;
typedef Matrix4x4t<float> Matrix4x4f;
typedef Matrix2x2t<float> float2x2;
typedef Matrix3x3t<float> float3x3;
typedef Matrix4x4t<float> float4x4;
typedef Raycast3t<float> Raycast3f;
typedef Triangle3t<float> Triangle3f;
typedef Color3t<float> Color3f;
typedef Color4t<float> Color4f;
typedef Quaterniont<float> Quaternionf;
typedef Line3t<float> Line3f;
typedef AABBt<float> AABBf;
typedef BoundingBoxt<float> BoundingBoxf;
typedef Frustumt<float> Frustumf;
typedef OBBt<float> OBBf;
typedef Plane3t<float> Plane3f;
typedef Rect3t<float> Rect3f;
typedef Spheret<float> Spheref;

// double
typedef Sizet<double> Sized;
typedef Pointt<double> Pointd;
typedef Rectt<double> Rectd;
typedef Viewportt<double> Viewportd;
typedef Vector2t<double> Vector2d;
typedef Vector3t<double> Vector3d;
typedef Vector4t<double> Vector4d;
typedef Vector2t<double> Texcoordd;
typedef Vector3t<double> Normald;
typedef Matrix2x2t<double> Matrix2x2d;
typedef Matrix3x3t<double> Matrix3x3d;
typedef Matrix4x4t<double> Matrix4x4d;
typedef Matrix2x2t<double> double2x2;
typedef Matrix3x3t<double> double3x3;
typedef Matrix4x4t<double> double4x4;
typedef Raycast3t<double> Raycast3d;
typedef Triangle3t<double> Triangle3d;
typedef Color3t<double> Color3d;
typedef Color4t<double> Color4d;
typedef Quaterniont<double> Quaterniond;
typedef Line3t<double> Line3d;
typedef AABBt<double> AABBd;
typedef BoundingBoxt<double> BoundingBoxd;
typedef Frustumt<double> Frustumd;
typedef OBBt<double> OBBd;
typedef Plane3t<double> Plane3d;
typedef Rect3t<double> Rect3d;
typedef Spheret<double> Sphered;

// char
typedef std::int8_t char1;
typedef Vector2t<std::int8_t> char2;
typedef Vector3t<std::int8_t> char3;
typedef Vector4t<std::int8_t> char4;

// uchar
typedef std::uint8_t uchar1;
typedef Vector2t<std::uint8_t> uchar2;
typedef Vector3t<std::uint8_t> uchar3;
typedef Vector4t<std::uint8_t> uchar4;

typedef RGBt<unsigned char> RGB;
typedef RGBAt<unsigned char> RGBA;
typedef BGRt<unsigned char> BGR;
typedef BGRAt<unsigned char> BGRA;

// short
typedef short short1;
typedef Vector2t<std::uint16_t> short2;
typedef Vector3t<std::uint16_t> short3;
typedef Vector4t<std::uint16_t> short4;

// ushort
typedef unsigned short ushort1;
typedef Vector2t<std::uint16_t> ushort2;
typedef Vector3t<std::uint16_t> ushort3;
typedef Vector4t<std::uint16_t> ushort4;

// int
typedef int int1;
typedef Vector2t<std::int32_t> int2;
typedef Vector3t<std::int32_t> int3;
typedef Vector4t<std::int32_t> int4;

// uint
typedef std::uint32_t uint;
typedef std::uint32_t uint1;
typedef Vector2t<std::uint32_t> uint2;
typedef Vector3t<std::uint32_t> uint3;
typedef Vector4t<std::uint32_t> uint4;

// float
typedef float float1;
typedef Vector2t<float> float2;
typedef Vector3t<float> float3;
typedef Vector4t<float> float4;

// double
typedef double double1;
typedef Vector2t<double> double2;
typedef Vector3t<double> double3;
typedef Vector4t<double> double4;

// array
typedef std::vector<int> IntArray;
typedef std::vector<int2> Int2Array;
typedef std::vector<int3> Int3Array;
typedef std::vector<int4> Int4Array;
typedef std::vector<uint> UintArray;
typedef std::vector<uint2> Uint2Array;
typedef std::vector<uint3> Uint3Array;
typedef std::vector<uint4> Uint4Array;
typedef std::vector<Vector2> Vector2Array;
typedef std::vector<Vector3> Vector3Array;
typedef std::vector<Vector4> Vector4Array;
typedef std::vector<float>  FloatArray;
typedef std::vector<float2> Float2Array;
typedef std::vector<float3> Float3Array;
typedef std::vector<float4> Float4Array;
typedef std::vector<double> DoubleArray;
typedef std::vector<double2> Double2Array;
typedef std::vector<double3> Double3Array;
typedef std::vector<double4> Double4Array;
typedef std::vector<float2x2> Float2x2Array;
typedef std::vector<float3x3> Float3x3Array;
typedef std::vector<float4x4> Float4x4Array;
typedef std::vector<double2x2> double2x2Array;
typedef std::vector<double3x3> double3x3Array;
typedef std::vector<double4x4> double4x4Array;

_NAME_END

#endif