using System.Runtime.CompilerServices;

namespace HazelEngine
{
	public static class Debug
	{
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void LogDebug_Native(string message);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void LogInfo_Native(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void LogWarn_Native(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        private static extern void LogError_Native(string message);

        public static void Log(string message)
        {
            LogDebug_Native(message);
        }

        public static void LogInfo(string message)
        {
            LogInfo_Native(message);
        }

        public static void LogWarn(string message)
        {
            LogWarn_Native(message);
        }

        public static void LogError(string message)
        {
            LogError_Native(message);
        }
    }
}