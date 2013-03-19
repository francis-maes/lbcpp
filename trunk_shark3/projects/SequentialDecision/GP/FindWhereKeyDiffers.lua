
require 'Stochastic'

local formula1 = lbcpp.GPExpression.createFromString("U(exp, B(sub, C(7), V(2)))")
local formula2 = lbcpp.GPExpression.createFromString("U(exp, B(sub, C(3), V(2)))")

local function makeOrder(score1, score2)
  if score1 < score2 then
    return -1
  elseif score1 > score2 then
    return 1
  else
    return 0
  end
end


for i=1,10000 do

  local t = math.pow(10, Stochastic.standardUniform() * 5)
  local rk1 = Stochastic.standardUniform()
  local sk1 = Stochastic.standardUniform()
  local tk1 = math.floor(t * Stochastic.standardUniform())
  local rk2 = Stochastic.standardUniform()
  local sk2 = Stochastic.standardUniform()
  local tk2 = math.floor(t * Stochastic.standardUniform())
  
  local score11 = formula1:compute(rk1, sk1, tk1, t)
  local score12 = formula1:compute(rk2, sk2, tk2, t)
  local order1 = makeOrder(score11, score12)

  local score21 = formula2:compute(rk1, sk1, tk1, t)
  local score22 = formula2:compute(rk2, sk2, tk2, t)
  local order2 = makeOrder(score21, score22)

  if not (order1 == order2) then
    print (rk1, sk1, tk1, t)
    print (rk2, sk2, tk2, t)
    print (score11, score12, score21, score22)
    break
  end

end