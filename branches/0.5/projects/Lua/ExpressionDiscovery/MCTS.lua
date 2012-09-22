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
    f:write('  <data key="d1"><y:ShapeNode>')
    f:write('<y:Geometry height="30.0" width="150"/>')
    f:write('<y:NodeLabel>' .. text .. '</y:NodeLabel>')
    f:write('</y:ShapeNode></data>\n')
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
  parameter fullPathExpand = {default = false},
  parameter useMaxReward = {default = false},
  parameter verbose = {default = false},

  newNode = function (self, x, preAction, preReward)
    local res = {x = x, stats = Statistics.meanVarianceAndBounds(),
                 preAction = preAction, preReward = preReward or 0,
                 fullyExpanded = false, subNodes = {}, U = self.problem.U(x)}
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
  
    local function nodeIndex(node, subNode)
      assert(subNode)
      local stats = subNode.stats
      local res
      if stats:getCount() == 0 then
        res = math.huge
      else
        local r = useMaxReward and stats:getMaximum() or stats:getMean() 
        res = indexFunction(r, stats:getStandardDeviation(), stats:getCount(), node.stats:getCount())
      end
      return res
    end

    local function select(node) -- returns the whole path from root to leaf
      local nodeSequence = {}
      local score = 0
      assert(node)
      while not self:isFinal(node) and node.fullyExpanded do
        table.insert(nodeSequence, node)
        assert(#node.U > 0)
        local i = math.argmax(node.U, |index, action| nodeIndex(node, node.subNodes[index]))
        assert(i ~= nil)
        node = node.subNodes[i]
        score = score + node.preReward
        assert(node)
      end
      table.insert(nodeSequence, node)
      return score, nodeSequence, nodeSequence[#nodeSequence]
    end

    local function simulate(x)
      return DecisionProblem.randomEpisode(self.problem, x)
    end

    local function expand(leaf, action)
      assert(leaf)
      leaf.fullyExpanded = true
      local res
      if verbose then
        context:information("Expand " .. self.problem.stateToString(leaf.x))
      end
      local actionFound = false
      for index,u in ipairs(leaf.U) do
        local isSelectedAction = self.problem.actionToString(u) == self.problem.actionToString(action) -- FIXME: implement == between actions
        if leaf.subNodes[index] == nil then
          if not partialExpand or isSelectedAction then
            leaf.subNodes[index] = self:newNode(self.problem.f(leaf.x, u), u, self.problem.g(leaf.x, u))
          else
            leaf.fullyExpanded = false
          end
        end
        if isSelectedAction then
          res = leaf.subNodes[index]
          actionFound = true
        end
      end
      if not actionFound then
        context:information("Could not find action ", self.problem.actionToString(action))
        for index,u in ipairs(leaf.U) do
          context:information(" ==> action " .. self.problem.actionToString(u))
        end
      end
      assert(actionFound)
      assert(res)
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
    local selectScore, nodeSequence, leaf = select(self.root)
    assert(self:isFinal(leaf) or not leaf.fullyExpanded)
    if verbose then
      local str = ""
      for i,node in ipairs(nodeSequence) do
        str = str .. " -> " .. self.problem.stateToString(node.x)
      end
      context:information("Select " .. str .. " (score = " .. selectScore .. ")")
    end

    -- simulate
    local score, actionSequence, finalState = simulate(leaf.x)

    -- expand
    if not self:isFinal(leaf) then      
      if fullPathExpand then
        for i,action in ipairs(actionSequence) do
          leaf = expand(leaf, action)
          table.insert(nodeSequence, leaf)
        end
      else
        local newNode = expand(leaf, actionSequence[1])
        table.insert(nodeSequence, newNode)
      end
    end
 
    -- assemble select part and simulate part
    score = selectScore + score -- score of 'select' part + score of 'simulate' part
    for i=2,#nodeSequence do
      table.insert(actionSequence, i - 1, nodeSequence[i].preAction) -- insert first actions in actionSequence
    end
    if verbose then
      context:information("Simulate " .. 
        DecisionProblem.actionSequenceToString(self.problem, actionSequence) ..
        "(" .. self.problem.stateToString(finalState) .. ") ==> " ..
        score)
    end

    -- backpropagate
    backPropagate(nodeSequence, score)
    
    return score, actionSequence, finalState
  end,

  getTreeSize = function (self, tree)
    local res = 1
    for i,u in ipairs(tree.U) do
      node = tree.subNodes[i]
      if node then
        res = res + self:getTreeSize(node)
      end
    end
    return res
  end,

  finalizeIteration = function (self)
    context:result("tree size", self:getTreeSize(self.root))
  end,

  finalize = function(self)
   
    local test = self:bestActionSequence()
    for i,action in ipairs(test) do
      print (i, self.problem.actionToString(action)) 
    end
    print ("Tree Size: " .. self:getTreeSize(self.root))
    --writeTreeToGraphML(self.problem, self.root, "mcts.graphml")
  end
}