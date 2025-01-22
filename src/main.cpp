#include "terminal.h"
#include <gtkmm-4.0/gtkmm/application.h>

auto main(int argc, char* argv[]) -> int
{
    auto app = Gtk::Application::create("com.gtkmm.app.terminal");

    // Show the window and returns when it is closed
    return app->make_window_and_run<Terminal>(argc, argv);
}
