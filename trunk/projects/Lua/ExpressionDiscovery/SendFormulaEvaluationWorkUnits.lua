
local filename = "formulas2.txt"
local outputFilename = "banditScores2.txt"

local managerHostName = "monster24.montefiore.ulg.ac.be"
local managerPort = 1664
local resourceEstimator = lbcpp.Object.create("FixedResourceEstimator", 1, 5, 1)
local manager = context:connect(managerHostName, managerPort, "Lua", "Francis-PC", "jbecker@nic3", resourceEstimator)

local function createWorkUnit(formulaString)
  local workUnit = lbcpp.Object.create("ExecuteLuaString")
--  workUnit.code = "require '../ExpressionDiscovery/OptimizeFormulaConstants'\nresult = evaluateBanditFormulaStructure('" .. formulaString .. "')"
  workUnit.code = "print ('hello world') result = 51"
  workUnit.description = formulaString
  workUnit.inteluaDirectory = "/u/jbecker/LBC++/projects/Lua"
  return workUnit
end

local function workUnitFinished(workUnit, result)
  print ("WorkUnitFinished", workUnit.description, result)
  outputFile = assert(io.open(outputFilename, "at"))
  outputFile:write(tostring(result) .. " " .. workUnit.description .. "\n")
  outputFile:close()
end

--for line in io.lines(filename) do
--  manager:push(createWorkUnit(line), workUnitFinished, false)
--end

manager:push(createWorkUnit("coucou"), workUnitFinished)

--while true do
  context:sleep(60)
--end


--context:kill()

--local res = context:run(workUnit)
--print (res)



--[[
context:call("formula5", evaluateBanditFormulaStructure, "rk + __cst[1] / tk")
context:call("ucbC", evaluateBanditFormulaStructure, "rk + math.sqrt(__cst[2] * math.log(t) / tk)")
context:call("ucb2", evaluateBanditFormulaStructure, "rk + math.sqrt(2 * math.log(t) / tk)")

for C=0,3,0.1 do
  context:enter("C=" .. C)
  context:result("C", C)
  score = banditObjective(|rk,sk,tk,t| math.max(rk, C))
  context:result("score", score)
  context:leave()
end


context:call("KL-UCB(0)", banditObjective, DiscreteBandit.klUcb{c=0})
context:call("KL-UCB(3)", banditObjective, DiscreteBandit.klUcb{c=3})

context:call("Greedy", banditObjective, DiscreteBandit.greedy)
context:call("UCB1Tuned", banditObjective, DiscreteBandit.ucb1Tuned)
context:call("UCB1Tuned", banditObjective, DiscreteBandit.ucb1Tuned)
context:call("UCB1(2)", banditObjective, DiscreteBandit.ucb1C{C=2.0})
context:call("UCB1(1)", banditObjective, DiscreteBandit.ucb1C{C=1.0})


print ("score", score, "solution", solution)
]]