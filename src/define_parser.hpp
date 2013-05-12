#pragma once

#define DEF_FIRST(R, F) template<>                    \
   inline first_t first_impl<R>(){return first_t F;}

#define DEF_NAME(R, N) template<>                    \
   inline const char * rule_name<R>(){return N;}

#define DEF_PARSER_F(R, L, F)\
   DEF_NAME(R, #R)                                      \
   DEF_FIRST(R, F)                                      \
   template<>                                           \
   inline ast::base_t::ptr_t parse<R>(klexer_t & L)

#define DEF_PARSER(R, L)\
   DEF_NAME(R, #R)                               \
   template<>                                       \
   inline ast::base_t::ptr_t parse<R>(klexer_t & L)
