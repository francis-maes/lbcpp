require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'

filename = "C:/Projets/lbcpp/projects/Examples/Data/BinaryClassification/a1a.test"
labels = Dictionary.new()
examples = Data.load(Parser.libSVMClassification, 10, filename, labels)

print (#examples .. " examples, " .. labels:size() .. " labels")

for k,v in ipairs(examples) do 
  for k2,v2 in ipairs(v) do
    print (k2,v2)
  end
end

