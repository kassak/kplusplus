#include "kparser.hpp"
#include "tokens.h"
#include "klexer.hpp"

namespace ast
{
   namespace
   {
#include "parser.hpp"
#include "define_parser.hpp"

      struct program_;
      struct global_statement_list_;
      struct global_statement_;
      struct class_definition_;
      struct function_definition_;
      struct variable_definition_;
      struct variable_initialization_list;
      struct expression_;
      struct expression_atom_;

      DEF_PARSER_F(expression_atom_, lex, ({TOK_FLOAT, TOK_INT, TOK_ID}))
      {
         check_first<expression_atom_>(lex);
         switch(lex.token())
         {
         case TOK_FLOAT:
            return std::make_shared<value_t<double>>(lex.value<double>());
         case TOK_INT:
            return std::make_shared<value_t<long>>(lex.value<long>());
         case TOK_ID:
            return std::make_shared<variable_t>(lex.text());
         }
         throw std::logic_error("should not be");
      }

      DEF_PARSER(expression_, lex)
      {
         return parse<expression_atom_>(lex);
      }

      DEF_PARSER_F(variable_definition_, lex, ({TOK_ID}))
      {
         check_first<variable_definition_>(lex);
         std::string type = lex.text();
         base_t::ptr_t vars = std::make_shared<sequence_t>();
         lex.next();
         do
         {
            if(!vars->children.empty())
            {
               check_expected<variable_definition_>(lex, {TOK_COLON});
               lex.next();
            }
            check_expected<variable_definition_>(lex, {TOK_ID});
            std::string name = lex.text();
            base_t::ptr_t var = std::make_shared<variable_def_t>(type, name);
            vars->children.push_back(var);

            lex.next();
            check_expected<variable_definition_>(lex, {TOK_COLON, TOK_SEMICOLON,
                     TOK_ASSIGNMENT, TOK_BRACE_OPEN});
            switch(lex.token())
            {
            case TOK_COLON:
            case TOK_SEMICOLON:
               continue;
            case TOK_ASSIGNMENT:
               {
                  lex.next();
                  base_t::ptr_t expr = parse<expression_>(lex);
                  var->children.push_back(expr);
               }
            case TOK_BRACE_OPEN:
               {
                  lex.next();
                  base_t::ptr_t expr; // = parse<expression_>(lex);
                  var->children.push_back(expr);
               }
            }
         } while(lex.token() != TOK_SEMICOLON);
         return vars;
      }

      DEF_PARSER(function_definition_, lex)
      {
         return nullptr;
      }

      DEF_PARSER(class_definition_, lex)
      {
         return nullptr;
      }

      DEF_PARSER_F(global_statement_, lex, ({TOK_ID, TOK_CLASS}))
      {
         check_first<global_statement_>(lex);
         switch(lex.token())
         {
         case TOK_CLASS:
            return parse<class_definition_>(lex);
         case TOK_ID:
            {
               bool is_function;
               {
                  track_guard_t _(lex);
                  lex.next();
                  check_expected<global_statement_>(lex, TOK_ID);
                  lex.next();
                  check_expected<global_statement_>(lex, {TOK_BRACE_OPEN,
                           TOK_ASSIGNMENT, TOK_COLON, TOK_SEMICOLON});
                  lex.next();
                  if(lex.token() == TOK_BRACE_CLOSE)
                     is_function = true;
                  else if(lex.token() == TOK_ID)
                  {
                     lex.next();
                     is_function = (lex.token() == TOK_ID);
                  }
                  else
                     is_function = false;
               }
               if(is_function)
                  return parse<function_definition_>(lex);
               else
                  return parse<variable_definition_>(lex);
            }
         }
         throw std::logic_error("should not be");
      }

      DEF_PARSER(global_statement_list_, lex)
      {
         base_t::ptr_t seq = std::make_shared<sequence_t>();
         while(lex.next())
         {
            try
            {
               base_t::ptr_t stmt = parse<global_statement_>(lex);
               seq->children.push_back(stmt);
            }
            catch(unexpected_token_base & e)
            {
               std::cerr << "Error: " << e.what() << std::endl;
            }
         }
         return seq;
      }

      DEF_PARSER(program_, lex)
      {
         return parse<global_statement_list_>(lex);
      }

#include "undefine_parser.hpp"
   }

   base_t::ptr_t parse(std::istream & stream, config_t const & cfg)
   {
      klexer_t lex(stream);
      lex.configure(cfg);
      return parse<program_>(lex);
   }
}
