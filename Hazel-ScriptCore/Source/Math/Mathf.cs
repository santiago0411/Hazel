namespace Hazel
{
    public static class Mathf
    {
        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;
            return value;
        }

        public static float Clamp(int value, int min, int max)
        {
            if (value < min)
                value = min;
            else if (value > max)
                value = max;
            return value;
        }

        public static float Clamp01(float value)
        {
            if (value < 0F)
                return 0F;
            if (value > 1F)
                return 1F;
            return value;
        }

        public static float Lerp(float a, float b, float c)
        {
            return a + (b - a) * Clamp01(c);
        }

        public static float LerpUnclamped(float a, float b, float t)
        {
            return a + (b - a) * t;
        }
    }
}
