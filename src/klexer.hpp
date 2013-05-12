#pragma once
#include "tokens.h"
#include "config.hpp"
#include <FlexLexer.h>
#include <boost/variant.hpp>

struct klexer_t;

struct track_guard_t
{
   track_guard_t(klexer_t & lex);
   ~track_guard_t();
private:
   klexer_t * lex_;
};

struct klexer_t : private yyFlexLexer
{
   friend struct track_guard_t;
   typedef
      boost::variant<double, long>
      value_t;
   klexer_t(std::istream& input);

   template<class T>
   T const & value() const;

   token_t token() const;
   std::string text() const;
   int line() const;

   bool next();

   config_t const & config() const;
   void configure(config_t const & cfg);
private:
   int yylex();

   void set_tracking(bool t);

   const value_t & pvalue() const;
private:
   struct state_t
   {
      state_t(std::string text, token_t token, value_t value, int lineno)
         : text(text)
         , token(token)
         , value(value)
         , lineno(lineno)
      {}

      std::string text;
      token_t token;
      value_t value;
      int lineno;
   };
protected:
   token_t token_;
   value_t value_;
   std::istream * input_;
   bool tracking_;
   config_t cfg_;
   std::list<state_t> track_, out_;
};

template<class T>
inline T const & klexer_t::value() const
{
   if(const T * v = boost::get<T>(&pvalue()))
      return *v;
   throw std::logic_error(std::string("value is not of type ") + typeid(T).name());
}
