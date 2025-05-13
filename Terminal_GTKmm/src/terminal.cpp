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
#include "terminal.hpp"

#include <giomm.h>
#include <glib.h>
#include <gtkmm-4.0/gtkmm/application.h>
#include <gtkmm-4.0/gtkmm/filechooserdialog.h>
#include <gtkmm-4.0/gtkmm/messagedialog.h>

#include <fstream>

Terminal::Terminal() {
  set_title("Experimental Terminal");
  set_default_size(800, 600);

  setup_interface();
  setup_signals();
}

void Terminal::setup_interface() {
  // Configure main box
  m_main_box.set_margin(5);
  m_main_box.set_spacing(5);

  // Create menu
  create_menu();
  m_main_box.append(m_menu_bar);

  // Setup command area
  setup_command_area();

  // Set the main container
  set_child(m_main_box);

  // Set Interpreter
  on_menu_interpreter(Interpreter::Languages::BASH);
}

void Terminal::setup_signals() {
  // Button events
  m_btn_input_execute.signal_clicked().connect(
      sigc::mem_fun(*this, &Terminal::on_execute_command));
  m_btn_input_clear.signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_tools_clear), 1));
  m_btn_output_clear.signal_clicked().connect(
      sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_tools_clear), 2));
}

void Terminal::create_menu() {
  auto menu_model = Gio::Menu::create();

  // File menu
  auto file_menu = Gio::Menu::create();
  file_menu->append("Open", "app.open");
  file_menu->append("Save", "app.save");
  file_menu->append("Save as", "app.saveas");
  file_menu->append("Quit", "app.quit");
  menu_model->append_submenu("File", file_menu);

  // Tools menu
  auto tools_menu = Gio::Menu::create();

  auto clear_menu = Gio::Menu::create();
  clear_menu->append("Clear All", "app.clear");
  clear_menu->append("Clear Input", "app.clear_input");
  clear_menu->append("Clear Output", "app.clear_output");

  auto interpreter_menu = Gio::Menu::create();
  interpreter_menu->append("Bash", "app.interpreter_bash");
  interpreter_menu->append("Python", "app.interpreter_python");
  interpreter_menu->append("Lua", "app.interpreter_lua");

  tools_menu->append("Execute", "app.run");
  tools_menu->append_submenu("Interpreter", interpreter_menu);
  tools_menu->append_submenu("Clear", clear_menu);

  menu_model->append_submenu("Tools", tools_menu);

  // Help menu
  auto help_menu = Gio::Menu::create();
  help_menu->append("About", "app.about");
  menu_model->append_submenu("Help", help_menu);

  // Set the menu model for the PopoverMenuBar
  m_menu_bar.set_menu_model(menu_model);

  // Actions
  auto app = Gtk::Application::get_default();
  if (app) {
    // File
    app->add_action("open", sigc::mem_fun(*this, &Terminal::on_menu_file_open));
    app->add_action("save", sigc::mem_fun(*this, &Terminal::on_menu_file_save));
    app->add_action("saveas",
                    sigc::mem_fun(*this, &Terminal::on_menu_file_saveAs));
    app->add_action("quit", sigc::mem_fun(*this, &Terminal::on_menu_file_quit));
    app->add_action("run", sigc::mem_fun(*this, &Terminal::on_execute_command));
    // Interpreter
    app->add_action(
        "interpreter_bash",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_interpreter),
                   Interpreter::Languages::BASH));
    app->add_action(
        "interpreter_python",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_interpreter),
                   Interpreter::Languages::PYTHON));
    app->add_action(
        "interpreter_lua",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_interpreter),
                   Interpreter::Languages::LUA));
    // Clear
    app->add_action(
        "clear",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_tools_clear), 0));
    app->add_action(
        "clear_input",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_tools_clear), 1));
    app->add_action(
        "clear_output",
        sigc::bind(sigc::mem_fun(*this, &Terminal::on_menu_tools_clear), 2));
    app->add_action("about",
                    sigc::mem_fun(*this, &Terminal::on_menu_help_about));
  }
}

