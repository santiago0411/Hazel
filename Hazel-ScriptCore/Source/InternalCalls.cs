using System.Runtime.CompilerServices;

namespace Hazel
{
    internal static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog(string text, int parameter);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog_Vector(ref Vector3 parameter);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_GetPosition(ulong entityId, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Entity_SetPosition(ulong entityId, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode keyCode);
    }
}
