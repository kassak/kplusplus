#pragma once

#include "ast.hpp"

namespace ast
{
   void ast2dot(std::ostream & ss, base_t::ptr_t const & n)
   {
      struct helper
      {
         helper(std::ostream & ss)
            : ss_(ss)
            , number_(0)
         {}

         std::string get_name(size_t x)
         {
            std::stringstream s;
            s << "v" << x;
            return s.str();
         }

         void null()
         {
            std::string my_name = get_name(number_++);
            ss_ << my_name << " [label=\"null\"];" << std::endl;
         }

         void visit(base_t::ptr_t const & p)
         {
            std::string my_name = get_name(number_++);

            ss_ << my_name << " [label=\"" << p->repr() << "\"];" << std::endl;

            for(base_t::ptr_t const & c : p->children)
            {
               std::string c_name = get_name(number_);
               if(c)
                  visit(c);
               else
                  null();
               ss_ << my_name << " -> " << c_name << "[];" << std::endl;
            }
         }

         std::ostream & ss_;
         size_t number_;
      };

      ss << "digraph g {" << std::endl;

      helper h(ss);
      h.visit(n);

      ss << "}" << std::endl;
   }
}
