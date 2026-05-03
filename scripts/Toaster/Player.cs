namespace Toaster;

public class Player : Entity
{
	public void OnCreate()
	{
		Console.WriteLine("Player: OnCreate | ID: {0}", ID);

		Input.CursorMode = Input.ECursorMode.Disabled;
	}

	public void OnUpdate(float p_dt)
	{
		const float speed = 2.0f;

		Vec3 translation = Translation;

		if (Input.IsMouseButtonDown(Input.EMouseButton.Middle) && Input.CursorMode != Input.ECursorMode.Normal)
			Input.CursorMode = Input.ECursorMode.Normal;

		if (Input.IsKeyDown(Input.EKeyCode.Up))
			translation.Y += speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Down))
			translation.Y -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Left))
			translation.X -= speed * p_dt;
		if (Input.IsKeyDown(Input.EKeyCode.Right))
			translation.X += speed * p_dt;

		Translation = translation;
	}
}