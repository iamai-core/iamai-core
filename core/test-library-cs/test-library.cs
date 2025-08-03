using System.Runtime.InteropServices;
using System.Text;

namespace iamai_core_lib
{
    public class AIConfig
    {
        public int MaxTokens { get; set; } = 256;
        public int Batch { get; set; } = 64;
        public int ContextSize { get; set; } = 2048;
        public int Threads { get; set; } = 8;
        public int TopK { get; set; } = 50;
        public float TopP { get; set; } = 0.9f;
        public float Temperature { get; set; } = 0.5f;
        public uint Seed { get; set; } = 42;
    }

    public class AI : IDisposable
    {
        private IntPtr ctx;
        private IntPtr dllHandle;
        private bool disposed = false;
        private const string DLL_PATH = "iamai-core.dll";

        // Win32 API functions
        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll")]
        private static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern bool SetDllDirectory(string lpPathName);

        // Function delegate types
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr InitDelegate([MarshalAs(UnmanagedType.LPStr)] string modelPath);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr FullInitDelegate(
            [MarshalAs(UnmanagedType.LPStr)] string modelPath,
            int maxTokens, int batch, int contextSize, int threads,
            int topK, float topP, float temperature, uint seed);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate bool GenerateDelegate(IntPtr context, [MarshalAs(UnmanagedType.LPStr)] string prompt,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder output, int maxLength);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetMaxTokensDelegate(IntPtr context, int maxTokens);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void SetPromptFormatDelegate(IntPtr context, [MarshalAs(UnmanagedType.LPStr)] string format);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ClearPromptFormatDelegate(IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void FreeDelegate(IntPtr context);

        // Function delegates
        private InitDelegate _init;
        private FullInitDelegate _fullInit;
        private GenerateDelegate _generate;
        private SetMaxTokensDelegate _setMaxTokens;
        private SetPromptFormatDelegate _setPromptFormat;
        private ClearPromptFormatDelegate _clearPromptFormat;
        private FreeDelegate _free;

        public AI(string modelName, AIConfig config = null)
        {
            // Get the current directory and navigate to the DLL location
            string exePath = Directory.GetCurrentDirectory();
            string projectRoot = Path.GetFullPath(Path.Combine(exePath, "..", "..", "..", "..", ".."));
            string dllDirectory = Path.Combine(projectRoot, "build", "bin", "Debug");
            string dllPath = Path.Combine(dllDirectory, DLL_PATH);
            string modelPath = Path.Combine(projectRoot, "models", modelName);

            if (!Directory.Exists(dllDirectory))
            {
                throw new DirectoryNotFoundException($"DLL directory not found: {dllDirectory}");
            }

            Console.WriteLine($"Loading DLL from: {dllPath}");
            SetDllDirectory(dllDirectory);

            // Load the DLL
            dllHandle = LoadLibrary(dllPath);
            if (dllHandle == IntPtr.Zero)
            {
                int errorCode = Marshal.GetLastWin32Error();
                throw new InvalidOperationException($"Failed to load DLL. Error code: {errorCode}");
            }

            // Get function pointers
            _init = GetDelegate<InitDelegate>("Init");
            _fullInit = GetDelegate<FullInitDelegate>("FullInit");
            _generate = GetDelegate<GenerateDelegate>("Generate");
            _setMaxTokens = GetDelegate<SetMaxTokensDelegate>("SetMaxTokens");
            _setPromptFormat = GetDelegate<SetPromptFormatDelegate>("SetPromptFormat");
            _clearPromptFormat = GetDelegate<ClearPromptFormatDelegate>("ClearPromptFormat");
            _free = GetDelegate<FreeDelegate>("Free");

            // Initialize the model
            if (config == null)
            {
                // Use simple initialization
                ctx = _init(modelPath);
            }
            else
            {
                // Use full initialization with configuration
                ctx = _fullInit(
                    modelPath,
                    config.MaxTokens,
                    config.Batch,
                    config.ContextSize,
                    config.Threads,
                    config.TopK,
                    config.TopP,
                    config.Temperature,
                    config.Seed
                );
            }

            if (ctx == IntPtr.Zero)
            {
                throw new InvalidOperationException("Failed to initialize model");
            }
        }

        private T GetDelegate<T>(string procName) where T : Delegate
        {
            IntPtr procAddress = GetProcAddress(dllHandle, procName);
            if (procAddress == IntPtr.Zero)
            {
                int errorCode = Marshal.GetLastWin32Error();
                throw new InvalidOperationException(
                    $"Failed to get proc address for {procName}. Error code: {errorCode}");
            }
            return Marshal.GetDelegateForFunctionPointer<T>(procAddress);
        }

        public string Generate(string prompt, int maxLength = 4096)
        {
            StringBuilder output = new StringBuilder(maxLength);
            bool success = _generate(ctx, prompt, output, maxLength);

            if (!success)
            {
                throw new InvalidOperationException("Generation failed");
            }

            return output.ToString();
        }

        public void SetMaxTokens(int maxTokens)
        {
            _setMaxTokens(ctx, maxTokens);
        }

        public void SetPromptFormat(string format)
        {
            _setPromptFormat(ctx, format);
        }

        public void ClearPromptFormat()
        {
            _clearPromptFormat(ctx);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (ctx != IntPtr.Zero)
                {
                    _free(ctx);
                    ctx = IntPtr.Zero;
                }
                if (dllHandle != IntPtr.Zero)
                {
                    FreeLibrary(dllHandle);
                    dllHandle = IntPtr.Zero;
                }
                disposed = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        ~AI()
        {
            Dispose(false);
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                // Example with simple initialization
                using (var ai = new AI(@"Llama-3.2-1B-Instruct-Q4_K_M.gguf"))
                {
                    ai.SetMaxTokens(256);
                    string response = ai.Generate("Tell me a story about a robot.");
                    Console.WriteLine("Simple init response:");
                    Console.WriteLine(response);
                }

                // Example with full configuration
                var config = new AIConfig
                {
                    MaxTokens = 128,
                    Batch = 32,
                    ContextSize = 1024,
                    Threads = 4,
                    TopK = 40,
                    TopP = 0.8f,
                    Temperature = 0.7f,
                    Seed = 1337
                };

                using (var ai = new AI(@"Llama-3.2-1B-Instruct-Q4_K_M.gguf", config))
                {
                    ai.SetPromptFormat("Human: {prompt}\nAssistant: ");
                    string response = ai.Generate("What is the meaning of life?");
                    Console.WriteLine("\nConfigured response:");
                    Console.WriteLine(response);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"Stack trace: {ex.StackTrace}");
            }
        }
    }
}
