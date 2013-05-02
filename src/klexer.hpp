#pragma once
#include <FlexLexer.h>
#include "tokens.h"
#include <boost/variant.hpp>
#include <stdexcept>
#include <string>

struct klexer_t : yyFlexLexer
{
   typedef
      boost::variant<double, long>
      value_t;

   int yylex();

   template<class T>
   T const & value() const;
protected:
   value_t value_;
};

template<class T>
T const & klexer_t::value() const
{
   if(const T * v = boost::get<T>(&value_))
      return *v;
   throw std::logic_error(std::string("value is not of type ") + typeid(T).name());
}
