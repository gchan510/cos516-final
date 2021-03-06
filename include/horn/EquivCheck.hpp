#ifndef EQUIVHECK__HPP__
#define EQUIVHECK__HPP__

#include <string>

#include "Horn.hpp"
#include "CandChecker.hpp"
#include "ae/SMTUtils.hpp"

using namespace std;
using namespace boost;
namespace ufo
{
  inline void checkPrerequisites (CHCs& r)
  {
    if (r.decls.size() > 1)
    {
      outs() << "Nested loops not supported\n";
      exit(0);
    }

    if (r.chcs.size() < 2)
    {
      outs() << "Please provide a file with at least two rules (INIT and TR)\n";
      exit(0);
    }

    HornRuleExt* tr = NULL;
    HornRuleExt* fc = NULL;

    for (auto & a : r.chcs)
      if (a.isInductive) tr = &a;
      else if (a.isFact) fc = &a;

    if (tr == NULL)
    {
      outs() << "TR is missing\n";
      exit(0);
    }

    if (fc == NULL)
    {
      outs() << "INIT is missing\n";
      exit(0);
    }
  }

  /*
   * Return vector with [TR, INIT, BAD]
   */
  struct TRRels{
    HornRuleExt* TR;
    HornRuleExt* INIT;
    HornRuleExt* BAD;
  };
  inline struct TRRels getTransitionRelations( CHCs &chc )
  {
    TRRels retval;
    /* chc.print(); */
    for (auto &hr: chc.chcs)
    {
      // if hr is a TR()
      if ( !hr.isFact && !hr.isQuery  && hr.isInductive)
      {
        retval.TR =  &hr;
        /* outs () << "TR: " << *(hr.body) << "\n"; */
      }

      // if hr is an INIT()
      else if ( hr.isFact && !hr.isQuery && !hr.isInductive )
      {
        retval.INIT =  &hr;
        /* outs () << "INIT: " << *(hr.body) << "\n"; */
      }

      // if hr is a BAD()
      else if ( !hr.isFact && hr.isQuery  && !hr.isInductive)
      {
        retval.BAD =  &hr;
        /* outs () << "BAD: " << *(hr.body) << "\n"; */
      }
    }

    return retval;
  }

