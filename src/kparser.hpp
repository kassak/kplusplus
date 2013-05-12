#pragma once

#include "ast.hpp"
#include "config.hpp"

namespace ast
{
   base_t::ptr_t parse(std::istream & stream, config_t const & cfg);
}
