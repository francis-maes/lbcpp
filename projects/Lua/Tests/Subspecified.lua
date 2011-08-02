require 'Subspecified'


toto = subspecified function (x)
  parameter constant1 = {default = 51}
  parameter constant2 = {default = 1}
  return constant1 + constant2 * x
end

t1 = toto{}
print (t1(x))

t2 = toto{constant1 = 8.6}
print (t2(x))

t2.saveToFile("t2.data")
t3 = toto.loadFromFile("t2.data")

subspecified function totoWrapper()
  parameter constant = {default = 8.6}
  return toto{constant1 = constant, constant2 = constant}
end

t3 = totoWrapper{8.6}

--energy = |totoFunction| totoFunction(x)
--objective = |c| energy(totoWrapper{c})

--[[ output should be as following:

function toto(parameters)
  return {
end

------------------------