#include <iostream>
extern "C" {
#include "aiger.h"
}

int main() {
    const char* aigerFile = "example.aag";
    
    // Load AIGER file
    aiger* circuit = aiger_init();
    auto result = aiger_open_and_read_from_file(circuit, aigerFile);
    if (result != NULL) {
        std::cerr << "Failed to read AIGER file: " << aiger_error(circuit) << std::endl;
        return 1;
    }
    
    // Access circuit information
    uint64_t numInputs = circuit->num_inputs;
    uint64_t numOutputs = circuit->num_outputs;
    uint64_t numLatches = circuit->num_latches;
    uint64_t numAnds = circuit->num_ands;
    
    // Print circuit information
    std::cout << "Inputs: " << numInputs << std::endl;
    std::cout << "Outputs: " << numOutputs << std::endl;
    std::cout << "Latches: " << numLatches << std::endl;
    std::cout << "AND gates: " << numAnds << std::endl;

    // Iterate over the AND gates
    for (unsigned i = 0; i < circuit->num_ands; ++i) {
        aiger_and* and_gate = circuit->ands + i;
        unsigned output_var = and_gate->lhs;
        unsigned input1_var = and_gate->rhs0;
        unsigned input2_var = and_gate->rhs1;
        std::cout << "AND Gate: Output = " << output_var
                  << ", Inputs = " << input1_var << ", " << input2_var << std::endl;
    }
    // BMC
    for(unsigned i = 0; i<20; i++){
        for(unsigned j=0; j < circuit->num_ands; j++){
            aiger_and* and_gate = circuit->ands + j;
            unsigned output_var = and_gate->lhs;
            unsigned input1_var = and_gate->rhs0;
            unsigned input2_var = and_gate->rhs1;
        }
    }
    // Cleanup
    aiger_reset(circuit);
    
    return 0;
}
