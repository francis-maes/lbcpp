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

bool lbcppgo::francisInit()
{
  // initialize lbcpp
  lbcpp::initialize("toto");
  setDefaultExecutionContext(singleThreadedExecutionContext());
  ExecutionContext& context = defaultExecutionContext();
  if (!lbcpp::importLibraryFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("libsequentialdecision.so"))))
    {
      std::cerr << "Could not import library libsequentialdecision.so" << std::endl;
      return false;
    }

  // load go decision maker
  goDecisionMaker = CompositeFunction::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("lbcppgo.model")));
  if (!goDecisionMaker)
  {
    std::cerr << "Could not load lbcppgo.model" << std::endl;
    return false;
  }
  if (!goDecisionMaker->initialize(context, goStateClass, positiveIntegerPairClass))
    {
      std::cerr << "Could not initialize goDecisionMaker" << std::endl;
      return false;
    }
  std::cerr << "LBCpp initialized." << std::endl;
  return true;
}

void lbcppgo::francisCompute(int boardSize, int board[361], double *patternValues, double values[361], int blackEatenStones, int whiteEatenStones, int color, int tour, int lastMove, int lastLastMove)
{
  memset(values, 0, sizeof (double) * 361);
  if (!goDecisionMaker)
    return;

  ExecutionContext& context = defaultExecutionContext();

  size_t size = (size_t)boardSize;

  GoStatePtr state = new GoState(T("mogoState"), size);
  GoBoardPtr stateBoard = state->getBoard();

  if (!(((color == 1) && (tour % 2 == 0)) || // black player's turn
	((color == 2) && (tour % 2 == 1)))) // white player's turn
    {
      std::cerr << "Warning: Color and turn number do not match" << std::endl;
      ++tour;
    }

  state->setTime(tour);
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

  DoubleMatrixPtr patternValuesMatrix = new DoubleMatrix(size, size);
  for (size_t i = 0; i < size; ++i)
    for (size_t j = 0; j < size; ++j)
    {
      int b = board[i * size + j];
      stateBoard->set(j, i, (b > 0 ? blackPlayer : (b < 0 ? whitePlayer : noPlayers)));
      patternValuesMatrix->setValue(i, j, patternValues[size + 3 + i * (size + 2) + j]);
    }


  // compute available actions
  state->recomputeAvailableActions();
  ContainerPtr actions;
  DenseDoubleVectorPtr scoreVector = GoPredict::computeScores(context, state, goDecisionMaker, actions);
  if (!scoreVector)
  {
    context.warningCallback(T("No predicted scores, setting all scores to zero"));
    return;
  }

  DoubleMatrixPtr scores = new DoubleMatrix(size, size, 0.0);
  // fill output scores
  size_t n = actions->getNumElements();
  size_t bestX, bestY;
  double bestScore = -DBL_MAX;
  for (size_t i = 0; i < n; ++i)
  {
    PositiveIntegerPairPtr position = actions->getElement(i).getObjectAndCast<PositiveIntegerPair>();
    if (position->getFirst() == size && position->getSecond() == size)
    {
      //std::cerr << "Pass Score: " << scoreVector->getValue(i) << std::endl; // FIXME: no way to forward it
    }
    else
    {
      size_t x = position->getFirst();
      size_t y = position->getSecond();
      double score = scoreVector->getValue(i);
      values[y * size + x] = score;
      scores->setValue(y, x, score);
      if (score > bestScore)
	bestScore = score, bestX = x, bestY = y;
    }
  }

  std::cerr << "Francis Compute: tour = " << tour << " color = " << color << " best score: " << bestScore << " best move: (" << bestX << ", " << bestY << ")" << std::endl;

  //#if 0
  // DBG: save state
  static int counter = 1;
  std::cerr << "Francis Compute: Saving to state" << counter << ".xml" << std::endl;
  PairPtr pair = new Pair(state, scores);
  pair->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("state") + String(counter) + T(".xml")));
  pair = new Pair(state, patternValuesMatrix);
  pair->saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("statePatt") + String(counter) + T(".xml")));
  ++counter;
  //#endif // 0
}
