global using Vec2 = System.Numerics.Vector2;
global using Vec3 = System.Numerics.Vector3;
global using Vec4 = System.Numerics.Vector4;
global using Quat = System.Numerics.Quaternion;
global using Mat4 = System.Numerics.Matrix4x4;

#if false
public struct Vec2
{
	public static Vec2 Zero => new Vec2(0.0f);

	public float X;
	public float Y;

	public Vec2(float p_x, float p_y)
	{
		X = p_x;
		Y = p_y;
	}

	public Vec2(float p_s)
	{
		X = p_s;
		Y = p_s;
	}

	public static Vec2 operator +(Vec2 p_a, Vec2 p_b) { return new Vec2(p_a.X + p_b.X, p_a.Y + p_b.Y); }
	public static Vec2 operator -(Vec2 p_a, Vec2 p_b) { return new Vec2(p_a.X - p_b.X, p_a.Y - p_b.Y); }
	public static Vec2 operator *(Vec2 p_a, float p_b) { return new Vec2(p_a.X * p_b, p_a.Y * p_b); }
	public static Vec2 operator /(Vec2 p_a, float p_b) { return new Vec2(p_a.X / p_b, p_a.Y / p_b); }
}

public struct Vec3
{
	public static Vec3 Zero => new Vec3(0.0f);

	public float X;
	public float Y;
	public float Z;

	public Vec3(float p_x, float p_y, float p_z)
	{
		X = p_x;
		Y = p_y;
		Z = p_z;
	}

	public Vec3(float p_s)
	{
		X = p_s;
		Y = p_s;
		Z = p_s;
	}

	public Vec3(Vec4 p_vec4)
	{
		X = p_vec4.X;
		Y = p_vec4.Y;
		Z = p_vec4.Z;
	}

	public static Vec3 operator +(Vec3 p_a, Vec3 p_b) { return new Vec3(p_a.X + p_b.X, p_a.Y + p_b.Y, p_a.Z + p_b.Z); }
	public static Vec3 operator -(Vec3 p_a, Vec3 p_b) { return new Vec3(p_a.X - p_b.X, p_a.Y - p_b.Y, p_a.Z - p_b.Z); }
	public static Vec3 operator *(Vec3 p_a, float p_b) { return new Vec3(p_a.X * p_b, p_a.Y * p_b, p_a.Z * p_b); }
	public static Vec3 operator /(Vec3 p_a, float p_b) { return new Vec3(p_a.X / p_b, p_a.Y / p_b, p_a.Z / p_b); }
}

public struct Vec4
{
	public static Vec4 Zero => new Vec4(0.0f);

	public float X;
	public float Y;
	public float Z;
	public float W;

	public Vec4(float p_x, float p_y, float p_z, float p_w)
	{
		X = p_x;
		Y = p_y;
		Z = p_z;
		W = p_w;
	}

	public Vec4(float p_s)
	{
		X = p_s;
		Y = p_s;
		Z = p_s;
		W = p_s;
	}

	public Vec4(Vec2 p_vec2, float p_z, float p_w)
	{
		X = p_vec2.X;
		Y = p_vec2.Y;
		Z = p_z;
		W = p_w;
	}

	public Vec4(Vec2 p_vec1, Vec2 p_vec2)
	{
		X = p_vec1.X;
		Y = p_vec1.Y;
		Z = p_vec2.X;
		W = p_vec2.Y;
	}

	public Vec4(Vec3 p_vec3, float p_w)
	{
		X = p_vec3.X;
		Y = p_vec3.Y;
		Z = p_vec3.Z;
		W = p_w;
	}

	public static Vec4 operator +(Vec4 p_a, Vec4 p_b) { return new Vec4(p_a.X + p_b.X, p_a.Y + p_b.Y, p_a.Z + p_b.Z, p_a.W + p_b.W); }
	public static Vec4 operator -(Vec4 p_a, Vec4 p_b) { return new Vec4(p_a.X - p_b.X, p_a.Y - p_b.Y, p_a.Z - p_b.Z, p_a.W - p_b.W); }
	public static Vec4 operator *(Vec4 p_a, float p_b) { return new Vec4(p_a.X * p_b, p_a.Y * p_b, p_a.Z * p_b, p_a.W * p_b); }
	public static Vec4 operator /(Vec4 p_a, float p_b) { return new Vec4(p_a.X / p_b, p_a.Y / p_b, p_a.Z / p_b, p_a.W / p_b); }

