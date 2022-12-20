using System;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Hazel
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3 : IEquatable<Vector3>, IFormattable
    {
        public float X;
        public float Y;
        public float Z;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector3(float scalar)
        {
            X = Y = Z = scalar;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector3(Vector2 xy, float z)
        {
            X = xy.X;
            Y = xy.Y;
            Z = z;
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector3(float x, Vector2 yz)
        {
            X = x;
            Y = yz.X;
            Z = yz.Y;
        }

        public Vector2 XY
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector2(X, Y);
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }

        public Vector2 XZ
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector2(X, Z);
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set
            {
                X = value.X;
                Z = value.Y;
            }
        }
        
        public Vector2 YZ
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector2(Y, Z);
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            set
            {
                Y = value.X;
                Z = value.Y;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public float LengthSquared()
        {
            return X * X + Y * Y + Z * Z;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public float Length()
        {
            return (float) Math.Sqrt(X * X + Y * Y + Z * Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector3 Normalized()
        {
            float length = Length();
            float x = X;
            float y = Y;
            float z = Z;

            if (length > 0.0f)
            {
                x /= length;
                y /= length;
                z /= length;
            }

            return new Vector3(x, y, z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public float Distance(Vector3 other)
        {
            return (float) Math.Sqrt(Math.Pow(other.X - X, 2) +
                                     Math.Pow(other.Y - Y, 2) +
                                     Math.Pow(other.Z - Z, 2));
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Distance(Vector3 p1, Vector3 p2)
        {
            return (float) Math.Sqrt(Math.Pow(p1.X - p2.X, 2) +
                                     Math.Pow(p1.Y - p2.Y, 2) +
                                     Math.Pow(p1.Z - p2.Z, 2));
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Lerp(Vector3 p1, Vector3 p2, float maxDistanceDelta)
        {
            if (maxDistanceDelta < 0.0f)
                return p1;
            if (maxDistanceDelta > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * maxDistanceDelta);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Cross(Vector3 x, Vector3 y)
        {
            return new Vector3(
                x.Y * y.Z - y.Y * x.Z,
                x.Z * y.X - y.Z * x.X,
                x.X * y.Y - y.X * x.Y
            );
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Cos(Vector3 vector)
        {
            return new Vector3((float) Math.Cos(vector.X), (float) Math.Cos(vector.Y), (float) Math.Cos(vector.Z));
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 Sin(Vector3 vector)
        {
            return new Vector3((float) Math.Sin(vector.X), (float) Math.Sin(vector.Y), (float) Math.Sin(vector.Z));
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static implicit operator Vector2(Vector3 vector)
        {
            return new Vector2(vector.X, vector.Y);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Vector3 left, float scalar)
        {
            return new Vector3(left.X * scalar, left.Y * scalar, left.Z * scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(float scalar, Vector3 right)
        {
            return new Vector3(scalar * right.X, scalar * right.Y, scalar * right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator /(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator /(Vector3 left, float scalar)
        {
            return new Vector3(left.X / scalar, left.Y / scalar, left.Z / scalar);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator /(float scalar, Vector3 right)
        {
            return new Vector3(scalar/ right.X, scalar/ right.Y, scalar/ right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator +(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator +(Vector3 left, float right)
        {
            return new Vector3(left.X + right, left.Y + right, left.Z + right);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator -(Vector3 vector)
        {
            return new Vector3(-vector.X, -vector.Y, -vector.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public override bool Equals(object other)
        {
            return other is Vector3 otherVec && Equals(otherVec);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Equals(Vector3 other)
        {
            return other.X == X && other.Y == Y && other.Z == Z;
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

            return $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)}, {Z.ToString(format, formatProvider)})";
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public override int GetHashCode()
        {
            return X.GetHashCode() ^ (Y.GetHashCode() << 2) ^ (Z.GetHashCode() >> 2);
        }

        public static Vector3 Zero
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(0);
        }

        public static Vector3 One
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(1);
        }

        public static Vector3 Forward
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(0, 0, 1);
        }

        public static Vector3 Back
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(0, 0, -1);
        }

        public static Vector3 Right
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(1, 0, 0);
        }

        public static Vector3 Left
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(-1, 0, 0);
        }

        public static Vector3 Up
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(0, 1, 0);
        }

        public static Vector3 Down
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get => new Vector3(0, -1, 0);
        }
    }
}