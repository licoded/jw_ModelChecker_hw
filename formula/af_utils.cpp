#include "af_utils.h"

aalta_formula *xnf(aalta_formula *phi)
{
    if (phi == NULL)
        return NULL;
    int op = phi->oper();
    if ((op == aalta_formula::True || op == aalta_formula::False) || op == aalta_formula::Not || (op == aalta_formula::Next || op == aalta_formula::WNext) || op >= 11)
    {
        return phi;
    }
    if (op == aalta_formula::And || op == aalta_formula::Or)
    {
        return aalta_formula(op, xnf(phi->l_af()), xnf(phi->r_af())).unique();
    }
    else if (op == aalta_formula::Until)
    { // l U r=xnf(r) | (xnf(l) & X(l U r))
        aalta_formula *next_phi = aalta_formula(aalta_formula::Next, NULL, phi).unique();
        aalta_formula *xnf_l_and_next_phi = aalta_formula(aalta_formula::And, xnf(phi->l_af()), next_phi).unique();
        return aalta_formula(aalta_formula::Or, xnf(phi->r_af()), xnf_l_and_next_phi).unique();
    }
    else if (op == aalta_formula::Release)
    { // l R r=xnf(r) & (xnf(l) | WX(l R r))
        aalta_formula *wnext_phi = aalta_formula(aalta_formula::WNext, NULL, phi).unique();
        aalta_formula *xnf_l_or_wnext_phi = aalta_formula(aalta_formula::Or, xnf(phi->l_af()), wnext_phi).unique();
        return aalta_formula(aalta_formula::And, xnf(phi->r_af()), xnf_l_or_wnext_phi).unique();
    }
}

aalta_formula *FormulaProgression(aalta_formula *predecessor, unordered_set<int> &edge)
{
    if (predecessor == NULL)
        return NULL;
    int op = predecessor->oper();
    if (op == aalta_formula::True || op == aalta_formula::False)
        return predecessor;
    else if (op == aalta_formula::And)
    {
        aalta_formula *lf = FormulaProgression(predecessor->l_af(), edge);
        aalta_formula *rf = FormulaProgression(predecessor->r_af(), edge);
        if ((lf->oper()) == aalta_formula::False || (rf->oper()) == aalta_formula::False)
            return aalta_formula::FALSE();
        else if ((lf->oper()) == aalta_formula::True)
            return rf;
        else if ((rf->oper()) == aalta_formula::True)
            return lf;
        else
            return aalta_formula(aalta_formula::And, lf, rf).unique();
    }
    else if (op == aalta_formula::Or)
    {
        aalta_formula *l_fp = FormulaProgression(predecessor->l_af(), edge);
        aalta_formula *r_fp = FormulaProgression(predecessor->r_af(), edge);
        if ((l_fp->oper()) == aalta_formula::True || (r_fp->oper()) == aalta_formula::True)
            return aalta_formula::TRUE();
        else if ((l_fp->oper()) == aalta_formula::False)
            return r_fp;
        else if ((r_fp->oper()) == aalta_formula::False)
            return l_fp;
        else
            return aalta_formula(aalta_formula::Or, l_fp, r_fp).unique();
    }
    else if (op == aalta_formula::Not || op >= 11)
    { // literal
        int lit = (op >= 11) ? op : (-((predecessor->r_af())->oper()));
        if (edge.find(lit) != edge.end())
            return aalta_formula::TRUE();
        else
            return aalta_formula::FALSE();
    }
    else if (op == aalta_formula::Next || op == aalta_formula::WNext)
    {
        return predecessor->r_af();
    }
    // if predecessor is in XNF,
    // the following two cases cannot appear
    else if (op == aalta_formula::Until)
    { // l U r = r | (l & X(l U r))
        aalta_formula *first_part = FormulaProgression(predecessor->r_af(), edge);
        if ((first_part->oper()) == aalta_formula::True)
            return aalta_formula::TRUE();
        aalta_formula *l_fp = FormulaProgression(predecessor->l_af(), edge);
        aalta_formula *second_part = NULL;
        if ((l_fp->oper()) == aalta_formula::True)
        {
            if (first_part == predecessor->r_af())
                return predecessor;
            second_part = predecessor;
        }
        else if ((l_fp->oper()) == aalta_formula::False)
            return first_part;
        else
            second_part = aalta_formula(aalta_formula::And, l_fp, predecessor).unique();
        if ((first_part->oper()) == aalta_formula::False)
            return second_part;
        else
            return aalta_formula(aalta_formula::Or, first_part, second_part).unique();
    }
    else if (op == aalta_formula::Release)
    { // l R r = r & (l | N(l R r))
        aalta_formula *first_part = FormulaProgression(predecessor->r_af(), edge);
        if ((first_part->oper()) == aalta_formula::False)
            return aalta_formula::FALSE();
        aalta_formula *l_fp = FormulaProgression(predecessor->l_af(), edge);
        aalta_formula *second_part = NULL;
        if ((l_fp->oper()) == aalta_formula::True)
            return first_part;
        else if ((l_fp->oper()) == aalta_formula::False)
            second_part = predecessor;
        else
            second_part = aalta_formula(aalta_formula::Or, l_fp, predecessor).unique();
        if ((first_part->oper()) == aalta_formula::True)
            return second_part;
        else
            return aalta_formula(aalta_formula::And, first_part, second_part).unique();
    }
}

