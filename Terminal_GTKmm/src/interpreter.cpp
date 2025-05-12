#include "interpreter.hpp"

#include <Python.h>

#include <lua.hpp>

#include <array>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

Interpreter::~Interpreter() {
  // Release the Python Interpreter before exiting.
  if (Py_IsInitialized()) {
    Py_FinalizeEx();
  }
}

auto Interpreter::language(int index) -> std::string {
  if (index >= 0 and index < names.size()) {
    return names.at(index);
  }
  return names.at(0);
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

auto Interpreter::name(int index) -> std::string {
  Interpreter temp;
  return temp.language(index);
}

auto Interpreter::execute_bash(const std::string_view command) -> std::string {
  std::array<char, 128> buffer;
  std::string result;

  // popen : pipe stream to or from a process (Standard C library)
  FILE *pipe = popen(command.data(), "r");
  if (!pipe) {
    throw std::runtime_error("Failed to execute command");
  }

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }

  auto status = pclose(pipe);
  if (status != 0) {
    throw std::runtime_error("Command execution failed with status " +
                             std::to_string(status) + "\n");
  }

  return result;
}

auto Interpreter::execute_python(const std::string_view command)
    -> std::string {
  // Initializes the Python interpreter (if not already initialized)
  if (!Py_IsInitialized()) {
    Py_Initialize();
  }

  // Creates context dictionary for globals and locals
  PyObject *main_module =
      PyImport_AddModule("__main__"); // No DECREF, it's singleton
  PyObject *main_dict = PyModule_GetDict(main_module);

  // Redirect sys.stdout to capture Python output
  PyObject *sys_module = PyImport_ImportModule("sys");
  PyObject *io_module = PyImport_ImportModule("io");

  if (!sys_module || !io_module) {
    PyErr_Print();
    return "Error: Failed to import sys or io modules.\n";
  }

  PyObject *string_io = PyObject_CallMethod(io_module, "StringIO", NULL);
  if (!string_io) {
    PyErr_Print();
    Py_DECREF(sys_module);
    Py_DECREF(io_module);
    return "Error: Failed to create StringIO.\n";
  }

  PyObject_SetAttrString(sys_module, "stdout", string_io);
  PyObject_SetAttrString(sys_module, "stderr", string_io);

  // Execute the Python code
  PyObject *py_result =
      PyRun_String(command.data(), Py_file_input, main_dict, main_dict);

  if (!py_result) {
    PyErr_Print();
  } else {
    Py_DECREF(py_result);
  }

  // Retrieves the contents of StringIO
  PyObject *output = PyObject_CallMethod(string_io, "getvalue", NULL);
  std::string result;

  if (output && PyUnicode_Check(output)) {
    result = PyUnicode_AsUTF8(output);
  } else {
    result = "Error: Could not retrieve output.\n";
  }

  // Release references
  Py_XDECREF(output);
  Py_DECREF(string_io);
  Py_DECREF(sys_module);
  Py_DECREF(io_module);

  // Não finalizamos o interpretador globalmente para manter consistência
  return result;
}

auto Interpreter::execute_lua(const std::string_view command) -> std::string {

  lua_State *L = luaL_newstate();
  if (!L) {
    return "Error: Failed to create Lua state.\n";
  }

  // Buffer to capture Lua output
  std::ostringstream output;

  lua_pushcfunction(L, [](lua_State *L) -> int {
    int nargs = lua_gettop(L);
    std::ostringstream oss;
    for (int i = 1; i <= nargs; ++i) {
      if (i > 1) {
        oss << "\t";
      }
      oss << lua_tostring(L, i);
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

  lua_setglobal(L, "print");

  // Store pointer to output stream in Lua registry
  lua_pushlightuserdata(L, static_cast<void *>(&output));
  lua_setfield(L, LUA_REGISTRYINDEX, "cpp_output_stream");

  // Execute Lua code
  int status = luaL_dostring(L, command.data());
  if (status != LUA_OK) {
    const char *error_msg = lua_tostring(L, -1);
    output << "Lua Error: " << (error_msg ? error_msg : "Unknown error")
           << "\n";
    lua_pop(L, 1);
  }

  lua_close(L);

  return output.str();
}
