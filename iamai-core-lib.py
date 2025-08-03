from ctypes import *
import os
from pathlib import Path

# Get script directory and construct relative path to DLL directory
script_dir = os.path.dirname(os.path.abspath(__file__))
dll_dir = os.path.join(script_dir, "..", "build", "bin", "Debug")
os.add_dll_directory(dll_dir)
LIBRARY_PATH = os.path.join(dll_dir, "iamai-core.dll")

class AI:
    def __init__(self, model_path, config=None):
        # Load the library using the relative path
        self.lib = cdll.LoadLibrary(LIBRARY_PATH)

        if config is None:
            # Use simple Init function
            self.lib.Init.argtypes = [c_char_p]
            self.lib.Init.restype = c_void_p

            # Initialize the model with simple init
            encoded_path = model_path.encode('utf-8')
            self.ctx = self.lib.Init(encoded_path)
        else:
            # Use FullInit function with configuration
            self.lib.FullInit.argtypes = [c_char_p, c_int, c_int, c_int, c_int, c_int, c_float, c_float, c_uint32]
            self.lib.FullInit.restype = c_void_p

            # Initialize with full configuration
            encoded_path = model_path.encode('utf-8')
            self.ctx = self.lib.FullInit(
                encoded_path,
                config.get('max_tokens', 256),
                config.get('batch', 64),
                config.get('ctx_size', 2048),
                config.get('threads', 8),
                config.get('top_k', 50),
                config.get('top_p', 0.9),
                config.get('temperature', 0.5),
                config.get('seed', 42)
            )

        if not self.ctx:
            raise RuntimeError("Failed to initialize model")

        # Configure other function signatures
        self.lib.Generate.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
        self.lib.Generate.restype = c_bool
        self.lib.SetMaxTokens.argtypes = [c_void_p, c_int]
        self.lib.SetMaxTokens.restype = None
        self.lib.SetPromptFormat.argtypes = [c_void_p, c_char_p]
        self.lib.SetPromptFormat.restype = None
        self.lib.ClearPromptFormat.argtypes = [c_void_p]
        self.lib.ClearPromptFormat.restype = None
        self.lib.Free.argtypes = [c_void_p]
        self.lib.Free.restype = None

    def generate(self, prompt, max_length=4096):
        output = create_string_buffer(max_length)
        success = self.lib.Generate(self.ctx, prompt.encode('utf-8'), output, max_length)
        if not success:
            raise RuntimeError("Generation failed")
        return output.value.decode('utf-8')

    def set_max_tokens(self, max_tokens):
        self.lib.SetMaxTokens(self.ctx, max_tokens)

    def set_prompt_format(self, format_string):
        self.lib.SetPromptFormat(self.ctx, format_string.encode('utf-8'))

    def clear_prompt_format(self):
        self.lib.ClearPromptFormat(self.ctx)

    def __del__(self):
        if hasattr(self, 'ctx') and self.ctx:
            self.lib.Free(self.ctx)

if __name__ == "__main__":
    try:
        # Example usage with simple initialization
        ai = AI("./models/Llama-3.2-1B-Instruct-Q4_K_M.gguf")
        ai.set_max_tokens(256)
        response = ai.generate("Tell me a story about a robot.")
        print("Simple init response:", response)

        # Example usage with full configuration
        config = {
            'max_tokens': 128,
            'batch': 32,
            'ctx_size': 1024,
            'threads': 4,
            'top_k': 40,
            'top_p': 0.8,
            'temperature': 0.7,
            'seed': 1337
        }
        ai_configured = AI("./models/llama-3.2-1b-instruct-q4_k_m.gguf", config)

        # Set a prompt format
        ai_configured.set_prompt_format("Human: {prompt}\nAssistant: ")
        response2 = ai_configured.generate("What is the meaning of life?")
        print("Configured response:", response2)

    except Exception as e:
        print(f"Error: {e}")
