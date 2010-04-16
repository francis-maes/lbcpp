//
// Copyright (C) 1997 Shigeru Chiba
// Copyright (C) 2000 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_Parser_hh_
#define Synopsis_Parser_hh_

#include <synopsis/PTree.hh>
#include <synopsis/SymbolFactory.hh>
#include <synopsis/Lexer.hh>  // francis
#include <synopsis/SymbolLookup.hh> // francis
#include <vector>

namespace Synopsis
{

//class Lexer; // francis #include <synopsis/Lexer.hh>, voir fin de fichier (ScopeGuard et declare<T>())
struct Token;

//. C++ Parser
//.
//. This parser is a LL(k) parser with ad hoc rules such as
//. backtracking.
//.
//. <name>() is the grammer rule for a non-terminal <name>.
//. opt_<name>() is the grammer rule for an optional non-terminal <name>.
//. is_<name>() looks ahead and returns true if the next symbol is <name>.
class Parser
{
public:
  //. RuleSet defines non-standard optional rules that can be chosen at runtime.
  enum RuleSet { CXX = 0x01, GCC = 0x02, MSVC = 0x04};

  //. Error is used to cache parse errors encountered during the execution
  //. of the parse method.
  class Error
  {
  public:
    virtual ~Error() {}
    virtual void write(std::ostream &) const = 0;
  };
  typedef std::vector<Error *> ErrorList;

  Parser(Lexer &lexer, SymbolFactory &symbols, int ruleset = CXX|GCC);
  // francis: virtual destructor
  virtual ~Parser();

  ErrorList const &errors() const { return my_errors;}

  //. Return the origin of the given pointer
  //. (filename and line number)
  unsigned long origin(const char *, std::string &) const;

  PTree::Node *parse();

  // francis
  virtual bool parse_user_statement(PTree::Node *&) {return false;}
  virtual bool parse_user_postfix_expr(PTree::Node *&) {return false;}
  virtual bool is_user_opt_member_spec(const Token& token) {return false;}

// francis: protected instead of private
protected:
  enum DeclKind { kDeclarator, kArgDeclarator, kCastDeclarator };
  enum TemplateDeclKind { tdk_unknown, tdk_decl, tdk_instantiation, 
			  tdk_specialization, num_tdks };

  struct ScopeGuard;
  friend struct ScopeGuard;

  //. A StatusGuard manages a tentative parse.
  //. All actions invoked after its instantiation
  //. will be rolled back in the destructor unless
  //. 'commit' has been called before.
  class StatusGuard
  {
  public:
    StatusGuard(Parser &);
    ~StatusGuard();
    void commit() { my_committed = true;}

  private:
    Lexer &                      my_lexer;
    char const *                 my_token_mark;
    ErrorList                    my_errors;
    Parser::ErrorList::size_type my_error_mark;
    bool                         my_committed;
  };
  friend class StatusGuard;


  bool mark_error();
  template <typename T>
  bool declare(T *);
  void show_message_head(const char*);

  bool definition(PTree::Node *&);
  bool null_declaration(PTree::Node *&);
  bool typedef_(PTree::Typedef *&);
  bool type_specifier(PTree::Node *&, bool, PTree::Encoding&);
  bool is_type_specifier();
  bool metaclass_decl(PTree::Node *&);
  bool meta_arguments(PTree::Node *&);
  bool linkage_spec(PTree::Node *&);
  bool namespace_spec(PTree::NamespaceSpec *&);
  bool namespace_alias(PTree::NamespaceAlias *&);
  bool using_directive(PTree::UsingDirective *&);
  bool using_declaration(PTree::UsingDeclaration *&);
  bool linkage_body(PTree::Node *&);
  bool template_decl(PTree::Node *&);
  bool template_decl2(PTree::TemplateDecl *&, TemplateDeclKind &kind);

  //. template-parameter-list:
  //.
  //. - template-parameter
  //. - template-parameter-list `,` template-parameter
  bool template_parameter_list(PTree::List *&);

  //. template-parameter:
  //.
  //. - type-parameter
  //. - parameter-declaration
  bool template_parameter(PTree::Node *&);

