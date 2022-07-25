using System;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Hazel
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Quaternion : IEquatable<Quaternion>, IFormattable
    {
        public float W;
        public float X;
        public float Y;
        public float Z;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Quaternion(Vector3 euler)
        {
            Vector3 c = Vector3.Cos(euler * 0.5f);
            Vector3 s = Vector3.Sin(euler * 0.5f);
            
            W = c.X * c.Y * c.Z + s.X * s.Y * s.Z;
            X = s.X * c.Y * c.Z - c.X * s.Y * s.Z;
            Y = c.X * s.Y * c.Z + s.X * c.Y * s.Z;
            Z = c.X * c.Y * s.Z - s.X * s.Y * c.Z;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Quaternion q, Vector3 v)
        {
            Vector3 qv = new Vector3(q.X, q.Y, q.Z);
            Vector3 uv = Vector3.Cross(qv, v);
            Vector3 uuv = Vector3.Cross(qv, uv);
            return v + ((uv * q.W) + uuv) * 2.0f;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public override bool Equals(object other)
        {
            return other is Quaternion otherQuat && Equals(otherQuat);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Equals(Quaternion other)
        {
            return other.W == W && other.X == X && other.Y == Y && other.Z == Z;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Quaternion lhs, Quaternion rhs) => lhs.Equals(rhs);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator !=(Quaternion lhs, Quaternion rhs) => !(lhs == rhs);
        
        public override string ToString()
        {
            return ToString(null, null);
        }

        public string ToString(string format)
        {
            return ToString(format, null);
        }

        public string ToString(string format, IFormatProvider formatProvider)
        {
            if (string.IsNullOrEmpty(format))
                format = "F5";
            if (formatProvider == null)
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            
            return $"({X.ToString(format, formatProvider)}, {Y.ToString(format, formatProvider)}, {Z.ToString(format, formatProvider)}, {W.ToString(format, formatProvider)})";
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public override int GetHashCode()
        {
            return X.GetHashCode() ^ (Y.GetHashCode() << 2) ^ (Z.GetHashCode() >> 2) ^ (W.GetHashCode() >> 1);
        }
    }
}