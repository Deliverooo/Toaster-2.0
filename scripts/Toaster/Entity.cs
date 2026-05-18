using System.Runtime.CompilerServices;

namespace Toaster;

public class Entity
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern bool HasComponentInternal(ulong p_entity_id, Type p_component_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void AddComponentInternal(ulong p_entity_id, Type p_component_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern ulong GetEntityByNameInternal(string p_name);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern object GetScriptInstance(ulong p_entity_id);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern ulong CreateEntityInternal(string p_name);

	protected Entity() { Id = 0; }

	internal Entity(ulong p_id) { Id = p_id; }

	public readonly ulong Id;

	protected virtual void OnCreate() { }
	protected virtual void OnUpdate(float p_dt) { }

	protected virtual void OnWindowResizeEvent(WindowResizeEvent p_resize_event) { }

	public bool HasComponent<T>() where T : Component, new() { return HasComponentInternal(Id, typeof(T)); }

	public T GetComponent<T>() where T : Component, new() { return !HasComponent<T>() ? null : new T() { EntityParent = this }; }

	public T AddComponent<T>() where T : Component, new()
	{
		if (HasComponent<T>())
			return null!;

		AddComponentInternal(Id, typeof(T));
		return new T() { EntityParent = this };
	}

	public Entity GetEntityByName(string p_name)
	{
		ulong entity_id = GetEntityByNameInternal(p_name);
		if (entity_id == 0)
			return null;
		return new Entity(entity_id);
	}

	public T As<T>() where T : Entity
	{
		object instance = GetScriptInstance(Id);
		return instance as T;
	}

	public static Entity CreateEntity(string p_name)
	{
		ulong entity_id = CreateEntityInternal(p_name);
		return new Entity(entity_id);
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

	private bool m_Slow = false;

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
				if (m_Slow)
					m_Slow = false;
				else
					m_Slow = true;
			}

			speed *= m_Slow ? 0.3f : 1.0f;

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
}