#pragma once

#define DEF_FIRST(R, F) template<>                    \
   inline first_t first_impl<R>(){return first_t F;}

#define DEF_NAME(R, N) template<>                    \
   inline const char * rule_name<R>(){return N;}

#define DECL_PARSER(R, L)   template<>             \
   inline ast::base_t::ptr_t parse_impl<R>(klexer_t & L)

#define FORWARD_PARSER(R) DECL_PARSER(R, dummy)

#define DEF_PARSER_F(R, L, F)\
   DEF_NAME(R, #R)                                      \
   DEF_FIRST(R, F)                                      \
   DECL_PARSER(R, L)

#define DEF_PARSER(R, L)\
   DEF_NAME(R, #R)                               \
   DECL_PARSER(R, L)
