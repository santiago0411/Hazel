using System.Runtime.CompilerServices;

namespace Hazel
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Position
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                InternalCalls.TransformComponent_GetPosition(Entity.Id, out var position);
                return position;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.TransformComponent_SetPosition(Entity.Id, ref value);
        }

        public Vector3 Rotation
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.Id, out var rotation);
                return rotation;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.TransformComponent_SetRotation(Entity.Id, ref value);
        }

        public Vector3 Scale
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.Id, out var scale);
                return scale;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.TransformComponent_SetScale(Entity.Id, ref value);
        }
    }

    public class RigidBody2DComponent : Component
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake = true)
        {
            InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(Entity.Id, ref impulse, ref worldPosition, wake);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void ApplyLinearImpulse(Vector2 impulse, bool wake = true)
        {
            InternalCalls.RigidBody2DComponent_ApplyLinearImpulseToCenter(Entity.Id, ref impulse, wake);
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Color Color
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                InternalCalls.SpriteRendererComponent_GetColor(Entity.Id, out var color);
                return color;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.SpriteRendererComponent_SetColor(Entity.Id, ref value);
        }
    }
}
