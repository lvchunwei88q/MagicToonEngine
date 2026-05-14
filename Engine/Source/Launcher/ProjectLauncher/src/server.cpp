#include <httplib/httplib.h>
#include "Main.h"
#include <Converter.h>
#include <thread>

static httplib::Server server;
int server_port;

static std::thread server_thread;

void run_server() {
    server_thread = std::thread([]() {
        std::wstring src = IO::AbsolutePath::Get().GetScriptPath() + L"\\ProjectLauncher\\";

        server.set_mount_point("/", IO::Converter::ToNarrowString(src));

        server.set_file_extension_and_mimetype_mapping("js", "application/javascript");
        server.set_file_extension_and_mimetype_mapping("mjs", "application/javascript");
        server.set_file_extension_and_mimetype_mapping("html", "text/html");
        server.set_file_extension_and_mimetype_mapping("css", "text/css");
        server_port = server.bind_to_any_port("localhost");
        if (server_port < 0) {
            // Bind Error
            return;
        }
        if (!server.listen_after_bind()) {
            // Error
            return;
        }
    });
    server_thread.detach();
}

void stop_server() {
    server.stop();
}