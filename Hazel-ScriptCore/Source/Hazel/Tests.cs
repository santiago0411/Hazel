namespace HazelEngine
{
    public static class Tests
    {
        public static void PrintAllLogs()
        {
            Debug.Log("Debug test log from mono C#");
            Debug.LogInfo("Info test log from mono C#");
            Debug.LogWarn("Warn test log from mono C#");
            Debug.LogError("Error test log from mono C#");
        }
    }
}