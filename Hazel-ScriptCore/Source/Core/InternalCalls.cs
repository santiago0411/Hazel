using System;
using System.Runtime.CompilerServices;

namespace Hazel
{
    internal static class InternalCalls
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong entityId, Type componentType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_FindEntityByName(string name);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern object GetScriptInstance(ulong entityId);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetPosition(ulong entityId, out Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetPosition(ulong entityId, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityId, out Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityId, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityId, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityId, ref Vector3 scale);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidBody2DComponent_ApplyLinearImpulse(ulong entityId, ref Vector2 impulse, ref Vector2 point, bool wake);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidBody2DComponent_ApplyLinearImpulseToCenter(ulong entityId, ref Vector2 impulse, bool wake);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern RigidBody2DComponent.BodyType RigidBody2DComponent_GetType(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidBody2DComponent_SetType(ulong entityId, RigidBody2DComponent.BodyType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void RigidBody2DComponent_GetLinearVelocity(ulong entityId, out Vector2 linearVelocity);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_GetColor(ulong entityId, out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetColor(ulong entityId, ref Color color);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string TextComponent_GetText(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetText(ulong entityId, string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float TextComponent_GetKerning(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetKerning(ulong entityId, float kerning);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float TextComponent_GetLineSpacing(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetLineSpacing(ulong entityId, float lineSpacing);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_GetColor(ulong entityId, out Color color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetColor(ulong entityId, ref Color color);


        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode keyCode);
    }
}
