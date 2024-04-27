#include <iostream>
#include <cstring>
#include <unordered_map>
extern "C"
{
#include "aiger/aiger.h"
}
#include "formula/af_utils.h"
#define BOUND_LEN 20
using namespace aalta;

enum class CIRCUIT_TYPE
{
    LATCHE,
    OUTPUT,
};

aalta_formula *equiv_af_global; // store the equivalence constraints

unsigned get_lit(aiger *circuit, CIRCUIT_TYPE type, unsigned offset)
{
    switch (type)
    {
    case CIRCUIT_TYPE::LATCHE:
        return circuit->latches[offset].lit;
        break;

    case CIRCUIT_TYPE::OUTPUT:
        return circuit->outputs[offset].lit;
        break;
    
    default:
        cerr << "Unknown circuit type" << endl;
        exit(0);
    }
}

string get_var_name(unsigned var, int i)
{
    string s = "p" + to_string(var) + "_" + to_string(0);
    return s;
}

aalta_formula *get_var_af(unsigned var, int i)
{
    return aalta_formula(get_var_name(var, i).c_str()).unique();
}

void add_equivalence(aalta_formula *lhs, aalta_formula *rhs)
{
    // TODO: change impl to add equivalence cons in Minisat?
    aalta_formula *cur_equiv_af = af_equiv(lhs, rhs);
    equiv_af_global = af_and(equiv_af_global, cur_equiv_af);
}

void add_and_gate(aalta_formula *lhs, aalta_formula *rhs0, aalta_formula *rhs1)
{
    add_equivalence(lhs, af_and(rhs0, rhs1));
}

void add_latch(aalta_formula *init, aalta_formula *next)
{
    // TODO: change next to wnext?
    add_equivalence(init, af_next(next));
}

bool check_SAT(aalta_formula *af)
{
    // TODO!!!
    return true;
}

bool check_valid(aalta_formula *af)
{
    aalta_formula *not_af = af_not(af);
    return !check_SAT(not_af);
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: bin_name [AIGER file]" << std::endl;
        return 1;
    }

    // cout << "usage is ok\t" << argv[1] << endl;

    aalta_formula *af;
	// set tail id to be 1
	af = aalta_formula::TAIL();

    equiv_af_global = aalta_formula::TRUE();
    const char *aigerFile = argv[1];

    // Load AIGER file
    aiger *circuit = aiger_init();
    auto result = aiger_open_and_read_from_file(circuit, aigerFile);
    if (result != NULL)
    {
        std::cerr << "Failed to read AIGER file: " << aiger_error(circuit) << std::endl;
        return 1;
    }

    // Print circuit information
    std::cout << "Inputs: " << circuit->num_inputs << std::endl;
    std::cout << "Outputs: " << circuit->num_outputs << std::endl;
    std::cout << "Latches: " << circuit->num_latches << std::endl;
    std::cout << "AND gates: " << circuit->num_ands << std::endl;

    // Iterate over the AND gates
    for (unsigned i = 0; i < circuit->num_ands; ++i)
    {
        aiger_and *and_gate = circuit->ands + i;
        unsigned output_var = and_gate->lhs;
        unsigned input1_var = and_gate->rhs0;
        unsigned input2_var = and_gate->rhs1;
        std::cout << "AND Gate: Output = " << output_var
                  << ", Inputs = " << input1_var << ", " << input2_var << std::endl;
    }

    // BMC
    aalta_formula *input_af = aalta_formula::TRUE(); // store the rules
    for (unsigned j = 0; j < circuit->num_latches; j++)
    {
        unsigned lit = get_lit(circuit, CIRCUIT_TYPE::LATCHE, j);
        aalta_formula *latch_af = get_var_af(lit, 0);

        // initial vals(all zeros) of latch_var are fixed inputs of our circuit model
        input_af = af_and(input_af, af_not(latch_af));
    }
    
    // TODO: add even and odd id var equivalence constraints

    aalta_formula *rule_af = aalta_formula::TRUE();
    for (unsigned bound_step = 0; bound_step < BOUND_LEN; bound_step++)
    {
        for (unsigned i = 0; i < circuit->num_outputs; i++)
        {
            unsigned lit = circuit->outputs[i].lit;
            aalta_formula *output_af = get_var_af(lit, 0);

            // rule: vals of outputs are zeros forever
            rule_af = af_and(rule_af, af_not(output_af));
        }

        for (unsigned i = 0; i < circuit->num_ands; i++)
        {
            aiger_and *and_gate = circuit->ands + i;
            aalta_formula *lhs_af = get_var_af(and_gate->lhs, bound_step);
            aalta_formula *rhs0_af = get_var_af(and_gate->rhs0, bound_step);
            aalta_formula *rhs1_af = get_var_af(and_gate->rhs1, bound_step);

            add_and_gate(lhs_af, rhs0_af, rhs1_af);
        }

        /* add latch trans formulas !!! */
        for (unsigned i = 0; i < circuit->num_latches; i++)
        {
            unsigned init_lit = circuit->latches[i].lit;
            unsigned next_lit = circuit->latches[i].next;
            aalta_formula *init_af = get_var_af(init_lit, bound_step);
            aalta_formula *next_af = get_var_af(next_lit, bound_step);

            add_latch(init_af, next_af);
        }

        /* check current step BMC */
        if (!check_valid(rule_af))
        {
            cout << "BMC fails at bound " << bound_step << endl;
            break;
        }
    }

    // Cleanup
    aiger_reset(circuit);

    return 0;
}