  inline void equivCheck(string chcfile1, string chcfile2, int unroll1, int unroll2)
  {
    ExprFactory efac;
    EZ3 z3(efac);

    CHCs ruleManager1(efac, z3);
    ruleManager1.parse(chcfile1, "v0_");
    checkPrerequisites(ruleManager1);
    /* outs () << "Program encoding #1:\n"; */
    /* ruleManager1.print(); */

    CHCs ruleManager2(efac, z3);
    ruleManager2.parse(chcfile2, "w0_");
    checkPrerequisites(ruleManager2);
    /* outs () << "Program encoding #2:\n"; */
    /* ruleManager2.print(); */

    ExprFactory expr_factory;
    SMTUtils utils(expr_factory);

    // TODO: check equivalence between programs encoded in ruleManager1 and ruleManager2
    std::vector<Expr> init_exprs;
    std::vector<Expr> trans_exprs;
    // std::vector<Expr> bad_exprs;
    HornRuleExt *bad_expr1;
    HornRuleExt *bad_expr2;

    HornRuleExt *trans_hr1;
    HornRuleExt *fact_hr1;
    HornRuleExt *trans_hr2;
    HornRuleExt *fact_hr2;

    struct TRRels trs1 = getTransitionRelations(ruleManager1);
    struct TRRels trs2 = getTransitionRelations(ruleManager2);

    trans_exprs.push_back(trs1.TR->body);
    trans_hr1 = trs1.TR;
    fact_hr1 = trs1.INIT;
    bad_expr1 = trs1.BAD;

    trans_exprs.push_back(trs2.TR->body);
    trans_hr2 = trs2.TR;
    fact_hr2 = trs2.INIT;
    bad_expr2 = trs2.BAD;

    /* outs() << "Original TR:\n"; */
    /* outs() << "\t" << *(trans_hr1->body) << "\n"; */

    HornRuleExt *trans_dst1 = trans_hr1;
    HornRuleExt *trans_dst2 = trans_hr2;
    HornRuleExt *prev_rule = trans_hr1;
    std::vector<CHCs> temp1;
    std::vector<CHCs> temp2;
    for ( int i = 1; i < unroll1; i++ )
    {
      CHCs real_temp(efac, z3);
      real_temp.parse(chcfile1, "v" + to_string(i) + "_");
      checkPrerequisites(real_temp);
      temp1.push_back(real_temp);
    }
    for ( int i = 1; i < unroll2; i++ )
    {
      CHCs real_temp(efac, z3);
      real_temp.parse(chcfile2, "w" + to_string(i) + "_");
      checkPrerequisites(real_temp);
      temp2.push_back(real_temp);
    }

    /* outs() << "Printing out TRs of unrolling\n"; */
    for ( int i = 1; i < unroll1; i++ )
    {
      struct TRRels rels = getTransitionRelations(temp1[i-1]);
      Expr replaced_tr = rels.TR->body;
      /* outs() << "Before:\n"; */
      /* outs() << "\t" << *(rels.TR->body) << "\n"; */

      /* outs() << "Prev rule pointer: " << prev_rule << "\n"; */
      for ( int i = 0; i < rels.TR->srcVars.size(); i++ )
        replaced_tr = replaceAll(replaced_tr, rels.TR->srcVars[i], prev_rule->dstVars[i]);
      trans_exprs.push_back(replaced_tr);

      /* outs() << "After:\n"; */
      /* outs() << "\t" << *(replaced_tr) << "\n"; */

      prev_rule = rels.TR;
      bad_expr1 = rels.BAD;
      trans_dst1 = rels.TR;
    }

    prev_rule = trans_hr2;
    for ( int i = 1; i < unroll2; i++ )
    {
      struct TRRels rels = getTransitionRelations(temp2[i-1]);
      Expr replaced_tr = rels.TR->body;
      /* outs() << "Before:\n"; */
      /* outs() << "\t" << *(rels.TR->body) << "\n"; */

      /* outs() << "Prev rule pointer: " << prev_rule << "\n"; */
      for ( int i = 0; i < rels.TR->srcVars.size(); i++ )
        replaced_tr = replaceAll(replaced_tr, rels.TR->srcVars[i], prev_rule->dstVars[i]);
      trans_exprs.push_back(replaced_tr);

      /* outs() << "After:\n"; */
      /* outs() << "\t" << *(replaced_tr) << "\n"; */

      prev_rule = rels.TR;
      bad_expr2 = rels.BAD;
      trans_dst2 = rels.TR;
    }

    /* outs () << "---- Transition relations ----\n"; */
    /* outs () << "Program 1: " << *(trans_hr1->body) << "\n"; */
    // outs() << "src vars: " << "\n";

    /* Rename INIT variables */
    Expr replaced_init1 = fact_hr1->body;
    for ( int i = 0; i < trans_hr1->srcVars.size(); i++ )
      replaced_init1 = replaceAll(replaced_init1, trans_hr1->dstVars[i], trans_hr1->srcVars[i]);
    // outs() << "TEST: " << *replaced_init1 << "\n";
    init_exprs.push_back(replaced_init1);

    /* outs () << "Program 2: " << *(trans_hr2->body) << "\n"; */
    // outs() << "src vars: " << "\n";
    Expr replaced_init2 = fact_hr2->body;
    for ( int i = 0; i < trans_hr2->srcVars.size(); i++ )
      replaced_init2 = replaceAll(replaced_init2, trans_hr2->dstVars[i], trans_hr2->srcVars[i]);
    // outs() << "TEST: " << *replaced_init2 << "\n";
    init_exprs.push_back(replaced_init2);

    /* Add BAD condition */

    outs() << "************************************************\n";

    /* outs() << "Length of init_exprs: " << init_exprs.size() << "\n"; */
    /* outs() << "Length of trans_exprs: " << trans_exprs.size() << "\n"; */
    /* outs() << "Length of bad_exprs: " << bad_exprs.size() << "\n"; */

    Expr combined_init = mk<AND>(init_exprs[0], init_exprs[1]);
    outs() << "Combined init: ";
    outs() << *combined_init << "\n\n";

    /* if ( utils.isSat(combined_init) ) */
    /*   outs() << "Combined init is SAT\n\n"; */

    /* Expr combined_trans = mk<AND>(trans_exprs[0], trans_exprs[1]); */
    Expr combined_trans = mknary<AND>(trans_exprs.begin(), trans_exprs.end());
    outs() << "Combined trans: ";
    outs() << *combined_trans << "\n\n";

    /* if ( utils.isSat(combined_trans) ) */
    /*   outs() << "Combined trans is SAT\n\n"; */

    // Expr combined_bad = mk<AND>(bad_exprs[0], bad_exprs[1]);
    /* Expr combined_bad = mk<AND>(bad_expr2->body, bad_expr2->body); */
    /* outs() << "Combined bad: "; */
    /* outs() << *combined_bad << "\n\n"; */

    /* if ( utils.isSat(combined_bad) ) */
    /*   outs() << "Combined bad is SAT\n\n"; */

    Expr combined_init_trans = mk<AND>(combined_init, combined_trans);
    outs () << "Combined init trans: ";
    outs () << *combined_init_trans << "\n\n";

    /* if ( utils.isSat(combined_init_trans) ) */
    /*   outs() << "Combined init_trans is SAT\n\n"; */

    if ( trans_hr1->srcVars.size() != trans_hr2->srcVars.size() )
    {
      outs() << "Different number of inputs; not equivalent!\n";
      return;
    }

    vector<Expr> eq_exprs;
    for ( int i = 0; i < trans_hr1->srcVars.size(); i++ )
    {
      eq_exprs.push_back(mk<EQ>(trans_hr1->srcVars[i], trans_hr2->srcVars[i]));
    }
    Expr eq_src;
    if (eq_exprs.size() > 1)
      eq_src = mknary<AND>(eq_exprs.begin(), eq_exprs.end());
    else
      eq_src = eq_exprs[0];
    outs() << "Equivalent inputs: \n";
    outs() << *eq_src << "\n";

    eq_exprs.clear();
    for ( int i = 0; i < trans_dst1->dstVars.size(); i++ )
    {
      eq_exprs.push_back(mk<EQ>(trans_dst1->dstVars[i], trans_dst2->dstVars[i]));
    }
    Expr eq_dst;
    if (eq_exprs.size() > 1)
      eq_dst = mknary<AND>(eq_exprs.begin(), eq_exprs.end());
    else
      eq_dst = eq_exprs[0];
    outs() << "Equivalent ouputs: \n";
    outs() << *eq_dst << "\n";

    eq_dst = mk<NEG>(eq_dst);
    Expr product_expr = mk<AND>(combined_init_trans, eq_src, eq_dst);
    outs() << "Product expr: " << *product_expr << "\n";

    if ( !utils.isSat(product_expr) )
      outs() << "Two programs are equivalent -- we are done!\n";
    else
      outs() << "Programs are not equivalent!\n";

  };
}

#endif
