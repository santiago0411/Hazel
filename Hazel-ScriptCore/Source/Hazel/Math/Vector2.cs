using System;
using System.Globalization;
using System.Runtime.InteropServices;

namespace HazelEngine
{
    [StructLayout(LayoutKind.Sequential)]
	public struct Vector2 : IEquatable<Vector2>, IFormattable
    {
        public static readonly Vector2 Zero = new Vector2(0, 0);
        public static readonly Vector2 One = new Vector2(1, 1);
        public static readonly Vector2 Right = new Vector2(1, 0);
        public static readonly Vector2 Left = new Vector2(-1, 0);
        public static readonly Vector2 Up = new Vector2(0, 1);
        public static readonly Vector2 Down = new Vector2(0, -1);
        
        public float X;
        public float Y;

        public Vector2(float scalar)
        {
            X = Y = scalar;
        }
        
		public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }
        
        public float Length()
        {
            return (float) Math.Sqrt(X * X + Y * Y);
        }
        
        public Vector2 Normalized()
        {
            float length = Length();
            float x = X;
            float y = Y;

            if (length > 0.0f)
            {
                x /= length;
                y /= length;
            }

            return new Vector2(x, y);
        }
        
        public float Distance(Vector2 other)
        {
            return (float) Math.Sqrt(Math.Pow(other.X - X, 2) +
                                     Math.Pow(other.Y - Y, 2));
        }
        
        public static float Distance(Vector2 p1, Vector2 p2)
        {
            return (float) Math.Sqrt(Math.Pow(p1.X - p2.X, 2) +
                                     Math.Pow(p1.Y - p2.Y, 2));
        }
        
		public static Vector2 Lerp(Vector2 p1, Vector2 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;
			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}
		
		public static Vector2 Cos(Vector2 vector)
		{
			return new Vector2((float) Math.Cos(vector.X), (float) Math.Cos(vector.Y));
		}

		public static Vector2 Sin(Vector2 vector)
		{
			return new Vector2((float) Math.Sin(vector.X), (float) Math.Sin(vector.Y));
		}

		public static Vector2 operator *(Vector2 left, float scalar)
		{
			return new Vector2(left.X * scalar, left.Y * scalar);
		}

		public static Vector2 operator *(float scalar, Vector2 right)
		{
			return new Vector2(scalar * right.X, scalar * right.Y);
		}

		public static Vector2 operator *(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X * right.X, left.Y * right.Y);
		}

		public static Vector2 operator /(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X / right.X, left.Y / right.Y);
		}

		public static Vector2 operator /(Vector2 left, float scalar)
		{
			return new Vector2(left.X / scalar, left.Y / scalar);
		}

		public static Vector2 operator /(float scalar, Vector2 right)
		{
			return new Vector2(scalar/ right.X, scalar/ right.Y);
		}

		public static Vector2 operator +(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X + right.X, left.Y + right.Y);
		}

		public static Vector2 operator +(Vector2 left, float right)
		{
			return new Vector2(left.X + right, left.Y + right);
		}

		public static Vector2 operator -(Vector2 left, Vector2 right)
		{
			return new Vector2(left.X - right.X, left.Y - right.Y);
		}

		public static Vector2 operator -(Vector2 vector)
		{
			return new Vector2(-vector.X, -vector.Y);
		}

        public override bool Equals(object other)
        {
            return other is Vector2 otherVec && Equals(otherVec);
        }

        public bool Equals(Vector2 other)
        {
            return other.X == X && other.Y == Y;
        }

        public override string ToString()
        {
            return ToString(null, CultureInfo.InvariantCulture.NumberFormat);
        }

        public string ToString(string format)
        {
            return ToString(format, CultureInfo.InvariantCulture.NumberFormat);
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
                format = "F1";

            return $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)})";
        }

        public override int GetHashCode()
        {
            return X.GetHashCode() ^ Y.GetHashCode() << 2;
        }
	}
}