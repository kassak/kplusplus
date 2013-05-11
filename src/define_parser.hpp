#pragma once

#define DEF_PARSER(R, L) template<> \
   inline ast::base_t::ptr_t parse<R>(klexer_t & L)

#define DEF_FIRST(R, F) template<> \
   inline first_t first_impl<R>(){return first_t F;}
