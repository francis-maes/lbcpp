
local traceFile = "BanditFormulaSearchTest.trace"
local numBests = 5
local outputFile = "BanditFormulaSearchTest.tex"


local trace = lbcpp.Object.fromFile(traceFile)
local traceRoot = trace.root.subItems[1]
local traceRootItems = traceRoot.subItems

local message = traceRootItems[1]
print (message.what)

local function getAttributes(node)
  local res = {}
  local n = #(node.results)
  for i=1,n do
    local pair = node.results[i]
    res[pair.first] = pair.second
  end
  return res
end

local function regretToLatex(regret)
  return string.format(regret > 10 and "%.1f" or (regret > 1 and "%.2f" or "%.3f"), regret)
end

local function formulaToLatex(formula)

  local variables = {"\\RewardMean", "\\RewardStddev", "\\PlayedCount", "\\TimeStep"} 
  local unaryOperators = {"id", "opp", "inv", "sqrt", "log", "exp", "abs"}
  local binaryOperators = {"add", "sub", "mul", "div", "min", "max", "pow", "lt"}
  local function exprToLatex(expr)
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
      local left = exprToLatex(expr.left)
      local right = exprToLatex(expr.right)
      if op == "add" then
        return left .. " + " .. right
      elseif op == "sub" then
        return left .. " - " .. right
      elseif op == "mul" then
        return left .. right
      elseif op == "div" then
        return "\\frac{" .. left .. "}{" .. right .. "}"
      else
        return op .. "(" .. left .. ", " .. right .. ")"
      end
    end
  end

  return "\n\\scriptsize{$" .. exprToLatex(formula) .. "$}"
end

local function entryToLatex(rank, formula, meanRegret, meanReward, playedCount, target)

  table.insert(target, formulaToLatex(formula))
  table.insert(target, regretToLatex(meanRegret))
  --table.insert(target, tostring(playedCount))
end

local function resultsNodeToLatex(results, target)

  local desc = results.description
  print (desc)

  for i=1,numBests do
    local attr = getAttributes(results.subItems[i])
    entryToLatex(attr.rank, attr.formula, attr.meanRegret, attr.meanReward, attr.playedCount, target[i])
  end  
end

local lines = {}
for i=1,numBests do
  table.insert(lines, {tostring(i)})
end

for i=2,#traceRootItems do
  resultsNodeToLatex(traceRootItems[i], lines)  
end


local f = assert(io.open(outputFile, "w"))
local function writeLine(line)
  f:write(line)
  f:write("\n")
  print (line)
end


writeLine("\\begin{tabular}{|c|c|c|c|c|c|c|c|c|}")
writeLine("\\hline")
writeLine(" & \\multicolumn{4}{c|}{Cumulative Regret} & \\multicolumn{4}{c|}{Simple Regret} \\\\")
writeLine(" & \\multicolumn{2}{c|}{Mean} & \\multicolumn{2}{c|}{Worst} & \\multicolumn{2}{c|}{Mean} & \\multicolumn{2}{c|}{Worst} \\\\")
writeLine("\\hline")
writeLine("Rank & Formula & Regret & Formula & Regret & Formula & Regret & Formula & Regret\\\\")
writeLine("\\hline")

for lineNumber,lineParts in ipairs(lines) do
  local line = table.concat(lineParts, " & ") .. "\\\\"
  writeLine(line)
end

writeLine("\\hline")
writeLine("\\end{tabular}")

f:close()