void Terminal::on_menu_file_open() {

  if (!m_pFileDialog) {
    // Filters
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Script Files");
    filter_text->add_pattern("*.py");
    filter_text->add_pattern("*.lua");
    filter_text->add_pattern("*.sh");
    // Dialog
    m_pFileDialog.reset(new Gtk::FileChooserDialog(
        "Select a script file", Gtk::FileChooser::Action::OPEN));
    m_pFileDialog->set_transient_for(*this);
    m_pFileDialog->set_modal(true);
    m_pFileDialog->set_hide_on_close(true);
    m_pFileDialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
    m_pFileDialog->add_button("_Open", Gtk::ResponseType::ACCEPT);
    m_pFileDialog->add_filter(filter_text);
    m_pFileDialog->signal_response().connect([this](int response_id) {
      if (response_id == Gtk::ResponseType::ACCEPT) {
        if (auto f = m_pFileDialog->get_file()) {
          m_path = f->get_path();
          // Read file content
          std::ifstream file(m_path);
          if (!file) {
            Gtk::MessageDialog error_dialog(*this, "Failed to open file.",
                                            false, Gtk::MessageType::ERROR);
            error_dialog.set_modal(true);
            error_dialog.present();
            return;
          }
          // Determine interpreter by file extension
          if (m_path.ends_with(".py")) {
            m_interpreter_type = Interpreter::Languages::PYTHON;
          } else if (m_path.ends_with(".lua")) {
            m_interpreter_type = Interpreter::Languages::LUA;
          } else if (m_path.ends_with(".sh")) {
            m_interpreter_type = Interpreter::Languages::BASH;
          } else {
            Gtk::MessageDialog error_dialog(*this, "Unsupported file type.",
                                            false, Gtk::MessageType::ERROR);
            error_dialog.set_modal(true);
            error_dialog.present();
            return;
          }
          // Updates
          std::ostringstream buffer;
          buffer << file.rdbuf();
          on_menu_tools_clear();
          on_menu_interpreter(m_interpreter_type);
          m_command_input_buffer->set_text(buffer.str());
        }
      }
      m_pFileDialog->hide();
    });
  }

  m_pFileDialog->show();
}

void Terminal::on_menu_file_save() {
  // Get texts
  auto input_text = m_command_input_buffer->get_text();
  auto output_text = m_command_output_buffer->get_text();

  if (input_text.empty()) {
    m_info_input.set_label("Empty command input! Enter a command:");
    return;
  }

  if (output_text.empty()) {
    m_info_output.set_label("Empty output!");
    return;
  }

  if (m_path.empty()) {
    on_menu_file_saveAs();
  }

  g_message("[Terminal App] Path: %s", m_path.c_str());

  auto filename = "terminal_commands.txt";
  auto path = m_path + "/" + filename;
  if (save(path, input_text)) {
    m_info_input.set_text("Save in " + m_path + " ... " + filename);
  } else {
    m_info_input.set_text("There was something wrong!");
  }
  filename = "terminal_result.txt";
  path = m_path + "/" + filename;
  if (save(path, output_text)) {
    m_info_output.set_text("Save in " + m_path + " ... " + filename);
  } else {
    m_info_output.set_text("There was something wrong!");
  }
}

void Terminal::on_menu_file_saveAs() {
  if (!m_pFileDialog) {
    m_pFileDialog.reset(new Gtk::FileChooserDialog(
        "Select Folder", Gtk::FileChooser::Action::SELECT_FOLDER));
    m_pFileDialog->set_transient_for(*this);
    m_pFileDialog->set_modal(true);
    m_pFileDialog->set_hide_on_close(true);
    m_pFileDialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
    m_pFileDialog->add_button("_Select", Gtk::ResponseType::ACCEPT);
    m_pFileDialog->signal_response().connect([this](int response_id) {
      if (response_id == Gtk::ResponseType::ACCEPT) {
        if (auto file = m_pFileDialog->get_file()) {
          m_path = file->get_path();
          on_menu_file_save();
        }
      }
      m_pFileDialog->hide();
    });
  }

  m_pFileDialog->show();
}

void Terminal::on_menu_file_quit() { close(); }

void Terminal::on_menu_help_about() {
  if (!m_pAboutDialog) {
    m_pAboutDialog.reset(new Gtk::AboutDialog);
    m_pAboutDialog->set_transient_for(*this);
    m_pAboutDialog->set_hide_on_close();
    m_pAboutDialog->set_program_name("Terminal App");
    m_pAboutDialog->set_version("1.0.0");
    m_pAboutDialog->set_copyright("jpenrici");
    m_pAboutDialog->set_comments("Simple application using GTKmm 4.");
    m_pAboutDialog->set_license("LGPL");
    m_pAboutDialog->set_website("http://www.gtkmm.org");
    m_pAboutDialog->set_website_label("gtkmm website");
    m_pAboutDialog->set_authors(std::vector<Glib::ustring>{"jpenrici"});
  }
  m_pAboutDialog->present();
}

void Terminal::on_menu_tools_clear(int operation) {
  // 0 (input and output), 1 (input), 2 (output)
  if (operation == 0 || operation == 1) {
    m_command_input_buffer->set_text("");
    m_info_input.set_label("Enter a command:");
  }
  if (operation == 0 || operation == 2) {
    m_command_output_buffer->set_text("");
    m_info_output.set_label("Result:");
  }
}

