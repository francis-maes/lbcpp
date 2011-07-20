--! typeimport 'luaanalyze.library.standard'
--! checkglobals()
--! checktypes()

local M = {}

-- Walks AST tree, locating variable uses.
local function find_vars(ast, istart, callback)
  --   print('fv', ast.tag, ast[istart].tag)
  local function process(ast, istart)
    if ast.tag == nil or ast.tag == 'Do' then  -- block
      if callback.newscope then callback.newscope() end
      for i,stat_ast in ipairs(ast) do
        if not istart or i >= istart then
          if i == 1 and callback.oncomment then
            callback.oncomment(stat_ast.lineinfo.first.comments, ast, 0)
          end
          process(stat_ast)
          if callback.oncomment then
            callback.oncomment(stat_ast.lineinfo.last.comments, ast, i)
          end
        end
      end
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'Set' then
      local ls_ast, rs_ast = ast[1], ast[2]
      for i=1,#rs_ast do process(rs_ast[i]) end
      for i=1,#ls_ast do
        if ls_ast[i].tag == 'Id' then
          --FIX:if multiret
          if callback.setvar then callback.setvar(ls_ast[i], rs_ast[i]) end
        else
          process(ast[1][i])
        end
      end
    elseif ast.tag == 'While' then
      process(ast[1])
      if callback.newscope then callback.newscope() end
      process(ast[2])
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'Repeat' then
      if callback.newscope then callback.newscope() end
      process(ast[1])
      process(ast[2])
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'If' then
      for i=1,#ast do
        if i % 2 == 0 or i == #ast then
          if callback.newscope then callback.newscope() end
          process(ast[i])
          if callback.endscope then callback.endscope() end
        else
          process(ast[i])
        end
      end
    elseif ast.tag == 'Fornum' then
      if callback.newscope then callback.newscope() end
      if callback.newvar then callback.newvar(ast[1]) end
      for i=2,#ast do process(ast[i]) end
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'Forin' then
      local vars_ast, vals_ast = ast[1], ast[2]
      if callback.newscope then callback.newscope() end
      for i=1,#vars_ast do
        if callback.newvar then callback.newvar(vars_ast[i]) end
      end
      for i=1,#vals_ast do process(vals_ast[i]) end
      process(ast[#ast]) --FIX?
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'Local' then
      local ls_ast, rs_ast = ast[1], ast[2]
      if rs_ast then
        for i=1,#rs_ast do process(ast[2][i]) end
      end
      for i=1,#ls_ast do
        --FIX:multiret
        if callback.newvar then callback.newvar(ls_ast[i], rs_ast[i]) end
      end
    elseif ast.tag == 'Localrec' then
      local ls_ast, rs_ast = ast[1], ast[2]
      assert(#ls_ast == 1)
      for i=1,#ls_ast do
        --note: no multiret
        if callback.newvar then callback.newvar(ls_ast[i], rs_ast[i]) end
      end
      if rs_ast then
        for i=1,#rs_ast do process(rs_ast[i]) end
      end
      --metalua: elseif ast.tag == 'Goto' or ast.tag == 'Label' then
    elseif ast.tag == 'Return' then
      for i=1,#ast do process(ast[i]) end
    elseif ast.tag == 'Break' then
    elseif ast.tag == 'Nil' or ast.tag == 'Dots' or ast.tag == 'True'
      or ast.tag == 'False' or ast.tag == 'Number' or ast.tag == 'String'
    then
    elseif ast.tag == 'Function' then
      if callback.newscope then callback.newscope() end
      for i=1,#ast[1] do
        if ast[1][i].tag ~= 'Dots' then
          if callback.newvar then callback.newvar(ast[1][i]) end
        end
      end
      process(ast[2])
      if callback.endscope then callback.endscope() end
    elseif ast.tag == 'Table' then
      for i=1,#ast do process(ast[i]) end
    elseif ast.tag == 'Pair' then
      for i=1,2 do process(ast[i]) end
    elseif ast.tag == 'Op' then
      for i=2,#ast do process(ast[i]) end
    elseif ast.tag == 'Paren' then
      process(ast[1])
      -- metalua: elseif ast.tag == 'Stat' then
    elseif ast.tag == 'Call' then
      for i=1,#ast do process(ast[i]) end
    elseif ast.tag == 'Invoke' then
      process(ast[1])
      for i=3,#ast do process(ast[i]) end
    elseif ast.tag == 'Index' then
      for i=1,2 do process(ast[i]) end
      
    elseif ast.tag == 'Id' then
      if callback.getvar then callback.getvar(ast) end

      -- associate types in lexical definitions
      -- with `Id nodes.
      --      local name = ast[1]
      --      local varinfo = scope[name]
      --      if varinfo then
      --        local var_ast = varinfo.ast
      --        _ast_type[ast] = var_ast.type
      --      end


    else
      assert(false, ast.tag)
    end

    if callback.onnode then callback.onnode(ast) end
  end

  process(ast, istart)
end
M.find_vars = find_vars


-- Similar to find_vars but keeps track of lexical scope.
local function find_vars_scoped(ast, istart, clientcallback)
  local callback = {}

  local scope = {}
  local saved_scopes = {}

  function callback.newscope()
    table.insert(saved_scopes, scope)
    scope = setmetatable({}, {__index=scope})
  end

  function callback.endscope(saved_scope)
    scope = assert(saved_scopes[#saved_scopes])
    saved_scopes[#saved_scopes] = nil
  end

  function callback.newvar(ast, val_ast)
    assert(ast.tag == 'Id', ast)
    local name = ast[1]
    --print('newvar',name, val_ast)
    scope[name] = {ast = ast}
    if clientcallback.newvar then
      clientcallback.newvar(ast, val_ast)
    end
  end

  function callback.getvar(ast)
    if clientcallback.getvar then
      local name = ast[1]
      local def_ast = scope[name] and scope[name].ast
      clientcallback.getvar(ast, def_ast)
    end
  end

  function callback.setvar(ast, val_ast)
    if clientcallback.setvar then
      local name = ast[1]
      --print('setvarrr',name,scope[name])
      local def_ast = scope[name] and scope[name].ast
      clientcallback.setvar(ast, val_ast, def_ast)
    end
  end

  return find_vars(ast, istart, callback)
end
M.find_vars_scoped = find_vars_scoped

return M
