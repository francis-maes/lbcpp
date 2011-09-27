
require 'Statistics'

local maxSize = 4
local numTrials = 1
local numArmsInSampling = 127

local objective = lbcpp.Object.create("BanditFormulaObjective", 2, 10, 1.0, 1000)
local problem = lbcpp.Object.create("BanditFormulaSearchProblem", objective, numArmsInSampling)


results = {} -- numSamples -> {num formula classes Statistics, num invalid formulas Statistics}
numWorkUnits = 0
numWorkUnitsDone = 0

local function myCallback(workUnit, result)
  --print (workUnit.maxSize, workUnit.numSamples, workUnit.numFinalStates, workUnit.numFormulas, workUnit.numInvalidFormulas, workUnit.numFormulaClasses)
  local numSamples = workUnit.numSamples
  local r = results[numSamples]
  if r == nil then
    r = {Statistics.meanAndVariance(), Statistics.meanAndVariance()}
    results[numSamples] = r
  end
  r[1]:observe(workUnit.numFormulaClasses)
  r[2]:observe(workUnit.numInvalidFormulas)
  
  numWorkUnitsDone = numWorkUnitsDone + 1
end

local function pushWorkUnit(maxSize, numSamples, outputFileName)
  local workUnit = lbcpp.Object.create("GenerateUniqueFormulas", problem, maxSize, numSamples, outputFileName)
  numWorkUnits = numWorkUnits + 1
  context:push(workUnit, myCallback, true)
end

context:enter("Processing...")
for i=1,numTrials do
  for k,numSamples in ipairs({1,2,5,10,20,50,100,200,500}) do --,2000,5000,10000,20000,50000,100000}) do
    pushWorkUnit(maxSize, numSamples, "formulas/formulas" .. maxSize .. "_" .. numSamples .. ".txt")
  end
end

local prevDone = 0
while numWorkUnitsDone < numWorkUnits do
  if numWorkUnitsDone > prevDone then
    context:progress(numWorkUnitsDone, numWorkUnits, "Work units")
    prevDone = numWorkUnitsDone
  end
  --print ("sleep", numWorkUnits)
  context:sleep(1)
end
context:progress(numWorkUnitsDone, numWorkUnits, "Work units")
context:leave()

context:enter("Results")

local sortedResults = {}
for k,v in pairs(results) do table.insert(sortedResults, {k,v}) end
table.sort(sortedResults, |a,b| a[1] < b[1])

for i,kv in ipairs(sortedResults) do
  local k = kv[1]
  local v = kv[2]
  context:enter(tostring(k) .. " samples")
  context:result("samples", k)
  context:result("numFormulaClasses", v[1]:getMean())
  context:result("numFormulaClassesStddev", v[1]:getStandardDeviation())
  context:result("numInvalidFormulas", v[2]:getMean())
  context:result("numInvalidFormulasStddev", v[2]:getStandardDeviation())
  context:result("log(samples)", math.log(k))
  context:leave()
end

context:leave()



--context:waitUntilAllWorkUnitsAreDone()