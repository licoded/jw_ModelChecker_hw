#include <iostream>
#include <cstring>
#include <unordered_map>
extern "C"
{
#include "aiger/aiger.h"
}
#include "formula/af_utils.h"
#include "carchecker.h"
#define BOUND_LEN 20
using namespace aalta;

enum class CIRCUIT_TYPE
{
    LATCHE,
    OUTPUT,
};

aalta_formula *rule_af;
aalta_formula *input_af;
aalta_formula *equiv_af_global; // store the equivalence constraints
vector<pair<aalta_formula*, aalta_formula*>> equiv_vec_global;

void add_equivalence(aalta_formula *lhs, aalta_formula *rhs)
{
    equiv_vec_global.push_back(make_pair(lhs, rhs));
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
    add_equivalence(init, next);
}

bool check_SAT(aalta_formula *af)
{
    // TODO!!!
    aalta_formula *to_check = af;
    to_check = to_check->add_tail();
    to_check = to_check->remove_wnext();
    to_check = to_check->simplify();
    to_check = to_check->split_next();
    CARChecker checker(to_check, false, false);
    return checker.check();
}

bool check_valid(aalta_formula *af)
{
    aalta_formula *not_af = af_not(af);
    return !check_SAT(not_af);
}

void print_check_af()
{
    // cout << "\t" << equiv_af_global->to_string() << endl;
    for (auto &equiv_pair : equiv_vec_global)
    {
        cout << "\t" << equiv_pair.first->to_string() << "\t<->\t" << equiv_pair.second->to_string() << endl;
    }
    cout << "\t" << input_af->to_string() << endl;
    cout << "\t" << rule_af->to_string() << endl;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "Usage: bin_name [AIGER file]" << std::endl;
        return 1;
    }

    // cout << "usage is ok\t" << argv[1] << endl;

    equiv_af_global = aalta_formula::TRUE();

    // Load AIGER file
    aiger *circuit = aiger_init();
    auto result = aiger_open_and_read_from_file(circuit, argv[1]);
    if (result != NULL)
    {
        std::cerr << "Failed to read AIGER file: " << aiger_error(circuit) << std::endl;
        return 1;
    }

    // BMC
    input_af = aalta_formula::TRUE(); // store the rules
    for (unsigned j = 0; j < circuit->num_latches; j++)
    {
        unsigned lit = circuit->latches[j].lit;
        aalta_formula *latch_af = get_var_af(lit, 0);

        // initial vals(all zeros) of latch_var are fixed inputs of our circuit model
        input_af = af_and(input_af, af_not(latch_af));
    }
    
    // TODO: add even and odd id var equivalence constraints

    rule_af = aalta_formula::TRUE();
    for (unsigned bound_step = 0; bound_step < BOUND_LEN; bound_step++)
    {
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
            aalta_formula *next_af = get_var_af(next_lit, bound_step+1);

            add_latch(init_af, next_af);
        }

        // if (bound_step == 0) // don't do empty-check
        //     continue;

        for (unsigned i = 0; i < circuit->num_outputs; i++)
        {
            unsigned lit = circuit->outputs[i].lit;
            aalta_formula *output_af = get_var_af(lit, bound_step);

            // rule: vals of outputs are zeros forever
            rule_af = af_and(rule_af, af_not(output_af));
        }

        /* check current step BMC */
        print_check_af();

        if (check_valid(af_imply(af_and(equiv_af_global, input_af), rule_af)))
        {
            cout << "STEP-" << int2string(bound_step) << " OK!!!" << endl;
        }
        else
        {
            cout << "BMC fails at bound " << int2string(bound_step) << endl;
            break;
        }
    }

    // Cleanup
    aiger_reset(circuit);

    return 0;
}