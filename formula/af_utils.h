#ifndef __AF_UTILS__
#define __AF_UTILS__

#include <unordered_set>
#include "formula/aalta_formula.h"
using namespace aalta;
using namespace std;

void af_init();

aalta_formula *xnf(aalta_formula *af);
aalta_formula *FormulaProgression(aalta_formula *predecessor, unordered_set<int> &edge);
bool BaseWinningAtY(aalta_formula *end_state, unordered_set<int> &Y);
bool IsAcc(aalta_formula *predecessor, unordered_set<int> &tmp_edge);

aalta_formula *af_not(aalta_formula *af);
aalta_formula *af_next(aalta_formula *af);
aalta_formula *af_and(aalta_formula *af1, aalta_formula *af2);
aalta_formula *af_or(aalta_formula *af1, aalta_formula *af2);
aalta_formula *af_imply(aalta_formula *af1, aalta_formula *af2);
aalta_formula *af_equiv(aalta_formula *af1, aalta_formula *af2);

aalta_formula *af_and_simplify(aalta_formula *af1, aalta_formula *af2);
aalta_formula *af_or_simplify(aalta_formula *af1, aalta_formula *af2);

string int2string(int i);
string get_var_name(unsigned var, int i);
aalta_formula *get_var_af(unsigned var, int i);

#endif