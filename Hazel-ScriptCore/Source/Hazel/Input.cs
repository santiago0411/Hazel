using System.Runtime.CompilerServices;

namespace HazelEngine
{
    public enum MouseButton
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        ButtonLast = Button7,
        ButtonLeft = Button0,
        ButtonRight = Button1,
        ButtonMiddle = Button2
    }
    
    public static class Input
    {
        public static bool IsKeyPressed(KeyCode keyCode)
        {
            return IsKeyPressed_Native(keyCode);
        }

        public static bool IsMouseButtonPressed(MouseButton mouseButton)
        {
            return IsMouseButtonPressed_Native(mouseButton);
        }

        public static Vector2 GetMousePosition()
        {
            GetMousePosition_Native(out Vector2 position);
            return position;
        }
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed_Native(KeyCode keyCode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsMouseButtonPressed_Native(MouseButton mouseButton);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetMousePosition_Native(out Vector2 position);
    }
}