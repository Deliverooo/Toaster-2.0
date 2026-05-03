using System.Numerics;

namespace Toaster;

public class Player : Entity
{
	private TagComponent Tag;
	private TransformComponent Transform;
	private SpriteRendererComponent SpriteRenderer;
	private MeshComponent Mesh;

	private float Time = 0.0f;

	public void OnCreate()
	{
		Transform = GetComponent<TransformComponent>();
		Tag = GetComponent<TagComponent>();
		SpriteRenderer = AddComponent<SpriteRendererComponent>();
		Mesh = GetComponent<MeshComponent>();
	}

	public void OnUpdate(float p_dt)
	{
		Time += p_dt;
		const float speed = 2.0f;

		float col = (float)System.Math.Abs(System.Math.Sin(Time));
		SpriteRenderer.Colour = new Vec4(col, col, 1.0f, 1.0f);
		
		Material mat = Mesh.GetMaterial(0);
		mat.AlbedoColour = new Vec3(col, 1.0f, 1.0f);

		Vec3 translation = Transform.Translation;

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