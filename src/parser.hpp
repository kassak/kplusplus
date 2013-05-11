#pragma once

typedef
  std::unordered_set<int>
  first_t;

template<class R>
inline ast::base_t::ptr_t parse(klexer_t & lex);

template<class R>
inline first_t first_impl();

template<class R>
inline const first_t & first()
{
   static const first_t tmp = first_impl<R>();
   return tmp;
}

struct unexpected_token_base : std::runtime_error
{
   unexpected_token_base(std::string const & s)
      : std::runtime_error(s)
   {}
};

template<class R>
struct unexpected_token : unexpected_token_base
{
   unexpected_token(klexer_t const & lex)
      : unexpected_token_base(error_msg(lex))
   {
   }
private:
   static std::string error_msg(klexer_t const & lex)
   {
      std::stringstream ss;
      ss << "Unexpected token `" << lex.text() << "`" << " at line " << lex.line();
      return ss.str();
   }
};

template<class R>
inline void check_expected(klexer_t const & lex, first_t const & toks)
{
   if(!toks.count(lex.token()))
      throw unexpected_token<R>(lex);
}

template<class R>
inline void check_expected(klexer_t const & lex, token_t tok)
{
   check_expected<R>(lex, first_t({tok}));
}

template<class R>
inline void check_first(klexer_t const & lex)
{
   check_expected<R>(lex, first<R>());
}
/*
#define DEF_PARSER(rule) ast::base_t::ptr_t parse_##rule()
#define PARSE(rule) parse_##rule()
#define DEF_FIRST(rule, list) std::unordered_set<token_t> & rule##_first()\
   {\
      static std::unordered_set<token_t> tmp_ = list; \
      return tmp;\
   }
#define FIRST(rule) rule##_first()
#define TOKEN(name) token_t name = lexer_.lex()
#define CHECK_FIRST(rule, token) if(!FIRST(rule).count(token))\
      throw kparser_t::unexpected_token(#rule, token));

struct kparser_t
{
   kparser_t(std::istream & input)
      : lexer_(input)
   {
   }

   struct unexpected_token
      : std::runtime_error
   {
      unexpected_token(std::string const & rule, token_t surprise)
         : std::runtime_error()
      {
      }
   private:
      static std::string error_message(std::string const & rule, token_t surprise)
      {
         std::stringstream ss;
         ss << "Unexpected token `" + surprise + "` at rule `" << rule << "`";
      }
   };
private:
   DEF_PARSER(program)
   {
      return PARSE(global_statement_list);
   }

   DEF_FIRST(global_statement_list, {TOK_ID});
   DEF_PARSER(global_statement_list)
   {
      TOKEN(token);
      if(!token)
         return std::nullptr;
      CHECK_FIRST(global_statement_list, token);
      return PARSE(global_statement_list);
   }
private:
   klexer_t lexer_;
}

#undef DEF_PARSER
#undef PARSE
*/