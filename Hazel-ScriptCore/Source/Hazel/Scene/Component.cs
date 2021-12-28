using System;
using System.Runtime.CompilerServices;

namespace HazelEngine
{
    public abstract class Component
    {
        public Entity Entity { get; set; }
    }

    public class TagComponent : Component
    {
        public string Tag
        {
            get => GetTag_Native(Entity.Id);
            set => SetTag_Native(Entity.Id, value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetTag_Native(ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTag_Native(ulong entityId, string tag);
    }

    public class TransformComponent : Component
    {
        public Transform Transform
        {
            get
            {
                GetTransform_Native(Entity.Id, out Transform result);
                return result;
            }
            set => SetTransform_Native(Entity.Id, ref value);
        }

        public Transform WorldPosition
        {
            get
            {
                GetWorldSpaceTransform_Native(Entity.Id, out Transform result);
                return result;
            }
        }

        public Vector3 Position
        {
            get
            {
                GetPosition_Native(Entity.Id, out Vector3 result);
                return result;
            }
            set => SetPosition_Native(Entity.Id, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                GetRotation_Native(Entity.Id, out Vector3 result);
                return result;
            }
            set => SetRotation_Native(Entity.Id, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                GetScale_Native(Entity.Id, out Vector3 result);
                return result;
            }
            set => SetScale_Native(Entity.Id, ref value);
        }
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetTransform_Native(ulong entityId, out Transform outTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTransform_Native(ulong entityId, ref Transform inTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Native(ulong entityId, out Vector3 outTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetPosition_Native(ulong entityId, ref Vector3 inTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetRotation_Native(ulong entityId, out Vector3 outRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRotation_Native(ulong entityId, ref Vector3 inRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Native(ulong entityId, out Vector3 outScale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(ulong entityId, ref Vector3 inScale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetWorldSpaceTransform_Native(ulong entityId, out Transform outTransform);
    }

    public class ScriptComponent : Component
    {
        public object Instance => GetInstance_Native(Entity.Id);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object GetInstance_Native(ulong entityId);
    }

    public enum RigidBody2DBodyType
    {
        Static, Dynamic, Kinematic
    }

    public class RigidBody2DComponent : Component
    {
        // TODO
    }
}