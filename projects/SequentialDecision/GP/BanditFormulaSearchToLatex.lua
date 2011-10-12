
local traceFile = "BanditFormulaSearchTest.trace"
local numBests = 5
local outputFile = "BanditFormulaSearchTest.tex"


local function getAttributes(node)
  local res = {}
  local n = #(node.results)
  for i=1,n do
    local pair = node.results[i]
    res[pair.first] = pair.second
  end
  return res
end

local function findSubNode(node, description)
  local n = #(node.subItems)
  for i=1,n do
    if node.subItems[i]:toShortString() == description then
      return node.subItems[i]
    end
  end
  return nil
end

local trace = lbcpp.Object.fromFile(traceFile)
local traceRoot = trace.root.subItems[1]
local traceRootItems = traceRoot.subItems

local message = traceRootItems[1]
print (message.what)
local rootAttributes = getAttributes(traceRoot)


local function regretToLatex(regret)
  return string.format(regret > 10 and "%.1f" or (regret > 1 and "%.2f" or "%.3f"), regret)
end

local function formulaToLatex(formula)

  local exprToLatex

  local variables = {"\\RewardMean", "\\RewardStddev", "\\PlayedCount", "\\TimeStep"} 
  local unaryOperators = {"id", "opp", "inv", "sqrt", "ln", "exp", "abs"}
  local binaryOperators = {"add", "sub", "mul", "div", "min", "max", "pow", "lt"}

  local function exprToLatexWithParent(expr)
    local res = exprToLatex(expr)
    if expr.className == "BinaryGPExpression" then
      local op = binaryOperators[1+expr.op]
      if op == "add" or op == "sub" then
        res = "(" .. res .. ")"
      end
    end
    return res
  end  

  exprToLatex =  function (expr)
    local cn = expr.className
    if cn == "ConstantGPExpression" then
      return tostring(expr.value)
    elseif cn == "VariableGPExpression" then
      return variables[1+expr.index]
    elseif cn == "UnaryGPExpression" then
      local op = unaryOperators[1+expr.pre]
      local sub = exprToLatex(expr.expr)
      if op == "sqrt" then
        return "\\sqrt{" .. sub .. "}"
      elseif op == "opp" then
        return "-" .. sub
      else
        return op .. "(" .. sub .. ")"
      end
    elseif cn == "BinaryGPExpression" then
      local op = binaryOperators[1+expr.op]
      local left = exprToLatexWithParent(expr.left)
      local right = exprToLatexWithParent(expr.right)
      if op == "add" then
        return left .. " + " .. right
      elseif op == "sub" then
        return left .. " - " .. right
      elseif op == "mul" then
        if expr.right.className == "ConstantGPExpression" then
          return right .. " " .. left
        else
          return left .. " " .. right
        end
      elseif op == "div" then
        return left .. " / " .. right
        --return "\\frac{" .. left .. "}{" .. right .. "}"
      else
        return op .. "(" .. exprToLatex(expr.left) .. ", " .. exprToLatex(expr.right) .. ")" -- no parenthesis
      end
    end
  end

  return "$" .. exprToLatex(formula) .. "$"
end


local function baselineToLatex(baseline)
  if baseline == "uniform" then
    return "\\textsc{Uniform}"
  elseif baseline == "greedy" then
    return "\\textsc{Greedy}"
  elseif baseline == "ucb1(2)" then
    return "\\ucbone"
  elseif baseline == "ucb1Bernoulli" then
    return "\\ucbonetuned"
  elseif baseline == "klUcb(0)" then
    return "\\klucb"
  else
    return baseline
  end
end

local function entryToLatex(attributes, target, doBaselines)

  if doBaselines then
    table.insert(target, baselineToLatex(attributes.baseline))
  else
    table.insert(target, formulaToLatex(attributes.formula))
  end
  table.insert(target, regretToLatex(attributes.meanRegret))
  table.insert(target, regretToLatex(attributes.regretStddev))
  if doBaselines then
    table.insert(target, "")
  else
    table.insert(target, tostring(attributes.playedCount))
  end
end

local function resultsNodeToLatex(results, target, doBaselines, count)

  local desc = results.description
  print ("Processing: " .. desc)

  local first = doBaselines and 9 or 0
  for i=1,count do
    local attr = getAttributes(results.subItems[first + i])
    entryToLatex(attr, target[i], doBaselines)
  end  
end

local function resultsNodePairToLatex(results1, results2, doBaselines, count)

  local lines = {}
  for i=1,count do
    local rank = doBaselines and "" or tostring(i)
    table.insert(lines, {rank})
  end
  resultsNodeToLatex(results1, lines, doBaselines, count)
  resultsNodeToLatex(results2, lines, doBaselines, count)
  return lines 
end


local f = assert(io.open(outputFile, "w"))
local function writeLine(line)
  f:write(line)
  f:write("\n")
  print (">>" .. line)
end

writeLine("\\begin{tabular}{|c||c|cc|c||c|cc|c|}")
writeLine("\\hline")
writeLine(" & \\multicolumn{4}{c||}{Cumulative Regret} & \\multicolumn{4}{c|}{Simple Regret} \\\\")
writeLine("\\hline")
writeLine("Rank & Formula & Regret & stddev. & $\\PlayedCount$ & Formula & Regret & stddev. & $\\PlayedCount$ \\\\")
writeLine("\\hline")

local horizon = rootAttributes.minHorizon

while horizon <= rootAttributes.maxHorizon do
  
  writeLine("\\multicolumn{9}{|c|}{Horizon " .. horizon .. "} \\\\")
  writeLine("\\hline")
  local resultsNode1 = findSubNode(traceRoot, "Horizon " .. horizon .. " Cumulative Regret")
  local resultsNode2 = findSubNode(traceRoot, "Horizon " .. horizon .. " Simple Regret")
  assert(resultsNode1)
  assert(resultsNode2)
  local lines = resultsNodePairToLatex(resultsNode1, resultsNode2, false, numBests)
  for lineNumber,lineParts in ipairs(lines) do
    local line = table.concat(lineParts, " & ") .. "\\\\"
    writeLine(line)
  end
  writeLine("\\hline")

  lines = resultsNodePairToLatex(resultsNode1, resultsNode2, true, 5)
  for lineNumber,lineParts in ipairs(lines) do
    local line = table.concat(lineParts, " & ") .. "\\\\"
    writeLine(line)
  end
  writeLine("\\hline")

  horizon = horizon * 10
end

writeLine("\\end{tabular}")

f:close()