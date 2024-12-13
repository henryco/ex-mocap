#include <argparse/argparse.hpp>
#include <filesystem>

#include "headers/boot.h"
#include "headers/nodes/test_nodes.h"
#include "headers/utils/log_utils.h"

int main(const int argc, char **argv) {
    ex::test::Node1 node1;
    node1.deserialize("123");

    const auto a = node1.pull<float>(3, 0);
    const auto b = node1.pull<int, 2>(0);

    node1.has_output<int, 2>();

    std::cout << "a: " << a << ", b: " << b << '\n';

    int size_i = 0;
    const auto inputs = node1.describe_inputs(&size_i);

    int size_o = 0;
    const auto outputs = node1.describe_outputs(&size_o);

    for (int i = 0; i < size_i; i++) {
        const auto &[type_id, index, aux, name] = inputs[i];
        std::cout << "input: " << index << " (" << type_id << "), " << aux << ", " << name << "\n";
    }

    for (int i = 0; i < size_o; i++) {
        const auto &[type_id, index, aux, name] = outputs[i];
        std::cout << "output: " << index << " (" << type_id << "), " << aux << ", " << name << "\n";
    }
    
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
