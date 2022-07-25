using System.Runtime.InteropServices;

namespace Hazel
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;

        public Vector3 Up => new Quaternion(Rotation) * Vector3.Up;
        public Vector3 Right => new Quaternion(Rotation) * Vector3.Right;
        public Vector3 Forward => new Quaternion(Rotation) * Vector3.Forward;
        
        public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
        {
            Position = position;
            Rotation = rotation;
            Scale = scale;
        }
    }
}