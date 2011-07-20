--! local t_G = typeimport 'luaanalyze.library.standard'
--! t_G.allow_write 'process_comments'
--! t_G.allow_write 'current_ast'
--! t_G.allow_write 'current_istart'
--! checkglobals()
--! checktypes()

local M = {}

local W = require "luaanalyze.walk"


-- Walks AST, processing Lua comments in AST.
local function process_comments(ast)

  local callback = {}

  -- processes Lua comments in current AST node.
  -- rcomments is list of comments from AST lineinfo (may be nil)
  function callback.oncomment(rcomments, ast, istart)
    if not rcomments then return end
    local allcode
    for i,rcomment in ipairs(rcomments) do
      local text = rcomment[1]
      local code = text:match('^!(.*)')
      if code then
        allcode = allcode and allcode .. '\n' .. code or code
      end
    end
    if allcode then
      _G.current_ast = ast
      _G.current_istart = istart
      local f = assert(loadstring(allcode))
      f()
      --print('DEBUG:type[' .. name .. ':' .. vtype .. ']')
      --local var = assert(scope[name],
      --  'lexical variable not defined: ' .. name)
      --var.ast.type = vtype
    end
  end

  W.find_vars(ast, 0, callback)
end
M.process_comments = process_comments
_G.process_comments = process_comments


--local function DEBUG(...)
--  local ts = {...}
--  for i,v in ipairs(ts) do
--    ts[i] = table.tostring(v,'nohash',60)
--  end
--  io.stderr:write(table.concat(ts, ' ') .. '\n')
--end

--local function NOTIMPL(s)
--  error('FIX - NOT IMPLEMENTED: ' .. s, 2)
--end


return M
