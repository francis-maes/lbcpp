-- Francis Maes, 12/08/2011
-- Monte Carlo Tree Search

require '../ExpressionDiscovery/DecisionProblem'
require 'DiscreteBandit'
require 'Stochastic'
require 'Statistics'

local function writeTreeToGraphML(problem, tree, filename)
  local f = assert(io.open(filename, "w"))
  f:write('<?xml version="1.0" encoding="UTF-8"?>\n')
  f:write('<graphml xmlns="http://graphml.graphdrawing.org/xmlns" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:y="http://www.yworks.com/xml/graphml" xmlns:yed="http://www.yworks.com/xml/yed/3" xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd">\n')
  f:write('  <key id="d1" for="node" yfiles.type="nodegraphics"/>\n')
  f:write('  <key id="d2" for="edge" yfiles.type="edgegraphics"/>\n')

  f:write('  <graph id="G" edgedefault="directed">\n')

  local nodeNumber = 1

  local function writeNode(node, text)
    local n = nodeNumber
    nodeNumber = nodeNumber + 1
    local id = "n" .. n
    f:write('<node id="' .. id .. '">\n')
    f:write('  <data key="d1"><y:ShapeNode><y:NodeLabel>' .. text .. '</y:NodeLabel></y:ShapeNode></data>\n')
    f:write('</node>\n')
    return id
  end

  local function writeEdge(source, target, text)
    f:write('<edge source="' .. source .. '" target="' .. target .. '">\n')
    f:write('  <data key="d2"><y:PolyLineEdge><y:EdgeLabel>' .. text .. '</y:EdgeLabel></y:PolyLineEdge></data>\n')
    f:write('</edge>\n')
  end

  local function writeRecursively(node, parentId)
    for i,u in ipairs(node.U) do
      local subNode = node.subNodes[i]
      if subNode then
        local subId = writeNode(subNode, problem.stateToString(subNode.x) .. "\n" .. tostring(subNode.stats))
        writeEdge(parentId, subId, problem.actionToString(node.U[i]))
        writeRecursively(subNode, subId)
      end
    end
  end

  local rootId = writeNode(tree, "root\n" .. tostring(tree.stats))
  writeRecursively(tree, rootId)

  f:write('  </graph>\n')
  f:write('</graphml>\n')
  f:close()
end

subspecified function DecisionProblem.SinglePlayerMCTS(problem, x0)
  parameter numEpisodes = {default = 10000, min = 0}
  parameter indexFunction = {default = DiscreteBandit.ucb1C{2}}
  parameter partialExpand = {default = false}
  parameter verbose = {default = false}

  local function newNode(x, preAction)
    local res = {x = x, stats = Statistics.meanAndVariance(), preAction = preAction, fullyExpanded = false, subNodes = {}, U = problem.U(x)}
    if res.U == nil then
      res.U = {}
    end
    return res
  end

  local root = newNode(x0 or problem.x0)

  local function score(node, subNode)
    assert(subNode)
    local stats = subNode.stats
    local res
    if stats:getCount() == 0 then
      res = math.huge
    else
      res = indexFunction(stats:getMean(), stats:getStandardDeviation(), stats:getCount(), node.stats:getCount())
    end
    return res
  end

  local function isFinal(node)
    return problem.isFinal(node.x)
  end

  local function select(node) -- returns the whole path from root to leaf
    local res = {}
    assert(node)
    while not isFinal(node) and node.fullyExpanded do
      table.insert(res, node)
      assert(#node.U > 0)
      local i = math.argmax(node.U, |index, action| score(node, node.subNodes[index]))
      assert(i ~= nil)
      node = node.subNodes[i]
      assert(node)
    end
    table.insert(res, node)
    return res
  end

  local function simulate(x)
    return DecisionProblem.randomEpisode(problem, x)
  end

  local function expand(leaf, action)
    assert(not leaf.fullyExpanded)
    leaf.fullyExpanded = true
    local res
    for index,u in ipairs(leaf.U) do
      local isSelectedAction = problem.actionToString(u) == problem.actionToString(action) -- FIXME: implement == between actions
      if leaf.subNodes[index] == nil then
        if not partialExpand or isSelectedAction then
          leaf.subNodes[index] = newNode(problem.f(leaf.x, u), u)
        else
          leaf.fullyExpanded = false
        end
      end
      if isSelectedAction then
        res = leaf.subNodes[index]
      end
    end
    return res
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
    assert(isFinal(leaf) or not leaf.fullyExpanded)
    if isFinal(leaf) then -- if we selected a final node, take the parent
      table.remove(nodeSequence, #nodeSequence)
      leaf = nodeSequence[#nodeSequence]
    end
    
    if verbose then
      local str = ""
      for i,node in ipairs(nodeSequence) do
        str = str .. " -> " .. problem.stateToString(node.x)
      end
      context:information("Select " .. str)
    end

    -- simulate
    local finalState, actionSequence, score = simulate(leaf.x)
    if verbose then
      context:information("Simulate -> " .. problem.stateToString(finalState) .. " return = " .. score)
    end

    -- expand
    if not leaf.fullyExpanded then
      local newNode = expand(leaf, actionSequence[1])
      table.insert(nodeSequence, newNode)
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
  --writeTreeToGraphML(problem, root, "mcts.graphml")
  return bestFinalState, bestActionSequence, bestScore
end