# Terminal_GTKmm

Simple GTKmm Terminal for experimentation.

## Features

* **Multi-Language Command Execution:**
    * Execute **Bash** scripts/commands.
    * Execute **Python** scripts/commands.
    * Execute **Lua** scripts/commands.

* **Interactive GTKmm 4.0 Interface:**
    * Dedicated input and output areas.
    * File Operations.

## Prerequisites

To build and run this project, you need the following:

* **CMake** (version 3.25 or higher)
* **C++ Compiler** (supporting C++23 standard, e.g., GCC, Clang)
* **GTKmm 4.0 Development Libraries:**
    * On **Debian/Ubuntu**: `sudo apt install libgtkmm-4.0-dev`
* **Python 3 Development Libraries:**
    * On **Debian/Ubuntu**: `sudo apt install python3-dev`
* **Lua Development Libraries:**
    * On **Debian/Ubuntu**: `sudo apt install liblua5.3-dev` (or `liblua5.4-dev` depending on your system's Lua version)

## Building the Project

1.  **Clone the repository:**
    ```bash
    git clone <url_of_this_repository>
    cd Terminal_GTKmm # Where is CMakeLists.txt
    ```

2.  **Create a build directory and navigate into it:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake to configure the project:**
    ```bash
    cmake ..
    ```
    If all prerequisites are met, CMake will find the necessary libraries and generate the build files. You should see output similar to:
    ```
    -- GTKmm found:
    --   Directory: ...
    --   Libraries: ...
    -- Python3 found:
    --   Directory: ...
    --   Libraries: ...
    -- Lua found:
    --   Directory: ...
    --   Libraries: ...
    ```
    If any library is not found, CMake will report a fatal error. Ensure you have all required development packages installed.

4.  **Build the application:**
    ```bash
    cmake --build .
    ```
    This will compile the source code and create the executable.

### Running the Application

After a successful build, you can run the executable from the `build` directory:

```bash
./TerminalApp # Program name in CMakeLists.txt
```

## References

[GTKmm](https://gtkmm.org/en/) : C++ Interfaces for GTK+ and GNOME.</br>
[Python](https://docs.python.org/3/extending/embedding.html) : Embedding Python in Another Application.</br>
[Lua](https://www.lua.org/docs.html) : Lua Documentation.</br>
