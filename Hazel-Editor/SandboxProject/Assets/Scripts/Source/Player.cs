using Hazel;

namespace Sandbox
{
    public class Player : Entity
    {
        private RigidBody2DComponent _rigidBody;

        private void OnCreate()
        {
            _rigidBody = GetComponent<RigidBody2DComponent>();
            GetComponent<SpriteRendererComponent>().Color = Color.Cyan;
        }

        private void OnUpdate(float ts)
        {
            const float speed = 20.0f;
            var velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.W))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.S))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X -= 1.0f;
            else if (Input.IsKeyDown(KeyCode.D))
                velocity.X += 1.0f;

            velocity *= speed;
            _rigidBody.ApplyLinearImpulse(velocity);
        }
    }
}