bool BaseWinningAtY(aalta_formula *end_state, unordered_set<int> &Y)
{
    if (end_state == NULL)
        return false;
    int op = end_state->oper();
    if (op == aalta_formula::True || op == aalta_formula::WNext)
        return true;
    else if (op == aalta_formula::False || op == aalta_formula::Next)
        return false;
    else if (op == aalta_formula::And)
        return BaseWinningAtY(end_state->l_af(), Y) && BaseWinningAtY(end_state->r_af(), Y);
    else if (op == aalta_formula::Or)
        return BaseWinningAtY(end_state->l_af(), Y) || BaseWinningAtY(end_state->r_af(), Y);
    else if (op == aalta_formula::Not || op >= 11)
    { // literal
        int lit = (op >= 11) ? op : (-((end_state->r_af())->oper()));
        return Y.find(lit) != Y.end();
    }
    else if (op == aalta_formula::Until || op == aalta_formula::Release)
        return BaseWinningAtY(end_state->r_af(), Y);
}

bool IsAcc(aalta_formula *predecessor, unordered_set<int> &tmp_edge)
{
    if (predecessor == NULL)
        return false;
    int op = predecessor->oper();
    if (op == aalta_formula::True || op == aalta_formula::WNext)
        return true;
    else if (op == aalta_formula::False || op == aalta_formula::Next)
        return false;
    else if (op == aalta_formula::And)
        return BaseWinningAtY(predecessor->l_af(), tmp_edge) && BaseWinningAtY(predecessor->r_af(), tmp_edge);
    else if (op == aalta_formula::Or)
        return BaseWinningAtY(predecessor->l_af(), tmp_edge) || BaseWinningAtY(predecessor->r_af(), tmp_edge);
    else if (op == aalta_formula::Not || op >= 11)
    { // literal
        int lit = (op >= 11) ? op : (-((predecessor->r_af())->oper()));
        return tmp_edge.find(lit) != tmp_edge.end();
    }
    else if (op == aalta_formula::Until || op == aalta_formula::Release)
        return BaseWinningAtY(predecessor->r_af(), tmp_edge);
}

aalta_formula *af_not(aalta_formula *af)
{
    aalta_formula *af_res = aalta_formula(aalta_formula::Not, NULL, af).unique();
    return af_res;
}

aalta_formula *af_next(aalta_formula *af)
{
    aalta_formula *af_res = aalta_formula(aalta_formula::Next, NULL, af).unique();
    return af_res;
}

aalta_formula *af_wnext(aalta_formula *af)
{
    aalta_formula *af_res = aalta_formula(aalta_formula::WNext, NULL, af).unique();
    return af_res;
}

aalta_formula *af_and_simplify(aalta_formula *af1, aalta_formula *af2)
{
    if (af1 == aalta_formula::TRUE())
        return af2;
    if (af2 == aalta_formula::TRUE())
        return af1;
    return aalta_formula(aalta_formula::And, af1, af2).unique();
}
