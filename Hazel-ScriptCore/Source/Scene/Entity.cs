using System;
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
                InternalCalls.TransformComponent_GetPosition(Id, out var position);
                return position;
            }
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set => InternalCalls.TransformComponent_SetPosition(Id, ref value);
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return InternalCalls.Entity_HasComponent(Id, typeof(T));
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T { Entity = this };
            // TODO cache
            return component;
        }

        public Entity FindEntityByName(string name)
        {
            ulong entityId = InternalCalls.Entity_FindEntityByName(name);
            return entityId == 0 ? null : new Entity(entityId);
        }

        public T As<T>() where T : Entity
        {
            object instance = InternalCalls.GetScriptInstance(Id);
            return instance as T;
        }
    }
}