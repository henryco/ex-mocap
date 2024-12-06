#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "headers/boot.h"
#include "headers/nodes/test_nodes.h"

int main(const int argc, char **argv) {

    ex::test::Node1 one;
    ex::test::Node2 two;

    const auto a = one.pull<int>(1, 0);
    const auto b = one.has_output<int, 4>();
    const auto c = one.has_output<int>(2);
    const auto d = one.has_input<short>(2);
    const auto e = one.has_input<short>(7);
    const auto f = one.has_input<short, 7>();
    const auto g = one.pull<int, 1>(0);

    std::cout << "a: " << a << '\n';
    std::cout << "b: " << b << '\n';
    std::cout << "c: " << c << '\n';
    std::cout << "d: " << d << '\n';
    std::cout << "e: " << e << '\n';
    std::cout << "f: " << f << '\n';
    std::cout << "g: " << g << '\n';


    return 1;

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
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    const auto path = program.get<std::string>("--project");

    if (program.get<bool>("--terminal"))
        return ex::boot::ttui(argc, argv, path.c_str());
    return ex::boot::gui(argc, argv, path.c_str());
}
