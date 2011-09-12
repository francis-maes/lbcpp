
local filename = "formulas2.txt"
local outputFilename = "test.txt" --bandits_hor100_size4.txt"

local managerHostName = "monster24.montefiore.ulg.ac.be"
local managerPort = 1664
local resourceEstimator = lbcpp.Object.create("FixedResourceEstimator", 1, 48, 1)
local manager1 = context:connect(managerHostName, managerPort, "Lua", "Monster24", "jbecker@nic3", resourceEstimator)
local manager2 = context:connect(managerHostName, managerPort, "Lua", "Monster24", "fmaes@nic3", resourceEstimator)
local manager3 = context:connect(managerHostName, managerPort, "Lua", "Monster24", "amarcos@nic3", resourceEstimator)

local function createWorkUnit(formulaString)
  local workUnit = lbcpp.Object.create("ExecuteLuaString")
  workUnit.code = "require '../ExpressionDiscovery/OptimizeFormulaConstants'\nresult = evaluateBanditFormulaStructure('" .. formulaString .. "')"
  --workUnit.code = "print ('hello world') result = 51"
  workUnit.description = formulaString
  workUnit.inteluaDirectory = "/u/jbecker/LBC++/projects/Lua/lib"
  return workUnit
end

local numWaitingResults = 0

local function workUnitFinished(workUnit, result)
  print ("WorkUnitFinished", workUnit.description, result)
  numWaitingResults = numWaitingResults - 1
  outputFile = assert(io.open(outputFilename, "at"))
  outputFile:write(tostring(result) .. " " .. workUnit.description .. "\n")
  outputFile:close()
end

--manager:push(createWorkUnit("hop"), workUnitFinished)

for line in io.lines(filename) do
  numWaitingResults = numWaitingResults + 1
  local manager
  if numWaitingResults % 3 == 0 then
    manager = manager1
  elseif numWaitingResults % 3 == 1 then
    manager = manager2
  else
    manager = manager3
  end  
  manager:push(createWorkUnit(line), workUnitFinished, false)
end

while numWaitingResults > 0 do
  print ("Still waiting for " .. numWaitingResults .. " results")
  manager1:sleep(10)
  manager2:sleep(10)
  manager3:sleep(10)
end

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