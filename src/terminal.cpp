#include "terminal.h"

#include <giomm.h>
#include <gtkmm-4.0/gtkmm/application.h>

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
}

void Terminal::setup_signals() {
  // Button events
  m_btn_input_clear.signal_clicked().connect(
      sigc::mem_fun(*this, &Terminal::on_btn_input_clear_clicked));
  m_btn_input_execute.signal_clicked().connect(
      sigc::mem_fun(*this, &Terminal::on_execute_command));
  m_btn_output_clear.signal_clicked().connect(
      sigc::mem_fun(*this, &Terminal::on_btn_output_clear_clicked));
}

void Terminal::create_menu() {
  auto menu_model = Gio::Menu::create();

  // File menu
  auto file_menu = Gio::Menu::create();
  file_menu->append("Save", "app.save");
  file_menu->append("Save as", "app.saveas");
  file_menu->append("Quit", "app.quit");
  menu_model->append_submenu("File", file_menu);

  // Execute menu
  auto tool_menu = Gio::Menu::create();
  tool_menu->append("Execute", "app.run");
  tool_menu->append("Clear Input", "app.clear_input");
  tool_menu->append("Clear Output", "app.clear_output");
  menu_model->append_submenu("Tools", tool_menu);

  // Help menu
  auto help_menu = Gio::Menu::create();
  help_menu->append("About", "app.about");
  menu_model->append_submenu("Help", help_menu);

  // Set the menu model for the PopoverMenuBar
  m_menu_bar.set_menu_model(menu_model);

  // Actions
  auto app = Gtk::Application::get_default();
  if (app) {
    app->add_action("save", sigc::mem_fun(*this, &Terminal::on_menu_file_save));
    app->add_action("saveas",
                    sigc::mem_fun(*this, &Terminal::on_menu_file_saveAs));
    app->add_action("quit", sigc::mem_fun(*this, &Terminal::on_menu_file_quit));
    app->add_action("run", sigc::mem_fun(*this, &Terminal::on_execute_command));
    app->add_action(
        "clear_input",
        sigc::mem_fun(*this, &Terminal::on_btn_input_clear_clicked));
    app->add_action(
        "clear_output",
        sigc::mem_fun(*this, &Terminal::on_btn_output_clear_clicked));
    app->add_action("about",
                    sigc::mem_fun(*this, &Terminal::on_menu_help_about));
  }
}

void Terminal::on_menu_file_save() {}

void Terminal::on_menu_file_saveAs() {}

void Terminal::on_menu_file_quit() { close(); }

void Terminal::on_menu_help_about() {}

void Terminal::setup_command_area() {
  // Configure label
  m_info_input.set_label("Enter the command:");
  m_info_output.set_label("");

  // Configure input area
  m_command_input.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
  m_command_input_buffer = m_command_input.get_buffer();

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

  // Main box
  m_main_box.append(m_info_input);
  m_main_box.append(m_input_scroll);
  m_main_box.append(m_input_tool_box);
  m_main_box.append(m_info_output);
  m_main_box.append(m_output_scroll);
  m_main_box.append(m_output_tool_box);
}

void Terminal::on_execute_command() {
  auto command = m_command_input_buffer->get_text();
  if (command.empty()) {
    m_info_input.set_label("Empty command input! Enter a command:");
    return;
  }

  try {
    // Execute command and show output
    auto output = execute_command(command);
    append_to_output(output);
  } catch (const std::runtime_error &e) {
    append_to_output(e.what(), true);
  }
}

auto Terminal::execute_command(const std::string &command) -> std::string {
  std::array<char, 128> buffer;
  std::string result;

  // popen : pipe stream to or from a process (Standard C library)
  FILE *pipe = popen(command.c_str(), "r");
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

void Terminal::append_to_output(const std::string &text, bool is_error) {
  auto end = m_command_output_buffer->end();

  if (is_error) {
    // Create tag for error text if it doesn't exist
    auto tag = m_command_output_buffer->get_tag_table()->lookup("error");
    if (!tag) {
      tag = m_command_output_buffer->create_tag("error");
      tag->property_foreground() = "#FF0000";
    }
    m_command_output_buffer->insert_with_tag(end, text, tag);
  } else {
    m_command_output_buffer->insert(end, text + "\n");
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

void Terminal::on_btn_input_clear_clicked() {
  m_command_input_buffer->set_text("");
  m_info_input.set_label("Enter a command:");
}

void Terminal::on_btn_output_clear_clicked() {
  m_command_output_buffer->set_text("");
  m_info_output.set_label("Result:");
}
