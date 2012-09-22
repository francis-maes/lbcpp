local filename = "bandits_hor1000_size6.txt"

local bestScore = math.huge
local bestSolution

local allScores = {}

for line in io.lines(filename) do
  local scoreString = string.match(line, "%d+%.%d*")
  if scoreString ~= nil then
    
    local remainingLine = string.sub(line, #scoreString + 1)
    
    local score = tonumber(scoreString)
    local solution = remainingLine
    table.insert(allScores, {score=score, solution=solution})
    --print (score, solution)
    if score < bestScore then
      bestScore = score
      bestSolution = solution
    end
  end
end

table.sort(allScores, |a,b| a.score < b.score)

local i = 1
for k,v in ipairs(allScores) do
  print (v.solution .. " [" .. v.score .. "]")
  i = i + 1
  if i == 10 then break end
end

--print ("Best score:", bestScore, "Best solution:", bestSolution)