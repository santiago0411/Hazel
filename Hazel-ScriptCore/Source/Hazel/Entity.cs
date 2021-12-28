using System;
using System.Runtime.CompilerServices;

namespace HazelEngine
{
    public class Entity
    {
        public ulong Id { get; }
        public string Tag => GetComponent<TagComponent>().Tag;

        public Vector3 Position
        {
            get => GetComponent<TransformComponent>().Position;
            set => GetComponent<TransformComponent>().Position = value;
        }

        public Vector3 Rotation
        {
            get => GetComponent<TransformComponent>().Rotation;
            set => GetComponent<TransformComponent>().Rotation = value;
        }
        
        public Vector3 Scale
        {
            get => GetComponent<TransformComponent>().Scale;
            set => GetComponent<TransformComponent>().Scale = value;
        }
        
        protected Entity()
        {
            Id = 0;
        }

        internal Entity(ulong id)
        {
            Id = id;
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(Id, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component
        {
            return HasComponent_Native(Id, typeof(T));
        }

        public bool HasComponent(Type type)
        {
            return HasComponent_Native(Id, type);
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T
                {
                    Entity = this
                };
                return component;
            }

            return null;
        }

        public bool Is<T>() where T : Entity
        {
            var sc = GetComponent<ScriptComponent>();
            var instance = sc?.Instance;
            return instance is T;
        }

        public T As<T>() where T : Entity
        {
            var sc = GetComponent<ScriptComponent>();
            var instance = sc?.Instance;
            return instance as T;
        }
        
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong CreateEntity_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong Instantiate_Native(ulong entityId, ulong prefabId);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong InstantiateWithPosition_Native(ulong entityId, ulong prefabId, ref Vector3 position);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong InstantiateWithTransform_Native(ulong entityId, ulong prefabId, ref Vector3 position, ref Vector3 rotation, ref Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong DestroyEntity_Native(ulong entityId);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(ulong entityId, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityId, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong FindEntityByTag_Native(string tag);
    }
}