	public static Vec4 operator *(Vec4 p_v, Mat4 p_m) { return new Vec4(); }
}

// [a11, a12]
// [a21, a22]
public struct Mat2
{
	public static Mat2 Zero = new Mat2(0.0f);
	public static Mat2 Identity = new Mat2(1.0f);

	public float[,] Data { get; set; }

	public Mat2(float p_m11, float p_m12, float p_m21, float p_m22) { Data = new float[2, 2] { { p_m11, p_m12 }, { p_m21, p_m22 } }; }

	public Mat2(float p_s) { Data = new float[2, 2] { { p_s, 0.0f }, { 0.0f, p_s } }; }

	public Mat2(Vec2 p_v1, Vec2 p_v2) { Data = new float[2, 2] { { p_v1.X, p_v1.Y }, { p_v2.X, p_v2.Y } }; }

	public static Mat2 operator *(Mat2 p_m1, Mat2 p_m2)
	{
		var result = new Mat2(0.0f);
		for (uint i = 0; i < 2; ++i)
		{
			for (uint j = 0; j < 2; ++j)
			{
				result.Data[i, j] = p_m1.Data[i, 0] * p_m2.Data[0, j] + p_m1.Data[i, 1] * p_m2.Data[1, j];
			}
		}

		return result;
	}
}

// [a11, a12, a13]
// [a21, a22, a23]
// [a31, a32, a33]
public struct Mat3
{
	public static Mat3 Zero = new Mat3(0.0f);
	public static Mat3 Identity = new Mat3(1.0f);

	public float[,] Data { get; set; }

	public Mat3(float p_m11, float p_m12, float p_m13, float p_m21, float p_m22, float p_m23, float p_m31, float p_m32, float p_m33)
	{
		Data = new float[3, 3] { { p_m11, p_m12, p_m13 }, { p_m21, p_m22, p_m23 }, { p_m31, p_m32, p_m33 } };
	}

	public Mat3(float p_s) { Data = new float[3, 3] { { p_s, 0.0f, 0.0f }, { 0.0f, p_s, 0.0f }, { 0.0f, 0.0f, p_s } }; }

	public Mat3(Vec3 p_v1, Vec3 p_v2, Vec3 p_v3) { Data = new float[3, 3] { { p_v1.X, p_v1.Y, p_v1.Z }, { p_v2.X, p_v2.Y, p_v2.Z }, { p_v3.X, p_v3.Y, p_v3.Z } }; }

	public static Mat3 operator *(Mat3 p_m1, Mat3 p_m2)
	{
		var result = new Mat3(0.0f);
		for (uint i = 0; i < 3; ++i)
		{
			for (uint j = 0; j < 3; ++j)
			{
				result.Data[i, j] = p_m1.Data[i, 0] * p_m2.Data[0, j] + p_m1.Data[i, 1] * p_m2.Data[1, j] + p_m1.Data[i, 2] * p_m2.Data[2, j];
			}
		}

		return result;
	}
}

// [a11, a12, a13, a14]
// [a21, a22, a23, a24]
// [a31, a32, a33, a34]
// [a41, a42, a43, a44]
public struct Mat4
{
	public static Mat4 Zero = new Mat4(0.0f);
	public static Mat4 Identity = new Mat4(1.0f);

	public float M11;
	public float M12;
	public float M13;
	public float M14;
	public float M21;
	public float M22;
	public float M23;
	public float M24;
	public float M31;
	public float M32;
	public float M33;
	public float M34;
	public float M41;
	public float M42;
	public float M43;
	public float M44;

	public Mat4(float p_m11,
		float p_m12,
		float p_m13,
		float p_m14,
		float p_m21,
		float p_m22,
		float p_m23,
		float p_m24,
		float p_m31,
		float p_m32,
		float p_m33,
		float p_m34,
		float p_m41,
		float p_m42,
		float p_m43,
		float p_m44)
	{
		M11 = p_m11;
		M12 = p_m12;
		M13 = p_m13;
		M14 = p_m14;
		M21 = p_m21;
		M22 = p_m22;
		M23 = p_m23;
		M24 = p_m24;
		M31 = p_m31;
		M32 = p_m32;
		M33 = p_m33;
		M34 = p_m34;
		M41 = p_m41;
		M42 = p_m42;
		M43 = p_m43;
		M44 = p_m44;
	}

