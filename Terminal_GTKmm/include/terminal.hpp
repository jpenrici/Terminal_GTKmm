/*
 * References:
 *    https://www.gtkmm.org
 *    https://docs.python.org/3/extending/index.html
 *
 * Requeriment
 *    libgtkmm-4.0-dev (Linux)
 *    python.h
 */
#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <gtkmm-4.0/gtkmm/aboutdialog.h>
#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#include <gtkmm-4.0/gtkmm/label.h>
#include <gtkmm-4.0/gtkmm/popovermenubar.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/textbuffer.h>
#include <gtkmm-4.0/gtkmm/textview.h>
#include <gtkmm-4.0/gtkmm/window.h>

#include "interpreter.hpp"

class Terminal : public Gtk::Window {

public:
    Terminal();
    virtual ~Terminal() = default;

protected:
    // Interface setup
    void create_menu();
    void setup_command_area();
    void setup_interface();
    void setup_signals();

    // Command handling
    auto execute_command(const std::string_view command) -> std::string;
    void append_to_output(const std::string_view text, bool is_error = false);
    void on_execute_command();

    // Buttons handling
    void on_btn_input_clear_clicked();
    void on_btn_output_clear_clicked();

    // Menu actions
    void on_menu_file_quit();
    void on_menu_file_open();
    void on_menu_file_save();
    void on_menu_file_saveAs();
    void on_menu_help_about();
    void on_menu_tools_clear(int operation = 0);
    void on_menu_interpreter(int interpreter_type = Interpreter::Languages::DEFAULT);

private:
    int m_interpreter_type;

    // UI Components
    Gtk::Box m_main_box{Gtk::Orientation::VERTICAL};
    Gtk::Box m_input_tool_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_output_tool_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_status_bar_box{Gtk::Orientation::HORIZONTAL};

    Gtk::Button m_btn_input_clear;
    Gtk::Button m_btn_input_execute;
    Gtk::Button m_btn_output_clear;

    Gtk::Label m_info_input;
    Gtk::Label m_info_output;
    Gtk::Label m_info_status_bar;

    Gtk::PopoverMenuBar m_menu_bar;
    Gtk::ScrolledWindow m_input_scroll;
    Gtk::ScrolledWindow m_output_scroll;
    Gtk::TextView m_command_input;
    Gtk::TextView m_command_output;

    std::unique_ptr<Gtk::AboutDialog> m_pAboutDialog;
    std::unique_ptr<Gtk::FileChooserDialog> m_pFileDialog;

    // Buffers
    Glib::RefPtr<Gtk::TextBuffer> m_command_input_buffer;
    Glib::RefPtr<Gtk::TextBuffer> m_command_output_buffer;
    Glib::RefPtr<Gtk::TextTag> m_input_tag;

    // Settings
    std::string m_path;

    static constexpr size_t MAX_HISTORY_SIZE = 1000;
    static constexpr size_t MAX_OUTPUT_BUFFER_SIZE = 100000;

    // Actions
    auto save(std::string path, std::string text) -> bool;
};

auto terminal(int argc, char *argv[]) -> int;

#endif // TERMINAL_HPP
