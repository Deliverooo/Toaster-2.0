namespace Toaster;

public class Player : Entity
{
	public void OnCreate()
	{
		Console.WriteLine("Player: OnCreate | ID: {0}", ID);
	}

	public void OnUpdate(float p_dt)
	{
		Vec3 translation = Translation;
		translation.X += 0.25f * p_dt;
		Translation = translation;
	}
}