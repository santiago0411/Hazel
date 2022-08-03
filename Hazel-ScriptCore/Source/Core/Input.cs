using System.Runtime.CompilerServices;

namespace Hazel
{
    public static class Input
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool IsKeyDown(KeyCode keyCode)
        {
            return InternalCalls.Input_IsKeyDown(keyCode);
        }
    }
}
