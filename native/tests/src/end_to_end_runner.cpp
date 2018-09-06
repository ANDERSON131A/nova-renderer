/*!
 * \author ddubois 
 * \date 30-Aug-18.
 */

#include <iostream>

#include "../../src/nova_renderer.hpp"

#include "../../src/platform.hpp"

#ifdef _WIN32
#include "../../src/render_engine/dx12/dx12_render_engine.hpp"
#else
#include "../../src/render_engine/vulkan/vulkan_render_engine.hpp"
#endif

#include "../../src/util/logger.hpp"
#include "../../src/loading/regular_folder_accessor.hpp"

int main(int num_args, const char** args) {
    // Add default logging handlers

    auto& log = nova::logger::instance;
    log.add_log_handler(nova::log_level::TRACE,     [] (auto msg) {std::cout << "TRACE: " << msg << "\n";});
    log.add_log_handler(nova::log_level::DEBUG,     [] (auto msg) {std::cout << "DEBUG: " << msg << "\n";});
    log.add_log_handler(nova::log_level::INFO,      [] (auto msg) {std::cout << "INFO: " << msg << "\n";});
    log.add_log_handler(nova::log_level::WARN,      [] (auto msg) {std::cerr << "WARN: " << msg << "\n";});
    log.add_log_handler(nova::log_level::ERROR,     [] (auto msg) {std::cerr << "ERROR: " << msg << "\n";});
    log.add_log_handler(nova::log_level::FATAL,     [] (auto msg) {std::cerr << "FATAL: " << msg << "\n";});
    log.add_log_handler(nova::log_level::MAX_LEVEL, [] (auto msg) {std::cerr << "MAX_LEVEL: " << msg << "\n";});

    auto file_test = nova::regular_folder_accessor(fs::path{"shaderpacks/DefaultShaderpack"});
    file_test.get_all_items_in_folder({"."});

    auto renderer = nova::nova_renderer::initialize();

    std::shared_ptr<nova::iwindow> window = renderer->get_engine()->get_window();

    while(!window->should_close()) {
        // renderer->execute_frame();
        window->on_frame_end();
    }

    nova::nova_renderer::deinitialize();

    return 0;
}
