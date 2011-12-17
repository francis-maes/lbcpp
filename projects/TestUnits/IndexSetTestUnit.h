/*-----------------------------------------.---------------------------------.
| Filename: IndexSetTestUnit.h             | Index Set Test Unit             |
| Author  : Francis Maes                   |                                 |
| Started : 17/12/2011 16:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_TEST_UNIT_INDEX_SET_H_
# define LBCPP_TEST_UNIT_INDEX_SET_H_

# include <lbcpp/Data/IndexSet.h>
# include <lbcpp/Execution/TestUnit.h>

namespace lbcpp
{

class IndexSetTestUnit : public TestUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    IndexSetPtr indexSet = new IndexSet();//0, 100);
    for (size_t i = 0; i <= 200; i += 2)
      indexSet->append(i);

    IndexSetPtr subset = new IndexSet();
    subset->randomlyExpandUsingSource(context, 54, indexSet, false);
    context.resultCallback("hop1", subset->cloneAndCast<IndexSet>());
    checkEgality(context, subset->size(), (size_t)54);

    subset->randomlyExpandUsingSource(context, 60, indexSet);
    context.resultCallback("hop2", subset->cloneAndCast<IndexSet>());
    checkEgality(context, subset->size(), (size_t)60);

    subset->randomlyExpandUsingSource(context, 100, indexSet);
    context.resultCallback("hop3", subset->cloneAndCast<IndexSet>());
    checkEgality(context, subset->size(), (size_t)100);

    //for (IndexSet::iterator it = indexSet->begin(); it != indexSet->end(); ++it)
    //  context.informationCallback(String((int)*it));

    for (size_t i = 0; i < 1000; ++i)
    {
      IndexSetPtr subset = new IndexSet();
      for (size_t size = 10; size <= 100; size += 10)
      {
        subset->randomlyExpandUsingSource(context, size, indexSet, false);
        checkEgality(context, subset->size(), size);
      }
      std::set<size_t> dbg;
      for (IndexSet::const_iterator it = subset->begin(); it != subset->end(); ++it)
        dbg.insert(*it);
      checkEgality(context, dbg.size(), (size_t)100);
    }

    std::vector<ScalarVariableStatistics> stats(200);
    for (size_t i = 0; i < 100000; ++i)
    {
      IndexSetPtr subset = new IndexSet();
      //subset->randomlyExpandUsingSource(context, 50, indexSet, true);

      for (size_t size = 10; size <= 50; size += 10)
        subset->randomlyExpandUsingSource(context, size, indexSet, false);
      //subset->randomlyExpandUsingSource(context, 10, indexSet, false);
      //subset->randomlyExpandUsingSource(context, 20, indexSet, false);
      //subset->randomlyExpandUsingSource(context, 50, indexSet, true);
      std::set<size_t> dbg;
      for (IndexSet::const_iterator it = subset->begin(); it != subset->end(); ++it)
        dbg.insert(*it);
      checkEgality(context, dbg.size(), (size_t)50);
      for (size_t i = 0; i < 200; ++i)
        stats[i].push(dbg.find(i) == dbg.end() ? 0.0 : 1.0);
    }
    context.enterScope(T("Curve"));
    for (size_t i = 0; i < stats.size(); ++i)
    {
      context.enterScope(String(i));
      context.resultCallback(T("index"), i);
      context.resultCallback(T("mean"), stats[i].getMean());
      context.leaveScope();
    }
    context.leaveScope();
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_TEST_UNIT_INDEX_SET_H_
