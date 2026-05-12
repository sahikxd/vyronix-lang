#include <iostream>
#include <vector>
#include <string>
#include "vyronix/VM.hpp"
#include "vyronix/Serialization.hpp"

using namespace vyronix;

void printUsage() {
    std::cout << "VYRONIX Virtual Machine (vyronixvm) — built " << __DATE__ << " " << __TIME__ << "\n";
    std::cout << "Usage: vyronixvm <input.vyb> [--debug] [--crash-dump]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_file = argv[1];
    bool debug = false;
    bool crash_dump = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            debug = true;
        } else if (arg == "--crash-dump") {
            crash_dump = true;
        }
    }

    try {
        auto code = Serializer::deserialize(input_file);

        if (debug) {
            std::cout << "Loaded " << code.size() << " instructions.\n";
            std::cout << "--- DISASSEMBLY ---\n";
            VirtualMachine::disassemble(code);
        }

        VirtualMachine vm;
        vm.setupStdLib();
        try {
            vm.run(code);
        } catch (const std::exception& e) {
            std::cerr << "Runtime error: " << e.what() << "\n";
            if (crash_dump || debug) {
                vm.dumpState(code);
            }
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "VM error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
