from ctypes import *
import os

# Determine library name based on platform
if os.name == 'nt':
    lib_name = 'iamai-core.dll'
elif os.name == 'posix':
    lib_name = 'libiamai-core.so'
else:
    lib_name = 'libiamai-core.dylib'

# Load the library
_lib = cdll.LoadLibrary(lib_name)

# Configure function signatures
_lib.Init.argtypes = [c_char_p]
_lib.Init.restype = c_void_p

_lib.Generate.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
_lib.Generate.restype = c_bool

_lib.SetMaxTokens.argtypes = [c_void_p, c_int]
_lib.SetMaxTokens.restype = None

_lib.SetThreads.argtypes = [c_void_p, c_int]
_lib.SetThreads.restype = None

_lib.SetBatchSize.argtypes = [c_void_p, c_int]
_lib.SetBatchSize.restype = None

_lib.Free.argtypes = [c_void_p]
_lib.Free.restype = None

class AI:
    def __init__(self, model_path):
        encoded_path = model_path.encode('utf-8')
        self.ctx = _lib.Init(encoded_path)
        if not self.ctx:
            raise RuntimeError("Failed to initialize model")

    def generate(self, prompt, max_length=4096):
        output = create_string_buffer(max_length)
        success = _lib.Generate(self.ctx, prompt.encode('utf-8'), output, max_length)
        if not success:
            raise RuntimeError("Generation failed")
        return output.value.decode('utf-8')

    def set_max_tokens(self, max_tokens):
        _lib.SetMaxTokens(self.ctx, max_tokens)

    def set_threads(self, n_threads):
        _lib.SetThreads(self.ctx, n_threads)

    def set_batch_size(self, batch_size):
        _lib.SetBatchSize(self.ctx, batch_size)

    def __del__(self):
        if hasattr(self, 'ctx'):
            _lib.Free(self.ctx)

# Example usage:
if __name__ == "__main__":
    ai = AI("path/to/model.gguf")
    ai.set_max_tokens(256)
    response = ai.generate("Tell me a story about a robot.")
    print(response)
