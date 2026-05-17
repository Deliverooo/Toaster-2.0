global using Vec2 = System.Numerics.Vector2;
global using Vec3 = System.Numerics.Vector3;
global using Vec4 = System.Numerics.Vector4;
global using Quat = System.Numerics.Quaternion;
global using Mat4 = System.Numerics.Matrix4x4;
using Toaster;
using Math = System.Math;

namespace Sandbox;

public class Player : Entity
{
	private TagComponent m_Tag;
	private TransformComponent m_Transform;
	private MeshComponent m_Mesh;

	private float m_Time = 0.0f;

	protected override void OnCreate()
	{
		m_Transform = GetComponent<TransformComponent>();
		m_Tag = GetComponent<TagComponent>();
		m_Mesh = GetComponent<MeshComponent>();
	}

	protected override void OnUpdate(float p_dt)
	{
		m_Time += p_dt;
		const float speed = 2.0f;
        
        if(Input.Is)

		//SpriteRenderer.Colour = new Vec4(col, col, 1.0f, 1.0f);
		// float col = (float)System.Math.Abs(System.Math.Sin(m_Time));

		// Material mat = Mesh.GetMaterial(0);
		// mat.AlbedoColour = new Vec3(col, 1.0f, 1.0f)	;

		Vec3 translation = m_Transform.Translation;

		if (Input.IsKeyDown(Input.EKeyCode.Up))
			translation.Y += speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Down))
			translation.Y -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Left))
			translation.X -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Right))
			translation.X += speed * p_dt;

		m_Transform.Translation = translation;
	}
}

public class CameraController : Entity
{
	private TransformComponent m_Transform;
	private CameraComponent m_Camera;

	private Vec2 m_InitialMousePos = Vec2.Zero;

	private float m_Time = 0.0f;

	private float m_Yaw = 0.0f;
	private float m_Pitch = 0.0f;

	private bool m_slow = false;

	public static readonly Vec3 ForwardDir = new Vec3(0.0f, 0.0f, -1.0f);
	public static readonly Vec3 RightDir = new Vec3(1.0f, 0.0f, 0.0f);
	public static readonly Vec3 UpDir = new Vec3(0.0f, 1.0f, 0.0f);

	public Quat GetOrientation() { return Quat.CreateFromYawPitchRoll(-m_Yaw, -m_Pitch, 0.0f); }

	public Vec3 GetForwardVector() { return Vec3.Transform(ForwardDir, Mat4.CreateFromQuaternion(GetOrientation())); }
	public Vec3 GetRightVector() { return Vec3.Transform(RightDir, Mat4.CreateFromQuaternion(GetOrientation())); }
	public Vec3 GetUpVector() { return Vec3.Transform(UpDir, Mat4.CreateFromQuaternion(GetOrientation())); }

	protected override void OnCreate()
	{
		m_Transform = GetComponent<TransformComponent>();
		m_Transform.Translation = new Vec3(0.0f, 1.5f, 2.5f);

		m_Camera = AddComponent<CameraComponent>();
		m_Camera.ProjectionType = CameraComponent.EProjectionType.Perspective;
		m_Camera.PerspectiveFov = Toaster.Math.Radians(90.0f);
		m_Camera.Primary = true;
	}

	protected override void OnUpdate(float p_dt)
	{
		m_Time += p_dt;

		if (Input.IsMouseButtonDown(Input.EMouseButton.Right))
		{
			if (Input.CursorMode != Input.ECursorMode.Disabled)
				Input.CursorMode = Input.ECursorMode.Disabled;

			float fov = m_Camera.PerspectiveFov;
			fov -= Toaster.Math.Radians(Input.MouseScroll.Y);
			m_Camera.PerspectiveFov = fov;

			float speed = Input.IsKeyDown(Input.EKeyCode.LeftControl) ? 30.0f : 10.0f;

			Vec3 delta_position = Vec3.Zero;
			if (Input.IsKeyDown(Input.EKeyCode.W))
				delta_position += ForwardDir;
			if (Input.IsKeyDown(Input.EKeyCode.A))
				delta_position -= RightDir;
			if (Input.IsKeyDown(Input.EKeyCode.S))
				delta_position -= ForwardDir;
			if (Input.IsKeyDown(Input.EKeyCode.D))
				delta_position += RightDir;

			delta_position = (delta_position.Length() == 0.0f) ? Vec3.Zero : Vec3.Normalize(delta_position) * p_dt;

			if (Input.IsKeyPressed(Input.EKeyCode.L))
			{
				if (m_slow)
					m_slow = false;
				else
					m_slow = true;
			}

			speed *= m_slow ? 0.3f : 1.0f;

			Quat orientation = GetOrientation();
			Vec4 rotated_position = Vec4.Transform(new Vec4(delta_position, 0.0f), Mat4.CreateFromQuaternion(orientation));
			m_Transform.Translation += new Vec3(rotated_position.X, rotated_position.Y, rotated_position.Z) * speed;

			if (Input.IsKeyDown(Input.EKeyCode.Space))
				m_Transform.Translation += UpDir * p_dt * speed;
			if (Input.IsKeyDown(Input.EKeyCode.LeftShift))
				m_Transform.Translation -= UpDir * p_dt * speed;

			Vec2 mouse = Input.MousePos;
			Vec2 mouse_delta = (mouse - m_InitialMousePos) * 0.002f;
			m_Yaw += mouse_delta.X;
			m_Pitch += mouse_delta.Y;
			if (m_Pitch > Toaster.Math.Radians(89.0f))
				m_Pitch = Toaster.Math.Radians(89.0f);
			if (m_Pitch < Toaster.Math.Radians(-89.0f))
				m_Pitch = Toaster.Math.Radians(-89.0f);

			m_InitialMousePos = mouse;

			m_Transform.Rotation = orientation;
		}
		else
		{
			if (Input.CursorMode != Input.ECursorMode.Normal)
				Input.CursorMode = Input.ECursorMode.Normal;

			m_InitialMousePos = Input.MousePos;
		}
	}

	protected override void OnWindowResizeEvent(WindowResizeEvent p_resize_event)
	{
		Console.WriteLine("Size: [{0}, {1}] | Aspect: {2}", p_resize_event.Size.X, p_resize_event.Size.Y, p_resize_event.GetAspectRatio());
	}
}

public class Test
{
	public int m_Test = 2;
	public static int s_Test = 0;
	public static string s_Str = "Orbicular Peeb";

	public int OrboMethod() { return m_Test; }

	public static int StaticOrbo() { return s_Test; }
}