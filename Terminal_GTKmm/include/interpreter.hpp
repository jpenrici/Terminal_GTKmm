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
#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <string>
#include <string_view>
#include <vector>

class Interpreter {

public:
    Interpreter() = default;
    ~Interpreter();

    enum Languages {
        DEFAULT,
        BASH,
        PYTHON,
        LUA
    };

    static auto name(int index) -> std::string;

    [[ nodiscard ]] static auto execute_command(const std::string_view command, size_t number) -> std::string;

private:
    static const std::vector<std::string> s_names;

    [[ nodiscard ]] static auto execute_bash(const std::string_view command) -> std::string;
    [[ nodiscard ]] static auto execute_python(const std::string_view command) -> std::string;
    [[ nodiscard ]] static auto execute_lua(const std::string_view command) -> std::string;
};

#endif // INTERPRETER_HPP
