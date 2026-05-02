namespace Toaster;

public class Player : Entity
{
	public void OnCreate()
	{
		Console.WriteLine("Player: OnCreate | ID: {0}", ID);
	}

	public void OnUpdate(float p_dt)
	{
		const float speed = 2.0f;

		Vec3 translation = Translation;
		if (Input.IsKeyDown(Input.KeyCode.Up))
			translation.Y += speed * p_dt;
		if (Input.IsKeyDown(Input.KeyCode.Down))
			translation.Y -= speed * p_dt;
		if (Input.IsKeyDown(Input.KeyCode.Left))
			translation.X -= speed * p_dt;
		if (Input.IsKeyDown(Input.KeyCode.Right))
			translation.X += speed * p_dt;

		Translation = translation;
	}
}