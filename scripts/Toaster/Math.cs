namespace Toaster;

public struct Vec2
{
	public float X;
	public float Y;

	public static Vec2 Zero => new Vec2(0.0f);

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

	public static Vec2 operator +(Vec2 p_a, Vec2 p_b)
	{
		return new Vec2(p_a.X + p_b.X, p_a.Y + p_b.Y);
	}

	public static Vec2 operator -(Vec2 p_a, Vec2 p_b)
	{
		return new Vec2(p_b.X - p_a.X, p_b.Y - p_a.Y);
	}

	public static Vec2 operator *(Vec2 p_a, float p_b)
	{
		return new Vec2(p_a.X * p_b, p_a.Y * p_b);
	}

	public static Vec2 operator /(Vec2 p_a, float p_b)
	{
		return new Vec2(p_a.X / p_b, p_a.Y / p_b);
	}
}

public struct Vec3
{
	public float X;
	public float Y;
	public float Z;

	public static Vec3 Zero => new Vec3(0.0f);

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

	public static Vec3 operator +(Vec3 p_a, Vec3 p_b)
	{
		return new Vec3(p_a.X + p_b.X, p_a.Y + p_b.Y, p_a.Z + p_b.Z);
	}

	public static Vec3 operator -(Vec3 p_a, Vec3 p_b)
	{
		return new Vec3(p_b.X - p_a.X, p_b.Y - p_a.Y, p_b.Z - p_a.Z);
	}

	public static Vec3 operator *(Vec3 p_a, float p_b)
	{
		return new Vec3(p_a.X * p_b, p_a.Y * p_b, p_a.Z * p_b);
	}

	public static Vec3 operator /(Vec3 p_a, float p_b)
	{
		return new Vec3(p_a.X / p_b, p_a.Y / p_b, p_a.Z / p_b);
	}
}

public struct Vec4
{
	public float X;
	public float Y;
	public float Z;
	public float W;

	public static Vec4 Zero => new Vec4(0.0f);

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

	public static Vec4 operator +(Vec4 p_a, Vec4 p_b)
	{
		return new Vec4(p_a.X + p_b.X, p_a.Y + p_b.Y, p_a.Z + p_b.Z, p_a.W + p_b.W);
	}

	public static Vec4 operator -(Vec4 p_a, Vec4 p_b)
	{
		return new Vec4(p_b.X - p_a.X, p_b.Y - p_a.Y, p_b.Z - p_a.Z, p_b.W - p_a.W);
	}

	public static Vec4 operator *(Vec4 p_a, float p_b)
	{
		return new Vec4(p_a.X * p_b, p_a.Y * p_b, p_a.Z * p_b, p_a.W * p_b);
	}

	public static Vec4 operator /(Vec4 p_a, float p_b)
	{
		return new Vec4(p_a.X / p_b, p_a.Y / p_b, p_a.Z / p_b, p_a.W / p_b);
	}
}

public static class Math
{
	public static float Dot(Vec2 p_a, Vec2 p_b)
	{
		return (p_a.X * p_b.X) + (p_a.Y * p_b.Y);
	}

	public static float Dot(Vec3 p_a, Vec3 p_b)
	{
		return (p_a.X * p_b.X) + (p_a.Y * p_b.Y) + (p_a.Z * p_b.Z);
	}

	public static float Dot(Vec4 p_a, Vec4 p_b)
	{
		return (p_a.X * p_b.X) + (p_a.Y * p_b.Y) + (p_a.Z * p_b.Z) + (p_a.W * p_b.W);
	}

	public static float Length(Vec2 p_a)
	{
		return (float)System.Math.Sqrt((p_a.X * p_a.X) + (p_a.Y * p_a.Y));
	}

	public static float Length(Vec3 p_a)
	{
		return (float)System.Math.Sqrt((p_a.X * p_a.X) + (p_a.Y * p_a.Y) + (p_a.Z * p_a.Z));
	}

	public static float Length(Vec4 p_a)
	{
		return (float)System.Math.Sqrt((p_a.X * p_a.X) + (p_a.Y * p_a.Y) + (p_a.Z * p_a.Z) +
		                               (p_a.W * p_a.W));
	}

	public static Vec2 Normalise(Vec2 p_a)
	{
		float l = Length(p_a);
		return (l == 0.0f) ? Vec2.Zero : p_a / l;
	}

	public static Vec3 Normalise(Vec3 p_a)
	{
		float l = Length(p_a);
		return (l == 0.0f) ? Vec3.Zero : p_a / l;
	}

	public static Vec4 Normalise(Vec4 p_a)
	{
		float l = Length(p_a);
		return (l == 0.0f) ? Vec4.Zero : p_a / l;
	}
}