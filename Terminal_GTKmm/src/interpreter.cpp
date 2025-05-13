/*
 * References:
 *    https://www.gtkmm.org
 *    https://docs.python.org/3/extending/index.html
 *    https://www.lua.org/docs.html
 *
 * Requeriment
 *    libgtkmm-4.0-dev (Linux)
 *    python.h
 *    lua
 */
#include "interpreter.hpp"

#include <Python.h>

#include <lua.hpp>

#include <array>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

const std::vector<std::string> Interpreter::s_names{"", "Bash", "Python",
                                                    "Lua"};

Interpreter::~Interpreter() {
  // Release the Python Interpreter before exiting.
  if (Py_IsInitialized()) {
    Py_FinalizeEx();
  }
}

auto Interpreter::name(int index) -> std::string {
  if (index >= 0 and index < s_names.size()) {
    return s_names.at(index);
  }
  return s_names.at(0);
}

auto Interpreter::execute_command(const std::string_view command,
                                  size_t language_type) -> std::string {

  if (language_type == Languages::BASH) {
    return execute_bash(command);
  } else if (language_type == Languages::PYTHON) {
    return execute_python(command);
  } else if (language_type == Languages::LUA) {
    return execute_lua(command);
  }

  return "Language not supported!";
}

auto Interpreter::execute_bash(const std::string_view command) -> std::string {

  // Custom deleter for FILE
  struct PipeDeleter {
    void operator()(FILE *pipe) {
      if (pipe) {
        pclose(pipe);
      }
    }
  };

  // Custom Smart Pointer Type
  using PipePtr = std::unique_ptr<FILE, PipeDeleter>;

  try {
    // popen : pipe stream to or from a process (Standard C library)
    PipePtr pipe(popen(command.data(), "r"));
    if (!pipe) {
      return "Failed to execute command";
    }

    std::array<char, 128> buffer;
    std::ostringstream result;

    try {
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result << buffer.data();
      }
    } catch (const std::exception &e) {
      return std::string("Error reading command output: ") + e.what();
    }

    auto status = pclose(pipe.release());
    if (status != 0) {
      return std::string("Command execution failed with status " +
                         std::to_string(WEXITSTATUS(status)) +
                         "\nCommand: " + std::string(command));
    }

    // Successfully executed
    return result.str();

  } catch (const std::runtime_error &e) {
    return std::string("Bash execution error: ") + e.what();
  } catch (const std::exception &e) {
    return std::string("Unexpected error in bash execution: ") + e.what();
  } catch (...) {
    return std::string("Unknown error in bash execution\n");
  }
}

auto Interpreter::execute_python(const std::string_view command)
    -> std::string {
  // Initializes the Python interpreter (if not already initialized)
  if (!Py_IsInitialized()) {
    Py_Initialize();
  }

  // Custom deleter for PyObject*
  // Called automatically when the Smart Pointer goes out of scope
  struct PyObjectDeleter {
    void operator()(PyObject *obj) {
      if (obj) {
        Py_DECREF(obj);
      }
    }
  };

  // Custom Smart Pointer Type
  using PyObjectPtr = std::unique_ptr<PyObject, PyObjectDeleter>;

  // Create PyObjectPtr
  auto make_py_object = [](PyObject *obj) -> PyObjectPtr {
    return PyObjectPtr(obj);
  };

  // Creates context dictionary for globals and locals
  PyObject *main_module =
      PyImport_AddModule("__main__"); // No DECREF, it's singleton
  PyObject *main_dict = PyModule_GetDict(main_module);

  // Redirect sys.stdout to capture Python output
  PyObjectPtr sys_module(PyImport_ImportModule("sys"));
  if (!sys_module) {
    PyErr_Print();
    return "Error: Failed to import sys module.\n";
  }

  PyObjectPtr io_module(PyImport_ImportModule("io"));
  if (!io_module) {
    PyErr_Print();
    return "Error: Failed to import io modules.\n";
  }

  PyObjectPtr string_io(PyObject_CallMethod(io_module.get(), "StringIO", NULL));
  if (!string_io) {
    PyErr_Print();
    return "Error: Failed to create StringIO.\n";
  }

  // Don't check for errors: string_io has already been validated.
  PyObject_SetAttrString(sys_module.get(), "stdout", string_io.get());
  PyObject_SetAttrString(sys_module.get(), "stderr", string_io.get());

  // Execute the Python code
  PyObjectPtr py_result(
      PyRun_String(command.data(), Py_file_input, main_dict, main_dict));
  if (!py_result) {
    PyErr_Print();
  }

  // Retrieves the contents of StringIO
  PyObjectPtr output(PyObject_CallMethod(string_io.get(), "getvalue", NULL));
  std::string result;

  if (output && PyUnicode_Check(output.get())) {
    result = PyUnicode_AsUTF8(output.get());
  } else {
    result = "Error: Could not retrieve output.\n";
  }

  return result;
}

auto Interpreter::execute_lua(const std::string_view command) -> std::string {

  // Custom deleter for lua_State*
  struct LuaStateDeleter {
    void operator()(lua_State *L) {
      if (L) {
        lua_close(L);
      }
    }
  };

  // Custom Smart Pointer Type
  using LuaStatePtr = std::unique_ptr<lua_State, LuaStateDeleter>;

  try {
    LuaStatePtr L(luaL_newstate());
    if (!L) {
      return "Error: Failed to create Lua state.\n";
    }

    // Buffer to capture Lua output
    std::ostringstream output;

    lua_pushcfunction(L.get(), [](lua_State *L) -> int {
      int nargs = lua_gettop(L);
      std::ostringstream oss;
      for (int i = 1; i <= nargs; ++i) {
        if (i > 1) {
          oss << "\t";
        }
        if (lua_isstring(L, i)) {
          oss << lua_tostring(L, i);
        } else {
          oss << "[non-string value]";
        }
      }
      oss << "\n";

      // Retrieve pointer to std::ostringstream from Lua registry
      lua_getfield(L, LUA_REGISTRYINDEX, "cpp_output_stream");
      auto *out = static_cast<std::ostringstream *>(lua_touserdata(L, -1));
      lua_pop(L, 1);

      if (out) {
        *out << oss.str();
      }

      return 0;
    });

    lua_setglobal(L.get(), "print");

    // Store pointer to output stream in Lua registry
    lua_pushlightuserdata(L.get(), static_cast<void *>(&output));
    lua_setfield(L.get(), LUA_REGISTRYINDEX, "cpp_output_stream");

    // Execute Lua code
    int status = luaL_dostring(L.get(), command.data());
    if (status != LUA_OK) {
      const char *error_msg = lua_tostring(L.get(), -1);
      std::string error_str = error_msg ? error_msg : "Unknown error";
      lua_pop(L.get(), 1);
      output << "Lua Error: " << error_str << "\n";
    }

    return output.str();

  } catch (const std::exception &e) {
    return std::string("Lua execution error: ") + e.what() + "\n";
  } catch (...) {
    return "Unknown error during Lua execution\n";
  }
}
