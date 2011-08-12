-- Francis Maes, 12/08/2011
-- Monte Carlo Tree Search

require '../ExpressionDiscovery/DecisionProblem'
require 'DiscreteBandit'
require 'Stochastic'
require 'Statistics'

subspecified function DecisionProblem.SinglePlayerMCTS(problem, x0)
  parameter numEpisodes = {default = 10000, min = 0}
  parameter indexFunction = {default = DiscreteBandit.ucb1C{2}}
  parameter verbose = {default = false}

  local function newNode(x, preAction)
    return {x = x, stats = Statistics.meanAndVariance(), preAction = preAction} -- U = vector of actions, subNodes = vector of subNodes
  end

  local root = newNode(x0 or problem.x0)

  local function score(node, subNode)
    local stats = subNode.stats
    if stats:getCount() == 0 then
      return math.huge
    else
      return indexFunction(stats:getMean(), stats:getStandardDeviation(), stats:getCount(), node.stats:getCount())
    end
  end

  local function isFinal(node)
    return problem.isFinal(node.x)
  end

  local function isLeaf(node)
    return isFinal(node) or node.subNodes == nil
  end

  local function select(node) -- returns the whole path from root to leaf
    local res = {}
    while not isLeaf(node) do
      table.insert(res, node)
      assert(node.U)
      assert(ipairs(node.U))
      local i = math.argmax(node.U, |index, action| score(node, node.subNodes[index]))
      node = node.subNodes[i]
    end
    table.insert(res, node)
    return res
  end

  local function simulate(x)
    return DecisionProblem.randomEpisode(problem, x)
  end

  local function expand(leaf)
    leaf.subNodes = {}
    leaf.U = problem.U(leaf.x)
    if leaf.U then   
      for index,action in ipairs(leaf.U) do
        table.insert(leaf.subNodes, newNode(problem.f(leaf.x, action), action))
      end
    else
      assert(problem.isFinal(leaf.x))
    end
  end

  local function backPropagate(nodeSequence, ret)
    if verbose then
      local str = ""
      for i,node in ipairs(nodeSequence) do
        node.stats:observe(ret)
        str = str .. " " .. tostring(node.stats)
      end
      context:information("Backpropagate: " .. str)
    else
      for i,node in ipairs(nodeSequence) do
        node.stats:observe(ret)
      end
    end
  end

  local bestScore = -math.huge
  local bestActionSequence
  local bestFinalState

  local function episode()
    -- select
    local nodeSequence = select(root)
    local leaf = nodeSequence[#nodeSequence]
    assert(isLeaf(leaf))
    if verbose then
      context:information("Select " .. problem.stateToString(leaf.x))
    end

    -- expand
    expand(leaf)

    -- simulate
    local finalState, actionSequence, score = simulate(leaf)
    if verbose then
      context:information("Simulate -> " .. problem.stateToString(finalState) .. " return = " .. score)
    end
  
    -- keep trace of best action sequence
    if score > bestScore then
      if verbose then
        context:information("New best: " .. problem.stateToString(finalState) .. " return = " .. score)
      end
      bestScore = score
      bestFinalState = finalState
      bestActionSequence = {}
      for i=2,#nodeSequence do
        table.insert(bestActionSequence, nodeSequence[i].preAction)
      end
      for i=1,#actionSequence do
        table.insert(bestActionSequence, actionSequence[i])
      end
    end

    -- backpropagate
    backPropagate(nodeSequence, score)
  end  

  for e=1,numEpisodes do
    if verbose then
      context:call("Episode " .. e, episode)
    else
      episode()
    end
  end
  if verbose then
    context:result("rootStats", root.stats)
  end
  return bestFinalState, bestActionSequence, bestScore
end