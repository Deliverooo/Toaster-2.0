using System.Numerics;

namespace Toaster;

public class Player : Entity
{
	private TagComponent Tag;
	private TransformComponent Transform;
	private SpriteRendererComponent SpriteRenderer;

	private float Time = 0.0f;

	public void OnCreate()
	{
		Console.WriteLine("Player: OnCreate | ID: {0}", Id);

		Input.CursorMode = Input.ECursorMode.Disabled;

		Transform = GetComponent<TransformComponent>();
		Tag = GetComponent<TagComponent>();
		SpriteRenderer = AddComponent<SpriteRendererComponent>();

		SpriteRenderer.Colour = new Vec4(0.0f, 0.2f, 1.0f, 1.0f);
	}

	public void OnUpdate(float p_dt)
	{
		Time += p_dt;
		const float speed = 2.0f;

		float col = (float)System.Math.Abs(System.Math.Sin(Time));

		Vec4 new_colour = new Vec4(col, col, 1.0f, 1.0f);
		SpriteRenderer.Colour = new_colour;

		Vec3 translation = Transform.Translation;

		if (Input.IsMouseButtonDown(Input.EMouseButton.Middle) &&
		    Input.CursorMode != Input.ECursorMode.Normal)
			Input.CursorMode = Input.ECursorMode.Normal;

		if (Input.IsKeyDown(Input.EKeyCode.Up))
			translation.Y += speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Down))
			translation.Y -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Left))
			translation.X -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Right))
			translation.X += speed * p_dt;

		Transform.Translation = translation;
	}
}