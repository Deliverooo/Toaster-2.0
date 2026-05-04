using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace Toaster
{
	public static class InternalCalls
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern bool HasComponent(uint p_entity_id, Type p_component_type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void AddComponent(uint p_entity_id, Type p_component_type);
	}
}