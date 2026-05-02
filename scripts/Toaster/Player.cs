namespace Toaster;

public class Player : Entity
{
	public void OnCreate()
	{
		Console.WriteLine("Player: OnCreate");
	}

	public void OnUpdate(float p_dt)
	{
		Console.WriteLine("Player: OnUpdate {0}", p_dt);
	}
}