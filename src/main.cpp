#include <argparse/argparse.hpp>
#include <filesystem>

#include "headers/boot.h"
#include "headers/utils/log_utils.h"

int main(const int argc, char **argv) {

    argparse::ArgumentParser program("exmotion", "0.0.1");
    program.add_description(R"desc(
                To list available capture devices on linux you can use:
                [  $ v4l2-ctl --list-devices  ]

                For proper configuration first check your camera allowed properties
                [  $ v4l2-ctl -d \"/dev/video${ID}\" --list-formats-ext  ]
                )desc");
    program.add_argument("-p", "--project")
            .default_value(std::string(std::filesystem::current_path().string()))
            .help("Project root directory");
    program.add_argument("-t", "--terminal")
            .help("Terminal (console) mode")
            .flag();
    program.add_argument("-v", "--verbose")
            .help("Increase output verbosity")
            .flag();

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << '\n';
        std::cout << program;
        return 1;
    }

    if (program.get<bool>("--verbose")) {
        ex::logs::set_level(spdlog::level::debug);
    } else {
        ex::logs::set_level(spdlog::level::info);
    }

    const auto path = program.get<std::string>("--project");

    if (program.get<bool>("--terminal"))
        return ex::boot::ttui(argc, argv, path.c_str());
    return ex::boot::gui(argc, argv, path.c_str());
}
