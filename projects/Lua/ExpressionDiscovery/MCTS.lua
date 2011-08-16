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

DecisionProblem.SinglePlayerMCTS = subspecified {

  parameter indexFunction = {default = DiscreteBandit.ucb1C{2}},
  parameter partialExpand = {default = false},
  parameter verbose = {default = false},

  newNode = function (self, x, preAction)
    local res = {x = x, stats = Statistics.meanVarianceAndBounds(), preAction = preAction, fullyExpanded = false, subNodes = {}, U = self.problem.U(x)}
    if res.U == nil then
      res.U = {}
    end
    return res
  end,

  initialize = function (self, problem)
    self.problem = problem
    self.root = self:newNode(problem.x0)
  end,

  isFinal = function (self, node)
    assert(node)
    assert(node.x)
    return self.problem.isFinal(node.x)
  end,

  bestActionSequence = function (self)
    local res = {}
    local node = self.root
    local depth = 1
   context:result("BestActionSequence " .. depth, node.stats)    
    while not self:isFinal(node) and node.fullyExpanded do
      assert(#node.U > 0)
      local i = math.argmax(node.U, |index, action| node.subNodes[index].stats:getMaximum())
      node = node.subNodes[i]
      depth = depth + 1
      context:result("BestActionSequence " .. depth, node.stats)
      table.insert(res, node.preAction)   
    end
    return res
  end,

  episode = function (self)
  
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

    local function select(node) -- returns the whole path from root to leaf
      local res = {}
      assert(node)
      while not self:isFinal(node) and node.fullyExpanded do
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
      return DecisionProblem.randomEpisode(self.problem, x)
    end

    local function expand(leaf, action)
      assert(not leaf.fullyExpanded)
      leaf.fullyExpanded = true
      local res
      for index,u in ipairs(leaf.U) do
        local isSelectedAction = self.problem.actionToString(u) == self.problem.actionToString(action) -- FIXME: implement == between actions
        if leaf.subNodes[index] == nil then
          if not partialExpand or isSelectedAction then
            leaf.subNodes[index] = self:newNode(self.problem.f(leaf.x, u), u)
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

    -- select
    local nodeSequence = select(self.root)
    local leaf = nodeSequence[#nodeSequence]
    assert(leaf)
    assert(self:isFinal(leaf) or not leaf.fullyExpanded)
    if self:isFinal(leaf) then -- if we selected a final node, take the parent
      if verbose then
        context:information("Selected final node " .. self.problem.stateToString(leaf.x))
      end
      table.remove(nodeSequence, #nodeSequence)
      leaf = nodeSequence[#nodeSequence]
    end
    
    if verbose then
      local str = ""
      for i,node in ipairs(nodeSequence) do
        str = str .. " -> " .. self.problem.stateToString(node.x)
      end
      context:information("Select " .. str)
    end

    -- simulate
    local score, actionSequence, finalState = simulate(leaf.x)
    for i=2,#nodeSequence do
      table.insert(actionSequence, i - 1, nodeSequence[i].preAction) -- insert first actions in actionSequence
    end
    if verbose then
      context:information("Simulate -> " .. self.problem.stateToString(finalState) .. " return = " .. score)
    end

    -- expand
    if not leaf.fullyExpanded then
      local newNode = expand(leaf, actionSequence[1])
      table.insert(nodeSequence, newNode)
    end

    -- backpropagate
    backPropagate(nodeSequence, score)
    
    return score, actionSequence, finalState
  end
}