void Terminal::on_menu_interpreter(int interpreter_type) {
  m_interpreter_type = interpreter_type;
  std::string interpreter = Interpreter::name(interpreter_type);
  m_info_status_bar.set_text(!interpreter.empty()
                                 ? "Interpreter: " + interpreter
                                 : "Undefined Interpreter");
}

void Terminal::setup_command_area() {
  // Configure label
  m_info_input.set_label("Enter the command:");
  m_info_output.set_label("");

  // Configure input area
  m_command_input.set_left_margin(15);
  m_command_input.set_monospace(true);
  m_command_input.set_top_margin(15);
  m_command_input.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
  m_command_input_buffer = m_command_input.get_buffer();

  auto tag = m_command_input_buffer->create_tag();
  tag->property_foreground() = "#00FF00"; // Define green color

  m_command_input_buffer->signal_changed().connect([this, tag]() {
    m_command_input_buffer->apply_tag(tag, m_command_input_buffer->begin(),
                                      m_command_input_buffer->end());
  });

  m_input_scroll.set_child(m_command_input);
  m_input_scroll.set_vexpand(true);

  // Configure input buttons
  m_btn_input_clear.set_label("Clear");
  m_btn_input_clear.set_margin(5);

  m_btn_input_execute.set_label("Execute");
  m_btn_input_execute.set_margin(5);

  // Configure output area
  m_command_output.set_cursor_visible(false);
  m_command_output.set_editable(false);
  m_command_output.set_left_margin(15);
  m_command_output.set_monospace(true);
  m_command_output.set_top_margin(15);
  m_command_output.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
  m_command_output_buffer = m_command_output.get_buffer();

  m_output_scroll.set_child(m_command_output);
  m_output_scroll.set_vexpand(true);

  // Configure output buttons
  m_btn_output_clear.set_label("Clear");
  m_btn_output_clear.set_margin(5);

  // Tool box
  m_input_tool_box.append(m_btn_input_clear);
  m_input_tool_box.append(m_btn_input_execute);
  m_output_tool_box.append(m_btn_output_clear);

  // Status box
  m_status_bar_box.append(m_info_status_bar);

  // Main box
  m_main_box.append(m_info_input);
  m_main_box.append(m_input_scroll);
  m_main_box.append(m_input_tool_box);
  m_main_box.append(m_info_output);
  m_main_box.append(m_output_scroll);
  m_main_box.append(m_output_tool_box);
  m_main_box.append(m_status_bar_box);
}

void Terminal::on_execute_command() {
  auto command = m_command_input_buffer->get_text();
  if (command.empty()) {
    m_info_input.set_label("Empty command input! Enter a command:");
    return;
  }

  try {
    // Execute command and show output
    auto output = execute_command(command.data());
    append_to_output(output);
  } catch (const std::exception &e) {
    append_to_output(e.what(), true);
  }
}

auto Terminal::execute_command(const std::string_view command) -> std::string {
  return Interpreter::execute_command(command, m_interpreter_type);
}

void Terminal::append_to_output(const std::string_view text, bool is_error) {
  auto end = m_command_output_buffer->end();

  if (is_error) {
    // Create tag for error text if it doesn't exist
    auto tag = m_command_output_buffer->get_tag_table()->lookup("error");
    if (!tag) {
      tag = m_command_output_buffer->create_tag("error");
      tag->property_foreground() = "#FF0000";
    }
    m_command_output_buffer->insert_with_tag(end, text.data(), tag);
  } else {
    auto tag = m_command_output_buffer->get_tag_table()->lookup("ok");
    if (!tag) {
      tag = m_command_output_buffer->create_tag("ok");
      tag->property_foreground() = "#95A3FC";
    }
    std::string txt = text.data();
    m_command_output_buffer->insert_with_tag(end, txt + "\n", tag);
  }

  // Scroll to end
  auto mark = m_command_output_buffer->create_mark(end);
  m_command_output.scroll_to(mark);

  // Limit buffer size
  if (m_command_output_buffer->get_char_count() > MAX_OUTPUT_BUFFER_SIZE) {
    auto start = m_command_output_buffer->begin();
    auto end = m_command_output_buffer->get_iter_at_offset(
        m_command_output_buffer->get_char_count() - MAX_OUTPUT_BUFFER_SIZE);
    m_command_output_buffer->erase(start, end);
  }
}

auto Terminal::save(std::string path, std::string text) -> bool {
  if (!text.empty()) {
    try {
      std::fstream fileout;
      fileout.open(path, std::ios::out);
      fileout << text;
      fileout.close();
    } catch (...) {
      return false;
    }
  }
  return true;
}

// Main
auto terminal(int argc, char *argv[]) -> int {
  auto app = Gtk::Application::create("com.gtkmm.app.terminal");
  const int status = app->make_window_and_run<Terminal>(argc, argv);

  return status;
}
