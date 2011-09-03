local filename = "results.txt"

local bestScore = math.huge
local bestSolution

for line in io.lines(filename) do
  local scoreString = string.match(line, "%d+%.%d*")
  if scoreString ~= nil then
    
    local remainingLine = string.sub(line, #scoreString + 1)
    
    local score = tonumber(scoreString)
    local solution = remainingLine
    --print (score, solution)
    if score < bestScore then
      bestScore = score
      bestSolution = solution
    end
  end
end

print ("Best score:", bestScore, "Best solution:", bestSolution)