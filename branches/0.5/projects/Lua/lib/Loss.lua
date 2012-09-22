-- Francis Maes, 09/08/2011
-- Loss functions

--[[
Interface:

]]

Loss = {}

--
-- Discriminant Losses
--
function Loss.perceptron(derivable x)
  return math.max(-x, 0.0)
end

function Loss.hinge(derivable x)
  return math.max(1 - x, 0.0)
end

function Loss.logBinomial(derivable x)
  if x < -10 then -- avoid approximation errors in the exp(-x) formula
    return -x
  else
    return math.log(1 + math.exp(-x))
  end
end

function Loss.exponential(derivable x)
  return math.exp(-x)
end

--
-- Regression Losses
--
function Loss.square(derivable prediction, supervision)
  return (prediction - supervision)^2
end

function Loss.absolute(derivable prediction, ycorrect)
  return math.abs(prediction - supervision)
end

--
-- Binary Classification Loss
--
subspecified function Loss.binary(derivable prediction, supervision)
  parameter loss = {default = Loss.hinge}
  local sign = supervision == 2 and 1 or -1
  return loss(prediction * sign)
end