  //. type-parameter:
  //.
  //. - class identifier [opt]
  //. - class identifier [opt] `=` type-id
  //. - typename identifier [opt]
  //. - typename identifier [opt] `=` type-id
  //. - template  `<` template-parameter-list `>` class identifier [opt]
  //. - template  `<` template-parameter-list `>` class identifier [opt] `=` id-expression
  bool type_parameter(PTree::Node *&);
  
  //. GNU extension:
  //. extern-template-decl:
  //.
  //. - extern template declaration
  bool extern_template_decl(PTree::Node *&);

  bool declaration(PTree::Declaration *&);
  bool integral_declaration(PTree::Declaration *&, PTree::Encoding&, PTree::Node *, PTree::Node *, PTree::Node *);
  bool const_declaration(PTree::Declaration *&, PTree::Encoding&, PTree::Node *, PTree::Node *);
  bool other_declaration(PTree::Declaration *&, PTree::Encoding&, PTree::Node *, PTree::Node *, PTree::Node *);

  //. condition:
  //.
  //. - expression
  //. - type-specifier-seq declarator `=` assignment-expression
  bool condition(PTree::Node *&);

  bool is_constructor_decl();
  bool is_ptr_to_member(int);
  bool opt_member_spec(PTree::Node *&);

  //. storage-spec:
  //.
  //. - empty
  //. - static
  //. - extern
  //. - auto
  //. - register
  //. - mutable
  bool opt_storage_spec(PTree::Node *&);

  //. cv-qualifier:
  //.
  //. - empty
  //. - const
  //. - volatile
  bool opt_cv_qualifier(PTree::Node *&);
  bool opt_integral_type_or_class_spec(PTree::Node *&, PTree::Encoding&);
  bool constructor_decl(PTree::Node *&, PTree::Encoding&);
  bool opt_throw_decl(PTree::Node *&);
  
  //. [gram.dcl.decl]
  bool init_declarator_list(PTree::Node *&, PTree::Encoding&, bool, bool = false);
  bool init_declarator(PTree::Node *&, PTree::Encoding&, bool, bool);
  bool declarator(PTree::Node *&, DeclKind, bool,
		  PTree::Encoding&, PTree::Encoding&, bool,
		  bool = false);
  bool declarator2(PTree::Node *&, DeclKind, bool,
		   PTree::Encoding&, PTree::Encoding&, bool,
		   bool, PTree::Node **);
  bool opt_ptr_operator(PTree::Node *&, PTree::Encoding&);
  bool member_initializers(PTree::Node *&);
  bool member_init(PTree::Node *&);
  
  bool name(PTree::Node *&, PTree::Encoding&);
  bool operator_name(PTree::Node *&, PTree::Encoding&);
  bool cast_operator_name(PTree::Node *&, PTree::Encoding&);
  bool ptr_to_member(PTree::Node *&, PTree::Encoding&);
  bool template_args(PTree::Node *&, PTree::Encoding&);
  
  bool parameter_declaration_list_or_init(PTree::Node *&, bool&,
					  PTree::Encoding&, bool);
  bool parameter_declaration_list(PTree::Node *&, PTree::Encoding&);

  //. parameter-declaration:
  //.
  //. - decl-specifier-seq declarator
  //. - decl-specifier-seq declarator `=` assignment-expression
  //. - decl-specifier-seq abstract-declarator [opt]
  //. - decl-specifier-seq abstract-declarator [opt] `=` assignment-expression
  bool parameter_declaration(PTree::ParameterDeclaration *&, PTree::Encoding&);
  
  bool function_arguments(PTree::Node *&);
  bool designation(PTree::Node *&);
  bool initialize_expr(PTree::Node *&);
  
  bool enum_spec(PTree::EnumSpec *&, PTree::Encoding&);
  bool enum_body(PTree::Node *&);
  bool class_spec(PTree::ClassSpec *&, PTree::Encoding&);

