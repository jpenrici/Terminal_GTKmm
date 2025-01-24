/*
 * Reference:
 *    https://www.gtkmm.org
 *
 * Requeriment
 *    libgtkmm-4.0-dev (Linux)
 */
#include "terminal.h"
#include <gtkmm-4.0/gtkmm/application.h>

auto main(int argc, char *argv[]) -> int {
  auto app = Gtk::Application::create("com.gtkmm.app.terminal");
  const int status = app->make_window_and_run<Terminal>(argc, argv);

  return status;
}
