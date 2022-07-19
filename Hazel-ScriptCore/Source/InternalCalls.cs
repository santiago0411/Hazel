using System.Runtime.CompilerServices;

namespace Hazel
{
    internal static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog(string text, int parameter);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog_Vector(ref Vector3 parameter);
    }
}
