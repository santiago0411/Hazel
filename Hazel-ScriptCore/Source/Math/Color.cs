using System;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Hazel
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color : IEquatable<Color>, IFormattable
    {
        public float R;
        public float G;
        public float B;
        public float A;

        public Color(float r, float g, float b, float a)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }

        public Color(float r, float g, float b)
        {
            R = r;
            G = g;
            B = b;
            A = 1.0f;
        }

        public bool Equals(Color other)
        {
            return R.Equals(other.R) && G.Equals(other.G) && B.Equals(other.B) && A.Equals(other.A);
        }

        public override bool Equals(object obj)
        {
            return obj is Color other && Equals(other);
        }

        public override string ToString()
        {
            return ToString(null);
        }

        public string ToString(string format, IFormatProvider formatProvider = null)
        {
            if (string.IsNullOrEmpty(format))
                format = "F3";
            if (formatProvider is null)
                formatProvider = CultureInfo.InvariantCulture.NumberFormat;
            return $"RGBA({R.ToString(format, formatProvider)}, {G.ToString(format, formatProvider)}, {B.ToString(format, formatProvider)}, {A.ToString(format, formatProvider)})";
        }

        public override int GetHashCode()
        {
            return R.GetHashCode() ^ (G.GetHashCode() << 2) ^ (B.GetHashCode() >> 2) ^ (A.GetHashCode() >> 1);
        }

        public static bool operator ==(Color lhs, Color rhs)
        {
            return lhs.Equals(rhs);
        }

        public static bool operator !=(Color lhs, Color rhs)
        {
            return !lhs.Equals(rhs);
        }

        public static Color Red { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(1f, 0f, 0f, 1f); }
        public static Color Green { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(0f, 1f, 0f, 1f); }
        public static Color Blue { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(0f, 0f, 1f, 1f); }
        public static Color White { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(1f, 1f, 1f, 1f); }
        public static Color Black { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(0f, 0f, 0f, 1f); }
        public static Color Yellow { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(1f, 235f / 255f, 4f / 255f, 1f); }
        public static Color Cyan { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(0f, 1f, 1f, 1f); }
        public static Color Magenta { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(1f, 0f, 1f, 1f); }
        public static Color Grey { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(.5f, .5f, .5f, 1f); }
        public static Color Clear { [MethodImpl(MethodImplOptions.AggressiveInlining)] get => new Color(0f, 0f, 0f, 0f); }
    }
}