	public Mat4(float p_s)
	{
		M11 = p_s;
		M12 = 0.0f;
		M13 = 0.0f;
		M14 = 0.0f;
		M21 = 0.0f;
		M22 = p_s;
		M23 = 0.0f;
		M24 = 0.0f;
		M31 = 0.0f;
		M32 = 0.0f;
		M33 = p_s;
		M34 = 0.0f;
		M41 = 0.0f;
		M42 = 0.0f;
		M43 = 0.0f;
		M44 = p_s;
	}

	public Mat4(Vec4 p_v1, Vec4 p_v2, Vec4 p_v3, Vec4 p_v4)
	{
		M11 = p_v1.X;
		M12 = p_v1.Y;
		M13 = p_v1.Z;
		M14 = p_v1.W;
		M21 = p_v2.X;
		M22 = p_v2.Y;
		M23 = p_v2.Z;
		M24 = p_v2.W;
		M31 = p_v3.X;
		M32 = p_v3.Y;
		M33 = p_v3.Z;
		M34 = p_v3.W;
		M41 = p_v4.X;
		M42 = p_v4.Y;
		M43 = p_v4.Z;
		M44 = p_v4.W;
	}

	public static Mat4 operator *(Mat4 p_m1, Mat4 p_m2)
	{

		return new Mat4(1.0f);
	}
}

public struct Quat
{
	public static Quat Zero = new Quat(0.0f, 0.0f, 0.0f, 0.0f);
	public static Quat Identity = new Quat(0.0f, 0.0f, 0.0f, 1.0f);

	public float X;
	public float Y;
	public float Z;
	public float W;

	Quat(float p_x, float p_y, float p_z, float p_w)
	{
		X = p_x;
		Y = p_y;
		Z = p_z;
		W = p_w;
	}

	Quat(float p_s)
	{
		X = p_s;
		Y = p_s;
		Z = p_s;
		W = p_s;
	}

	public static Quat operator *(Quat p_a, Quat p_b)
	{
		return new Quat(p_a.W * p_b.X + p_a.X * p_b.W + p_a.Y * p_b.Z - p_a.Z * p_b.Y, p_a.W * p_b.Y + p_a.Y * p_b.W + p_a.Z * p_b.X - p_a.X * p_b.Z,
			p_a.W * p_b.Z + p_a.Z * p_b.W + p_a.X * p_b.Y - p_a.Y * p_b.X, p_a.W * p_b.W - p_a.X * p_b.X - p_a.Y * p_b.Y - p_a.Z * p_b.Z);
	}

	public Quat Conjugate() => new Quat(-X, -Y, -Z, W);

	public static Quat CreateFromAxisAngle(float p_axis_x, float p_axis_y, float p_axis_z, float p_angle)
	{
		float half_angle = p_angle * 0.5f;
		float s = (float)System.Math.Sin(half_angle);
		return new Quat(p_axis_x * s, p_axis_y * s, p_axis_z * s, (float)System.Math.Cos(half_angle));
	}

	public Mat4 ToMat4()
	{
		float x2 = X + X;
		float y2 = Y + Y;
		float z2 = Z + Z;
		float xx = X * x2;
		float xy = X * y2;
		float xz = X * z2;
		float yy = Y * y2;
		float yz = Y * z2;
		float zz = Z * z2;
		float wx = W * x2;
		float wy = W * y2;
		float wz = W * z2;

		return new Mat4(1f - (yy + zz), xy - wz, xz + wy, 0f, xy + wz, 1f - (xx + zz), yz - wx, 0f, xz - wy, yz + wx, 1f - (xx + yy), 0f, 0f, 0f, 0f, 1f);
	}
}

#endif

namespace Toaster;

public static class Math
{
	public static float Radians(float p_degrees) { return p_degrees * ((float)System.Math.PI / 180.0f); }
	public static float Degrees(float p_radians) { return p_radians * (180.0f / (float)System.Math.PI); }
}