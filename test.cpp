#include <iostream>
#include <unordered_map>
extern "C" {
    #include "aiger.h"
}
#include "formula/af_utils.h"
using namespace aalta;

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
    aalta_formula* formula = aalta_formula::TRUE(); // store the rules
    for(unsigned j = 0; j< circuit->num_latches; j++){
        aiger_symbol* latches = circuit->latches + j;
        unsigned latch_var = latches->lit;
        aalta_formula* tmp = aalta_formula::not(aalta_formula(to_string(latch_var) + "_" + to_string(0)).unique()).unique();
        // initial vals(all zeros) of latch_var is also input of the rules
        formula = aalta_formula::simplify_and(formula,tmp);
    }
    for(unsigned i = 0; i<20; i++){
        
        for(unsigned j = 0; j< circuit->num_outputs; j++){
            aiger_symbol* outputs = circuit->outputs + j;
            unsigned output_var = outputs->lit;
            unsigned output_gate = output_var %2==1 ? output_var-1:output_var;
            aalta_formula* tmp = aalta_formula(to_string(output_var) + "_" + to_string(i)).unique();
            if(output_gate != output_var){
                tmp = aalta_formula::not(tmp).unique();
            formula = aalta_formula::simplify_and(formula,tmp);
        }
        unordered_map<int, aalta_formula>latch_map;

        for(unsigned j=0; j < circuit->num_ands; j++){
            // 处理latch
            aiger_and* and_gate = circuit->ands + j;
            unsigned output_var = and_gate->lhs;
            unsigned input1_var = and_gate->rhs0;
            unsigned input2_var = and_gate->rhs1;
            
            unsigned output_gate = and_gate->lhs%2==1? output_var-1:output_var;
            unsigned input1_gate = and_gate->rhs0%2==1? input1_var-1:input1_var;
            unsigned input2_gate = and_gate->rhs1%2==1? input2_var-1:input2_var;


            aalta_formula* r1=NULL;
            aalta_formula* r2=NULL;
            /* STEP1: 判断与门的值是否出现过，没有则构造 */
            if(latch_map[input1_gate]){
                // 如果是非门
                if(input1_gate!= input1_var){
                    r1 = aalta_formula::not(latch_map[input1_var]).unique();
                }else{
                    r1 = latch_map[input1_var].unique();
                }
            }else{
                if(input1_gate!= input1_var){
                    r1 = aalta_formula::not(aalta_formula(to_string(input1_var) + "_" + to_string(i)).unique()).unique();
                }else{
                    r1 = aalta_formula(to_string(input1_var) + "_" + to_string(i)).unique()
                }
            }
            if(latch_map[input2_gate]){
                // 如果是非门
                if(input2_gate!= input2_var){
                    r1 = aalta_formula::not(latch_map[input2_var]).unique();
                }else{
                    r1 = latch_map[input2_var].unique();
                }
            }else{
                if(input2_gate!= input2_var){
                    r1 = aalta_formula::not(aalta_formula(to_string(input2_var) + "_" + to_string(i)).unique()).unique();
                }else{
                    r1 = aalta_formula(to_string(input2_var) + "_" + to_string(i)).unique()
                }
            }
            latch_map[output_var] = temp; // 将新的与门加入map
            // formula = aalta_formula::simplify_and(aalta_formula::simplify_and(formula,r1).unique(),r2).unique();

            /* STEP2: TODO: add latch trans formulas !!! */

            /* STEP3: TODO: construct current BMC formula !!! */
        }
    }
    // Cleanup
    aiger_reset(circuit);
    
    return 0;
}
