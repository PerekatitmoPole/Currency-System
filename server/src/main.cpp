#include "app/Application.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        const unsigned short port = argc > 1 ? static_cast<unsigned short>(std::stoi(argv[1])) : 5555;
        const std::size_t workers = argc > 2 ? static_cast<std::size_t>(std::stoul(argv[2])) : 4;

        currency::app::Application application(port, workers);
        application.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Server failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
