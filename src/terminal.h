#ifndef TERMINAL_H
#define TERMINAL_H

#include <gtkmm-4.0/gtkmm/box.h>
#include <gtkmm-4.0/gtkmm/button.h>
#include <gtkmm-4.0/gtkmm/popovermenubar.h>
#include <gtkmm-4.0/gtkmm/scrolledwindow.h>
#include <gtkmm-4.0/gtkmm/textbuffer.h>
#include <gtkmm-4.0/gtkmm/textview.h>
#include <gtkmm-4.0/gtkmm/window.h>

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
    auto execute_shell_command(const std::string& command) -> std::string;
    void append_to_output(const std::string& text, bool is_error = false);
    void on_execute_command();

    // Buttons handling
    void on_btn_input_clear_clicked();
    void on_btn_input_execute();
    void on_btn_output_clear_clicked();

    // Menu actions
    void on_menu_file_quit();
    void on_menu_file_save();
    void on_menu_file_saveAs();
    void on_menu_help_about();

private:
    // UI Components
    Gtk::Box m_main_box{Gtk::Orientation::VERTICAL};
    Gtk::Box m_input_tool_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Box m_output_tool_box{Gtk::Orientation::HORIZONTAL};

    Gtk::Button m_btn_input_clear;
    Gtk::Button m_btn_input_execute;
    Gtk::Button m_btn_output_clear;

    Gtk::PopoverMenuBar m_menu_bar;
    Gtk::ScrolledWindow m_input_scroll;
    Gtk::ScrolledWindow m_output_scroll;
    Gtk::TextView m_command_input;
    Gtk::TextView m_command_output;

    // Buffers
    Glib::RefPtr<Gtk::TextBuffer> m_command_input_buffer;
    Glib::RefPtr<Gtk::TextBuffer> m_command_output_buffer;

    // Settings
    static constexpr size_t MAX_HISTORY_SIZE = 1000;
    static constexpr size_t MAX_OUTPUT_BUFFER_SIZE = 100000;
};

#endif // TERMINAL_H
