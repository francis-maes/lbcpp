/*-----------------------------------------.---------------------------------.
| Filename: GoInterface.cpp                | Go Interface                    |
| Author  : Francis Maes                   |                                 |
| Started : 05/04/2011 16:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "GoInterface.h"
#include "GoProblem.h"
#include "GoPredictWorkUnit.h"
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

static CompositeFunctionPtr goDecisionMaker;

void lbcppgo::francisInit()
{
  // initialize lbcpp
  lbcpp::initialize("toto");
  ExecutionContext& context = defaultExecutionContext();
  if (!lbcpp::importLibraryFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("libsequential-decision.so"))))
  {
    std::cerr << "Could not load libsequential-decision.so" << std::endl;
    return;
  }

  // load go decision maker
  goDecisionMaker = CompositeFunction::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("lbcppgo.model")));
  if (!goDecisionMaker)
  {
    std::cerr << "Could not load lbcppgo.model" << std::endl;
    return;
  }
  if (!goDecisionMaker->initialize(context, goStateClass, positiveIntegerPairClass))
    return;
}

// FIXME: double **patternValues should be *patternValues
void lbcppgo::francisCompute(int board[361], double **patternValues, double values[361], int blackEatenStones, int whiteEatenStones, int color, int tour, int lastMove, int lastLastMove)
{
  memset(values, 0, sizeof (double) * 361);
  if (!goDecisionMaker)
    return;

  ExecutionContext& context = defaultExecutionContext();

  size_t size = 19; // FIXME: should be given as argument

  GoStatePtr state = new GoState(T("mogoState"), size);
  GoBoardPtr stateBoard = state->getBoard();

  state->setTime(tour);
  jassert(((color == 1) && (tour % 2 == 0)) || // black player's turn
          ((color == 2) && (tour % 2 == 1))); // white player's turn

  state->setWhitePrisonerCount(whiteEatenStones);
  state->setBlackPrisonerCount(blackEatenStones);

  PositiveIntegerPairVectorPtr previousActions = state->getPreviousActions();
  if (tour > 0) // FIXME: is this enough to check the validity of lastMove ?
  {
    size_t x = lastMove % (size + 2) - 1;
    size_t y = lastMove / (size + 2) - 1;
    previousActions->append(std::make_pair(x, y));
  }
  if (tour > 1)
  {
    size_t x = lastLastMove % (size + 2) - 1;
    size_t y = lastLastMove / (size + 2) - 1;
    previousActions->append(std::make_pair(x, y));
  }

  //DoubleMatrixPtr patternValuesMatrix = new DoubleMatrix(size, size);
  for (size_t i = 0; i < size; ++i)
    for (size_t j = 0; j < size; ++j)
    {
      int b = board[i * size + j];
      stateBoard->set(i, j, (b > 0 ? blackPlayer : (b < 0 ? whitePlayer : noPlayers)));
      //patternValuesMatrix->setValue(i, j, patternValues[size + 3 + i * (size + 2) + j]);
    }

  // FIXME: patternValuesMatrix is not used yet

  // compute available actions
  state->recomputeAvailableActions();
  ContainerPtr actions;
  DenseDoubleVectorPtr scoreVector = GoPredict::computeScores(context, state, goDecisionMaker, actions);
  if (!scoreVector)
  {
    context.warningCallback(T("No predicted scores, setting all scores to zero"));
    return;
  }

  // fill output scores
  size_t n = actions->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    PositiveIntegerPairPtr position = actions->getElement(i).getObjectAndCast<PositiveIntegerPair>();
    if (position->getFirst() == size && position->getSecond() == size)
    {
      std::cerr << "Pass Score: " << scoreVector->getValue(i) << std::endl; // FIXME: no way to forward it
    }
    else
    {
      size_t x = position->getFirst();
      size_t y = position->getSecond();
      values[x * size + y] = scoreVector->getValue(i);
    }
  }
}
