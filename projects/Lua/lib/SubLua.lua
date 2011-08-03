-- Francis Maes, 02/08/2011
-- Include everything

require 'Language.LuaChunk' -- Should be renamed into "Internal"

require 'Subspecified'
require 'Derivable'
require 'Stochastic'

require 'Vector'
require 'Dictionary'
require 'Parser'
require 'Data'
require 'Context'
require 'Statistics'
require 'IterationFunction'



function print(...)
  context:information(...)
end