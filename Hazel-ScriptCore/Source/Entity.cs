using System;

namespace Hazel
{
    public class Entity
    {
        public float FloatVar { get; set; }

        public Entity()
        {
            Console.WriteLine("InternalCalls Constructor!");
            Log("Test Log From C#", 42069);
            
            var position = new Vector3(3, 5, 6);
            Log(position);
        }

        public void PrintMessage()
        {
            Console.WriteLine("Hello World from C#!");
        }

        public void PrintInt(int value)
        {
            Console.WriteLine($"C# says: {value}");
        }

        public void PrintInts(int value1, int value2)
        {
            Console.WriteLine($"C# says: {value1} and {value2}");
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine($"C# says: {message}");
        }

        private void Log(string text, int parameter)
        {
            InternalCalls.NativeLog(text, parameter);
        }

        private void Log(Vector3 parameter)
        {
            InternalCalls.NativeLog_Vector(ref parameter);
        }
    }
}