  //. base-clause:
  //.
  //. - `:` base-specifier-list
  //.
  //. base-specifier-list:
  //.
  //. - base-specifier
  //. - base-specifier-list `,` base-specifier
  //.
  //. base-specifier:
  //.
  //. - virtual access-specifier [opt] `::` [opt] nested-name-specifier [opt] class-name
  //. - access-specifier virtual [opt] `::` [opt] nested-name-specifier [opt] class-name
  bool base_clause(PTree::Node *&);
  bool class_body(PTree::ClassBody *&);
  bool class_member(PTree::Node *&);
  bool access_decl(PTree::Node *&);
  bool user_access_spec(PTree::Node *&);
  
  //. expression:
  //.
  //. - assignment-expression
  //. - expression `,` assignment-expression
  bool expression(PTree::Node *&);

  //. assignment-expression:
  //.
  //. - conditional-expression
  //. - logical-or-expression assignment-operator assignment-expression
  //. - throw-expression
  bool assign_expr(PTree::Node *&);

  //. conditional-expression:
  //.
  //. - logical-or-expression
  //. - logical-or-expression `?` expression `:` assignment-expression
  bool conditional_expr(PTree::Node *&);

  //. logical-or-expression:
  //.
  //. - logical-and-expression
  //. - logical-or-expression `||` logical-and-expression
  bool logical_or_expr(PTree::Node *&);

  //. logical-and-expression:
  //.
  //. - inclusive-or-expression
  //. - logical-and-expr `&&` inclusive-or-expression
  bool logical_and_expr(PTree::Node *&);

  //. inclusive-or-expression:
  //.
  //. - exclusive-or-expression
  //. - inclusive-or-expression `|` exclusive-or-expression
  bool inclusive_or_expr(PTree::Node *&);

  //. exclusive-or-expression:
  //.
  //. - and-expression
  //. - exclusive-or-expression `^` and-expression
  bool exclusive_or_expr(PTree::Node *&);

  //. and-expression:
  //.
  //. - equality-expression
  //. - and-expression `&` equality-expression
  bool and_expr(PTree::Node *&);

  //. equality-expression:
  //.
  //. - relational-expression
  //. - equality-expression `==` relational-expression
  //. - equality-expression `!=` relational-expression
  bool equality_expr(PTree::Node *&);

  //. relational-expression:
  //.
  //. - shift-expression
  //. - relational-expression `<` shift-expression
  //. - relational-expression `>` shift-expression
  //. - relational-expression `<=` shift-expression
  //. - relational-expression `>=` shift-expression
  bool relational_expr(PTree::Node *&);

  //. shift-expression:
  //.
  //. - additive-expression
  //. - shift-expression `<<` additive-expression
  //. - shift-expression `>>` additive-expression
  bool shift_expr(PTree::Node *&);

  //. additive-expression:
  //.
  //. - multiplicative-expression
  //. - additive-expression `+` multiplicative-expression
  //. - additive-expression `-` multiplicative-expression
  bool additive_expr(PTree::Node *&);

  //. multiplicative-expression:
  //.
  //. - pm-expression
  //. - multiplicative-expression `*` pm-expression
  //. - multiplicative-expression `/` pm-expression
  //. - multiplicative-expression `%` pm-expression
  bool multiplicative_expr(PTree::Node *&);

  //. pm-expression:
  //.
  //. - cast-expression
  //. - pm-expression `.*` cast-expression
  //. - pm-expression `->*` cast-expression
  bool pm_expr(PTree::Node *&);

  //. cast-expression:
  //.
  //. - unary-expression
  //. - `(` type-id `)` cast-expression
  bool cast_expr(PTree::Node *&);

  //. type-id:
  //.
  //. - type-specifier-seq abstract-declarator [opt]
  bool type_id(PTree::Node *&);
  bool type_id(PTree::Node *&, PTree::Encoding&);

  //. unary-expression:
  //.
  //. - postfix-expression
  //. - `++` cast-expression
  //. - `--` cast-expression
  //. - unary-operator cast-expression
  //. - `sizeof` unary-expression
  //. - `sizeof` `(` unary-expression `)`
  //. - new-expression
  //. - delete-expression
  //.
  //. unary-operator:
  //.
  //. - `*`
  //. - `&`
  //. - `+`
  //. - `-`
  //. - `!`
  //. - `~`
  bool unary_expr(PTree::Node *&);

  //. throw-expression:
  //.
  //. - `throw` assignment-expression
  bool throw_expr(PTree::Node *&);

