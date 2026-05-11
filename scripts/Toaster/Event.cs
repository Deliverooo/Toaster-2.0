namespace Toaster;

public struct WindowResizeEvent
{
	public WindowResizeEvent() { }

	public Vec2 Size = Vec2.Zero;
	public float GetAspectRatio() => Size.X / Size.Y;
}