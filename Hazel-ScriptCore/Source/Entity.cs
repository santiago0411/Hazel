using System.Runtime.CompilerServices;

namespace Hazel
{
    public class Entity
    {
        public readonly ulong Id;

        protected Entity() { Id = 0; }

        internal Entity(ulong id)
        {
            Id = id;
        }

        public Vector3 Position
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                InternalCalls.Entity_GetPosition(Id, out var position);
                return position;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.Entity_SetPosition(Id, ref value);
        }
    }
}