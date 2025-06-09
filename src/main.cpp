// by xd

#include <filesystem>

#include "headers/utils/log_utils.h"
#include "headers/boot.h"

int main(const int argc, char **argv) {
    // TODO cli args parse
    ex::logs::set_level(spdlog::level::debug);
    const auto path = std::filesystem::current_path();
    return ex::boot::gui(argc, argv, path.c_str());
}
