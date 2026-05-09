global using Vec2 = System.Numerics.Vector2;
global using Vec3 = System.Numerics.Vector3;
global using Vec4 = System.Numerics.Vector4;
global using Quat = System.Numerics.Quaternion;
global using Mat4 = System.Numerics.Matrix4x4;

namespace Toaster;

public static class Math
{
	public static float Radians(float p_degrees) { return p_degrees * ((float)System.Math.PI / 180.0f); }
	public static float Degrees(float p_radians) { return p_radians * (180.0f / (float)System.Math.PI); }
}