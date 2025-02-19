iamai-core
----------

iamai-core is the core "brain" library for all iamai projects. It provides the core functionality and interfaces for building, evaluating, and running LLM (Large Language Model) and speech-related tasks.

Overview
--------
This repository includes:
- core: Contains the main executable (iamai-core) and a shared library (iamai-core-lib) that serve as the foundation for iamai-related applications.
- test-ggml: Example/test code showcasing usage of ggml (https://github.com/ggerganov/ggml) for time series datasets (like M4).
- test-whisper: Example/test code demonstrating usage of whisper.cpp (https://github.com/ggerganov/whisper.cpp) for speech-to-text tasks.

It also integrates the following submodules:
- whisper.cpp: For speech recognition, specifically Whisper models.
- llama.cpp: For LLM tasks, specifically LLaMA model variants.
- ggml: For efficient tensor operations in CPU/GPU (used by llama.cpp, whisper.cpp, and custom code).

Requirements
------------
1. Git (to clone the repository and submodules)
2. CMake >= 3.15
3. C++ Compiler supporting C++17
4. CUDA (Version 12 recommended) if you want GPU acceleration
   - You should have the environment variable CUDA_PATH set to your CUDA installation on Windows.
   - On Windows, the build scripts will attempt to copy CUDA DLLs (cudart64_12.dll, cublas64_12.dll, cublasLt64_12.dll) to the output directory automatically.

Cloning
-------
Make sure to clone the repository with its submodules:

    git clone --recurse-submodules https://github.com/yourusername/iamai-core.git
    cd iamai-core
    # If you forgot --recurse-submodules, run:
    git submodule update --init --recursive

Building
--------
The project uses multiple CMake configurations for different modules and tests. The root CMakeLists.txt orchestrates everything. Below is a typical out-of-source build flow:

    mkdir build
    cd build
    cmake ..
    cmake --build .

Notable CMake Targets
---------------------
1. iamai-core (executable)
   - Builds the main iamai-core application in core/.
   - By default, will copy required CUDA DLLs on Windows if found.

2. iamai-core-lib (shared library)
   - A shared library that exposes the core functionality.

3. test-ggml
   - Example binary in test-ggml/ that uses ggml for time-series forecasting tasks.

4. train_monthly and eval_monthly
   - Additional executables in test-ggml/ for training and evaluation of monthly time-series data (M4 dataset examples).

5. test-whisper
   - Example binary in test-whisper/ that demonstrates using whisper.cpp for speech-to-text processing.

Windows-Specific Considerations
-------------------------------
- Copying CUDA DLLs:
  The CMake scripts define custom targets (e.g., copy_cuda_dlls, copy_cuda_dlls_whisper) to automatically copy CUDA DLLs into the binary directories. Ensure your CUDA_PATH environment variable points to the correct CUDA installation (e.g., C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.0).

- Required Libraries:
  Some Windows system libraries like Shell32 and Ole32 are linked. This should be automatic on most standard toolchains.

Running
-------
After a successful build, the binaries will typically reside in:
- build/bin (for core/ targets)
- build/bin-ggml (for test-ggml targets)
- build/bin (for test-whisper targets)

The exact paths are determined by the CMake RUNTIME_OUTPUT_DIRECTORY settings.

You can run:

    ./bin/iamai-core

(or .\bin\iamai-core.exe on Windows) to launch the main core executable.

For the test projects:

    ./bin-ggml/test-ggml
    ./bin-ggml/train_monthly
    ./bin-ggml/eval_monthly
    ./bin/test-whisper

(Note: Adjust paths for your OS if necessary.)

Customizing the Build
---------------------
- Enable/Disable GPU Acceleration:
  By default, CUDA acceleration is enabled (GGML_CUDA=ON for ggml, and WHISPER_OPENBLAS=OFF for whisper.cpp).
  If you want a CPU-only build, you can manually edit the relevant CMakeLists.txt or pass flags to CMake:

      cmake -DGGML_CUDA=OFF -DWHISPER_OPENBLAS=OFF ..

- Additional Flags:
  - -DCMAKE_BUILD_TYPE=Release or Debug to switch build types.
  - -G "Visual Studio 17 2022" (or your specific generator) for Windows builds using MSVC.

Contributing
------------
1. Fork and clone this repository.
2. Create a new branch from main.
3. Make your changes and test thoroughly.
4. Submit a Pull Request.

License
-------
MIT
