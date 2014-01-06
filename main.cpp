#include <stdexcept>
#include <iostream>


int main() {
    try {
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
