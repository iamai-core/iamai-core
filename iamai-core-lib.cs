using System;
using System.Runtime.InteropServices;
using System.Text;

public class AI : IDisposable
{
    // Native library imports
    private const string LibName = "iamai-core";

    [DllImport(LibName)]
    private static extern IntPtr Init(string modelPath);

    [DllImport(LibName)]
    private static extern bool Generate(IntPtr ctx, string prompt, StringBuilder output, int outputSize);

    [DllImport(LibName)]
    private static extern void SetMaxTokens(IntPtr ctx, int maxTokens);

    [DllImport(LibName)]
    private static extern void SetThreads(IntPtr ctx, int nThreads);

    [DllImport(LibName)]
    private static extern void SetBatchSize(IntPtr ctx, int batchSize);

    [DllImport(LibName)]
    private static extern void Free(IntPtr ctx);

    private IntPtr _ctx;
    private bool _disposed;

    public AI(string modelPath)
    {
        _ctx = Init(modelPath);
        if (_ctx == IntPtr.Zero)
        {
            throw new Exception("Failed to initialize model");
        }
    }

    public string Generate(string prompt, int maxLength = 4096)
    {
        var output = new StringBuilder(maxLength);
        if (!Generate(_ctx, prompt, output, maxLength))
        {
            throw new Exception("Generation failed");
        }
        return output.ToString();
    }

    public void SetMaxTokens(int maxTokens)
    {
        SetMaxTokens(_ctx, maxTokens);
    }

    public void SetThreads(int nThreads)
    {
        SetThreads(_ctx, nThreads);
    }

    public void SetBatchSize(int batchSize)
    {
        SetBatchSize(_ctx, batchSize);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!_disposed)
        {
            if (_ctx != IntPtr.Zero)
            {
                Free(_ctx);
                _ctx = IntPtr.Zero;
            }
            _disposed = true;
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

// Example usage:
class Program
{
    static void Main()
    {
        using (var ai = new AI("path/to/model.gguf"))
        {
            ai.SetMaxTokens(256);
            string response = ai.Generate("Tell me a story about a robot.");
            Console.WriteLine(response);
        }
    }
}
