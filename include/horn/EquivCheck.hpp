#ifndef EQUIVHECK__HPP__
#define EQUIVHECK__HPP__

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
  inline struct TRRels getTransitionRelations( CHCs chc )
  {
    TRRels retval;
    for (auto &hr: chc.chcs)
    {
      // if hr is a TR()
      if ( !hr.isFact && !hr.isQuery  && hr.isInductive)
      {
	retval.TR =  &hr;
        /* outs () << *(trans_hr->body) << "\n"; */
      }

      // if hr is an INIT()
      else if ( hr.isFact && !hr.isQuery && !hr.isInductive )
      {
	retval.INIT =  &hr;
      }

      // if hr is a BAD()
      else if ( !hr.isFact && hr.isQuery  && !hr.isInductive)
	retval.BAD =  &hr;
    }

    return retval;
  }

  inline void equivCheck(string chcfile1, string chcfile2, int unroll1, int unroll2)
  {
    ExprFactory efac;
    EZ3 z3(efac);

    CHCs ruleManager1(efac, z3);
    ruleManager1.parse(chcfile1, "v_");
    checkPrerequisites(ruleManager1);
    outs () << "Program encoding #1:\n";
    ruleManager1.print();

    CHCs ruleManager2(efac, z3);
    ruleManager2.parse(chcfile2, "w_");
    checkPrerequisites(ruleManager2);
    outs () << "Program encoding #2:\n";
    ruleManager2.print();

    ExprFactory expr_factory;
    SMTUtils utils(expr_factory);

    // TODO: check equivalence between programs encoded in ruleManager1 and ruleManager2
    std::vector<Expr> init_exprs;
    std::vector<Expr> trans_exprs;
    std::vector<Expr> bad_exprs;

    HornRuleExt *trans_hr1;
    HornRuleExt *fact_hr1;
    HornRuleExt *trans_hr2;
    HornRuleExt *fact_hr2;

    struct TRRels trs1 = getTransitionRelations(ruleManager1);
    struct TRRels trs2 = getTransitionRelations(ruleManager2);

    trans_exprs.push_back(trs1.TR->body);
    trans_hr1 = trs1.TR;
    fact_hr1 = trs1.INIT;
    bad_exprs.push_back(trs1.BAD->body);

    trans_exprs.push_back(trs2.TR->body);
    trans_hr2 = trs2.TR;
    fact_hr2 = trs2.INIT;
    bad_exprs.push_back(trs2.BAD->body);

#if 0
    // Find parts of the CHC
    for (auto &hr: ruleManager1.chcs)
    {
      // if hr is a TR()
      if ( !hr.isFact && !hr.isQuery  && hr.isInductive)
      {
        trans_exprs.push_back(hr.body);
        trans_hr1 = &hr;
        /* outs () << *(trans_hr->body) << "\n"; */
      }

      // if hr is an INIT()
      else if ( hr.isFact && !hr.isQuery && !hr.isInductive )
      {
        fact_hr1 = &hr;
      }

      // if hr is a BAD()
      else if ( !hr.isFact && hr.isQuery  && !hr.isInductive)
        bad_exprs.push_back(hr.body);
    }
#endif

    for ( int i = 0; i < unroll1; i++ )
    {
      CHCs ruleManager(efac, z3);
    }


    outs () << "---- Transition relations ----\n";
    outs () << "Program 1: " << *(trans_hr1->body) << "\n";
    // outs() << "src vars: " << "\n";
    Expr replaced_init1 = fact_hr1->body;
    for ( int i = 0; i < trans_hr1->srcVars.size(); i++ )
      replaced_init1 = replaceAll(replaced_init1, trans_hr1->dstVars[i], trans_hr1->srcVars[i]);
    // outs() << "TEST: " << *replaced_init1 << "\n";
    init_exprs.push_back(replaced_init1);

#if 0
    for (auto &hr: ruleManager2.chcs)
    {
      // if hr is a TR()
      if ( !hr.isFact && !hr.isQuery  && hr.isInductive)
      {
        trans_exprs.push_back(hr.body);
        trans_hr2 = &hr;
        /* outs () << *(trans_hr->body) << "\n"; */
      }

      // basically if hr is an INIT()
      else if ( hr.isFact && !hr.isQuery && !hr.isInductive )
      {
        fact_hr2 = &hr;
      }

      // if hr is a BAD()
      else if ( !hr.isFact && hr.isQuery  && !hr.isInductive)
        bad_exprs.push_back(hr.body);
    }
#endif

    outs () << "Program 2: " << *(trans_hr2->body) << "\n";
    // outs() << "src vars: " << "\n";
    Expr replaced_init2 = fact_hr2->body;
    for ( int i = 0; i < trans_hr2->srcVars.size(); i++ )
      replaced_init2 = replaceAll(replaced_init2, trans_hr2->dstVars[i], trans_hr2->srcVars[i]);
    // outs() << "TEST: " << *replaced_init2 << "\n";
    init_exprs.push_back(replaced_init2);

#if 0
    for (auto &hr: ruleManager2.chcs)
    {
      // basically if hr is an INIT()
      if ( hr.isFact && !hr.isQuery && !hr.isInductive )
        init_exprs.push_back(hr.body);

      // if hr is a TR()
      else if ( !hr.isFact && !hr.isQuery && hr.isInductive)
        trans_exprs.push_back(hr.body);

      // if hr is a BAD()
      else if ( !hr.isFact && hr.isQuery  && !hr.isInductive)
        bad_exprs.push_back(hr.body);
    }
#endif

    outs() << "************************************************\n";

    outs() << "Length of init_exprs: " << init_exprs.size() << "\n";
    outs() << "Length of trans_exprs: " << trans_exprs.size() << "\n";
    outs() << "Length of bad_exprs: " << bad_exprs.size() << "\n";

    Expr combined_init = mk<AND>(init_exprs[0], init_exprs[1]);
    outs() << "Combined init: ";
    outs() << *combined_init << "\n\n";

    if ( utils.isSat(combined_init) )
      outs() << "Combined init is SAT\n\n";

    Expr combined_trans = mk<AND>(trans_exprs[0], trans_exprs[1]);
    outs() << "Combined trans: ";
    outs() << *combined_trans << "\n\n";

    if ( utils.isSat(combined_trans) )
      outs() << "Combined trans is SAT\n\n";

    Expr combined_bad = mk<AND>(bad_exprs[0], bad_exprs[1]);
    outs() << "Combined bad: ";
    outs() << *combined_bad << "\n\n";

    if ( utils.isSat(combined_bad) )
      outs() << "Combined bad is SAT\n\n";

    Expr combined_init_trans = mk<AND>(combined_init, combined_trans);
    outs () << "Combined init trans: ";
    outs () << *combined_init_trans << "\n\n";

    if ( utils.isSat(combined_init_trans) )
      outs() << "Combined init_trans is SAT\n\n";

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
    /* outs() << "Equivalent inputs: \n"; */
    /* outs() << *eq_src << "\n"; */

    eq_exprs.clear();
    for ( int i = 0; i < trans_hr1->dstVars.size(); i++ )
    {
      eq_exprs.push_back(mk<EQ>(trans_hr1->dstVars[i], trans_hr2->dstVars[i]));
    }
    Expr eq_dst;
    if (eq_exprs.size() > 1)
      eq_dst = mknary<AND>(eq_exprs.begin(), eq_exprs.end());
    else
      eq_dst = eq_exprs[0];
    /* outs() << "Equivalent ouputs: \n"; */
    /* outs() << *eq_dst << "\n"; */

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