  //. sizeof-expression:
  //.
  //. - `sizeof` unary-expression
  //. - `sizeof` `(` type-id `)`
  bool sizeof_expr(PTree::Node *&);

  bool offsetof_expr(PTree::Node *&);

  //. typeid-expression:
  //.
  //. - typeid `(` type-id `)`
  //. - typeid `(` expression `)`
  bool typeid_expr(PTree::Node *&);
  bool is_allocate_expr(Token::Type);
  bool allocate_expr(PTree::Node *&);
  bool userdef_keyword(PTree::Node *&);
  bool allocate_type(PTree::Node *&);
  bool new_declarator(PTree::Declarator *&, PTree::Encoding&);
  bool allocate_initializer(PTree::Node *&);
  bool postfix_expr(PTree::Node *&);
  bool primary_expr(PTree::Node *&);
  bool typeof_expr(PTree::Node *&);
  bool userdef_statement(PTree::Node *&);
  bool var_name(PTree::Node *&);
  bool var_name_core(PTree::Node *&, PTree::Encoding&);
  bool is_template_args();
  
  //. function-body:
  //.
  //. - compound-statement
  bool function_body(PTree::Block *&);

  //. compound-statement:
  //.
  //. - `{` statement [opt] `}`
  bool compound_statement(PTree::Block *&, bool create_scope = false);
  bool statement(PTree::Node *&);

  //. if-statement:
  //.
  //. - `if` `(` condition `)` statement
  //. - `if` `(` condition `)` statement else statement
  bool if_statement(PTree::Node *&);

  //. switch-statement:
  //.
  //. - `switch` `(` condition `)` statement
  bool switch_statement(PTree::Node *&);

  //. while-statement:
  //.
  //. - `while` `(` condition `)` statement
  bool while_statement(PTree::Node *&);

  //. do-statement:
  //.
  //. - `do` statement `while` `(` condition `)` `;`
  bool do_statement(PTree::Node *&);
  bool for_statement(PTree::Node *&);

  //. try-block:
  //.
  //. - `try` compound-statement handler-seq
  //.
  //. handler-seq:
  //.
  //. - handler handler-seq [opt]
  //.
  //. handler:
  //.
  //. - `catch` `(` exception-declaration `)` compound-statement
  //.
  //. exception-declaration:
  //.
  //. - type-specifier-seq declarator
  //. - type-specifier-seq abstract-declarator
  //. - type-specifier-seq
  //. - `...`
  bool try_block(PTree::Node *&);
  
  bool expr_statement(PTree::Node *&);
  bool declaration_statement(PTree::Declaration *&);
  bool integral_decl_statement(PTree::Declaration *&, PTree::Encoding&, PTree::Node *, PTree::Node *, PTree::Node *);
  bool other_decl_statement(PTree::Declaration *&, PTree::Encoding&, PTree::Node *, PTree::Node *);

  bool maybe_typename_or_class_template(Token&);
  void skip_to(Token::Type token);
  
// francis: protected instead of private
protected:
  bool more_var_name();

