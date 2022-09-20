using Hazel;

namespace Sandbox
{
    public class Camera : Entity
    {
        public float DistanceFromPlayer = 15.0f;

        private Player _player;
        
        private void OnCreate()
        {
            Position = new Vector3(Position.XY, DistanceFromPlayer);
            var playerEntity = FindEntityByName("Player");
            if (playerEntity != null)
                _player = playerEntity.As<Player>();
        }
        
        private void OnUpdate(float ts)
        {
            if (_player != null)
                Position = new Vector3(_player.Position.XY, DistanceFromPlayer);
            
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
