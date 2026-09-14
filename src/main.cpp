#include "PeachWallper.hpp"
#include "ur_log/ur-log.h"

#include <filesystem>
#include <string>

Peach::PeachWallpaper engine;

/** @brief Loads the wallpaper configuration and runs the wallpaper runtime. */
int main(int argc, char **argv) {
    std::filesystem::path exe_dir =
        std::filesystem::read_symlink("/proc/self/exe").parent_path();
    std::filesystem::path config_path = exe_dir / "config.json";

    if (!std::filesystem::exists(config_path)) {
        if (argc >= 3 && std::string(argv[1]) == "--config") {
            config_path = argv[2];
        } else {
            UR_CRITICAL("Usage: {} --config <filepath> (or place a "
                        "config.json next to the executable)",
                        argv[0]);
            return 1;
        }
    }

    engine.Start();

    if (!engine.LoadConfig(config_path.string())) {
        UR_CRITICAL("Failed to load config: {}", config_path.string());
        return 1;
    }

    engine.Update();
    engine.Shutdown();
    return 0;
}
