﻿using Hazel;

namespace Sandbox
{
    public class Camera : Entity
    {
        private void OnUpdate(float ts)
        {
            const float speed = 5.0f;
            var velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.Up))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.Down))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.Left))
                velocity.X -= 1.0f;
            else if (Input.IsKeyDown(KeyCode.Right))
                velocity.X += 1.0f;

            velocity *= speed * ts;
            Position += velocity;
        }
    }
}
