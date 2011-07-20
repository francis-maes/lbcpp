--! local t_G = typeimport 'luaanalyze.library.standard'
--! t_G.add_field 'mlp'
--! checkglobals()
--! checktypes()


-- note: includes gg/mlp Lua parsing Libraries taken from Metalua.
require "lexer"
require "gg"
require "mlp_lexer"
require "mlp_misc"
require "mlp_table"
require "mlp_meta"
require "mlp_expr"
require "mlp_stat"
require "mlp_ext"
mlc = {} -- make gg happy
local mlp = assert(_G.mlp)


-- Converts Lua source string to Lua AST (via mlp/gg)
local function string_to_ast(src, filename)
  local  lx  = mlp.lexer:newstream (src, filename)
  local  ast = mlp.chunk(lx)
  return ast
end

return string_to_ast