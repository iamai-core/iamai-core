from ctypes import *
import os
from pathlib import Path
from ctypes import *

# Get script directory and construct relative path to DLL directory
script_dir = os.path.dirname(os.path.abspath(__file__))
dll_dir = os.path.join(script_dir, "build", "bin", "Debug")
os.add_dll_directory(dll_dir)
LIBRARY_PATH = os.path.join(dll_dir, "iamai-core.dll")

class AI:
    def __init__(self, model_path):
        # Load the library using the relative path
        self.lib = cdll.LoadLibrary(LIBRARY_PATH)
        
        # Configure function signatures
        self.lib.Init.argtypes = [c_char_p]
        self.lib.Init.restype = c_void_p
        self.lib.Generate.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
        self.lib.Generate.restype = c_bool
        self.lib.SetMaxTokens.argtypes = [c_void_p, c_int]
        self.lib.SetMaxTokens.restype = None
        self.lib.SetThreads.argtypes = [c_void_p, c_int]
        self.lib.SetThreads.restype = None
        self.lib.SetBatchSize.argtypes = [c_void_p, c_int]
        self.lib.SetBatchSize.restype = None
        self.lib.Free.argtypes = [c_void_p]
        self.lib.Free.restype = None

        # Initialize the model
        encoded_path = model_path.encode('utf-8')
        self.ctx = self.lib.Init(encoded_path)
        if not self.ctx:
            raise RuntimeError("Failed to initialize model")

    def generate(self, prompt, max_length=4096):
        output = create_string_buffer(max_length)
        success = self.lib.Generate(self.ctx, prompt.encode('utf-8'), output, max_length)
        if not success:
            raise RuntimeError("Generation failed")
        return output.value.decode('utf-8')

    def set_max_tokens(self, max_tokens):
        self.lib.SetMaxTokens(self.ctx, max_tokens)

    def set_threads(self, n_threads):
        self.lib.SetThreads(self.ctx, n_threads)

    def set_batch_size(self, batch_size):
        self.lib.SetBatchSize(self.ctx, batch_size)

    def __del__(self):
        if hasattr(self, 'ctx'):
            self.lib.Free(self.ctx)

if __name__ == "__main__":
    try:
        ai = AI("./models/llama-3.2-1b-instruct-q4_k_m.gguf")
        ai.set_max_tokens(256)
        response = ai.generate("Tell me a story about a robot.")
        print(response)
    except Exception as e:
        print(f"Error: {e}")