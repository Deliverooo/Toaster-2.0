using System.Runtime.CompilerServices;

namespace Toaster;

public class Material
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetAlbedoColour(ulong p_entity_id, uint p_index, out Vec3 p_out_colour);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetAlbedoColour(ulong p_entity_id, uint p_index, ref Vec3 p_out_colour);

	private ulong EntityId;
	private uint Index;

	public Material(ulong p_entity_id, uint p_index)
	{
		EntityId = p_entity_id;
		Index = p_index;
	}

	public Vec3 AlbedoColour
	{
		get
		{
			GetAlbedoColour(EntityId, Index, out var colour);
			return colour;
		}
		set => SetAlbedoColour(EntityId, Index, ref value);
	}
}