  Lexer &         my_lexer;
  int             my_ruleset;
  SymbolFactory & my_symbols;
  //. Record whether the current scope is valid.
  //. This allows the parser to continue parsing even after
  //. it was unable to enter a scope (such as in a function definition
  //. with a qualified name that wasn't declared before).
  bool            my_scope_is_valid;
  ErrorList       my_errors;
  PTree::Node *   my_comments;
  //. If true, `>` is interpreted as ther greater-than operator.
  //. If false, it marks the end of a template-id or template-parameter-list.
  bool            my_gt_is_operator;
  bool            my_in_template_decl;
};

/*
** FRANCIS: moved from Parser.cc to Parser.hh to make it available in inherited Parsers
*/
class SyntaxError : public Parser::Error
{
public:
  SyntaxError(const std::string &f, unsigned long l, const std::string &c)
    : my_filename(f), my_line(l), my_context(c) {}
  virtual void write(std::ostream& ostr) const
  {
    // francis: pareil que gdb
    ostr << my_filename << ':' << my_line << ": error: "
            << "Syntax error before '" << my_context << "'";
  }
private:
  std::string   my_filename;
  unsigned long my_line;
  std::string   my_context;
};

class UndefinedSymbol : public Parser::Error
{
public:
  UndefinedSymbol(PTree::Encoding const &name,
		  std::string const &f, unsigned long l)
    : my_name(name), my_filename(f), my_line(l) {}
  virtual void write(std::ostream &os) const
  {
    // francis: pareil que gdb
    os << my_filename << ':' << my_line << ": error: "
      << "Undefined symbol : " << my_name.unmangled();
  }
private:
  PTree::Encoding my_name;
  std::string     my_filename;
  unsigned long   my_line;
};

class SymbolAlreadyDefined : public Parser::Error
{
public:
  SymbolAlreadyDefined(PTree::Encoding const &name, 
		       std::string const & f1, unsigned long l1,
		       std::string const & f2, unsigned long l2)
    : my_name(name), my_file1(f1), my_line1(l1), my_file2(f2), my_line2(l2) {}
  virtual void write(std::ostream &os) const
  {
    os << "Symbol already defined : definition of " << my_name.unmangled()
       << " at " << my_file1 << ':' << my_line1 << '\n'
       << "previously defined at " << my_file2 << ':' << my_line2 << std::endl;
  }
private:
  PTree::Encoding my_name;
  std::string     my_file1;
  unsigned long   my_line1;
  std::string     my_file2;
  unsigned long   my_line2;
};

class SymbolTypeMismatch : public Parser::Error
{
public:
  SymbolTypeMismatch(PTree::Encoding const &name, PTree::Encoding const &type)
    : my_name(name), my_type(type) {}
  virtual void write(std::ostream &os) const
  {
    os << "Symbol type mismatch : " << my_name.unmangled()
       << " has unexpected type " << my_type.unmangled() << std::endl;
  }
private:
  PTree::Encoding my_name;
  PTree::Encoding my_type;
};

template <typename T>
inline bool Parser::declare(T *t)
{
  // If the scope isn't valid, do nothing.
  if (!my_scope_is_valid) return true;
  try
  {
    my_symbols.declare(t);
  }
  catch (SymbolLookup::Undefined const &)
  {
    // TMP - francis
   /* std::string filename;
    unsigned long line = 0;
    if (e.ptree)
      line = my_lexer.origin(e.ptree->begin(), filename);

    my_errors.push_back(new UndefinedSymbol(e.name, filename, line));*/
  }
  catch (SymbolLookup::MultiplyDefined const &e)
  {
    std::string file1;
    unsigned long line1 = my_lexer.origin(e.declaration->begin(), file1);
    std::string file2;
    unsigned long line2 = my_lexer.origin(e.original->begin(), file2);
    my_errors.push_back(new SymbolAlreadyDefined(e.name,
						 file1, line1,
						 file2, line2));
  }
  catch (SymbolLookup::TypeError const &e)
  {
    my_errors.push_back(new SymbolTypeMismatch(e.name, e.type));
  }
  enum {max_errors = 10};
  return my_errors.size() < max_errors;
}

struct Parser::ScopeGuard
{
  template <typename T>
  ScopeGuard(Parser &p, T const *s)
    : parser(p), noop(s == 0), scope_was_valid(p.my_scope_is_valid)
  {
    // If s contains a qualified name this may fail, as the name
    // has to be declared before. We record the error but continune
    // processing.
    try { if (!noop) parser.my_symbols.enter_scope(s);}
    catch (SymbolLookup::Undefined const &e)
    {
      std::string filename;
      unsigned long line = 0;
      if (e.ptree)
	line = parser.my_lexer.origin(e.ptree->begin(), filename);

      std::cout << "Warning: undefined symbol " << e.name << std::endl;
      //parser.my_errors.push_back(new UndefinedSymbol(e.name, filename, line));
      parser.my_scope_is_valid = false;
      noop = true;
    }
  }
  ~ScopeGuard() 
  {
    if (!noop) parser.my_symbols.leave_scope();
    parser.my_scope_is_valid = scope_was_valid;
  }
  Parser & parser;
  bool     noop;
  bool     scope_was_valid;
};

}

#endif
