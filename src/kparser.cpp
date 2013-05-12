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
      struct access_id_;
      struct argument_list_;

      FORWARD_PARSER(expression_);

      DEF_PARSER_F(argument_list_, lex, ({TOK_BRACE_OPEN}))
      {
         check_first<argument_list_>(lex);
         base_t::ptr_t seq = std::make_shared<sequence_t>();
         lex.next();
         while(lex.token() != TOK_BRACE_CLOSE)
         {
            base_t::ptr_t arg = parse<expression_>(lex);
            check_expected<argument_list_>(lex, {TOK_COMMA, TOK_BRACE_CLOSE});
            if(lex.token() == TOK_COMMA)
               lex.next();
         }
         lex.next(); // consume ')'
         return seq;
      }

      DEF_PARSER_F(access_id_, lex, ({TOK_ID}))
      {
         check_first<access_id_>(lex);
         base_t::ptr_t var = std::make_shared<variable_t>(lex.text());
         lex.next();
         base_t::ptr_t last = var;
         while(lex.token() == TOK_DOT)
         {
            lex.next();
            check_expected<access_id_>(lex, TOK_ID);
            base_t::ptr_t svar = std::make_shared<variable_t>(lex.text());
            last->children.push_back(svar);
            last = svar;
         }
         return var;
      }

      DEF_PARSER_F(expression_atom_, lex,
                   ({TOK_FLOAT, TOK_INT, TOK_ID, TOK_BRACE_OPEN}))
      {
         check_first<expression_atom_>(lex);
         base_t::ptr_t res;
         switch(lex.token())
         {
         case TOK_BRACE_OPEN:
            lex.next(); //consume (
            res = parse<expression_>(lex);
            check_expected<expression_atom_>(lex, TOK_BRACE_CLOSE);
            lex.next(); //consume )
            break;
         case TOK_FLOAT:
            res = std::make_shared<value_t<double>>(lex.value<double>());
            lex.next(); //consume float
            break;
         case TOK_INT:
            res = std::make_shared<value_t<long>>(lex.value<long>());
            lex.next(); //consume float
            break;
         case TOK_ID:
            {
               base_t::ptr_t aname = parse<access_id_>(lex);
               if(lex.token() == TOK_BRACE_OPEN)
               {
                  base_t::ptr_t foo = std::make_shared<function_call_t>(aname);
                  base_t::ptr_t args = parse<argument_list_>(lex);
                  foo->children.swap(args->children);
                  res = foo;
               }
               else
                  res = aname;
               break;
            }
         default:
            throw std::logic_error("should not be");
         }
         return res;
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
               check_expected<variable_definition_>(lex, {TOK_COMMA});
               lex.next();
            }
            check_expected<variable_definition_>(lex, {TOK_ID});
            std::string name = lex.text();
            base_t::ptr_t var = std::make_shared<variable_def_t>(type, name);
            vars->children.push_back(var);

            lex.next();
            check_expected<variable_definition_>(lex, {TOK_COMMA, TOK_SEMICOLON,
                     TOK_ASSIGNMENT, TOK_BRACE_OPEN});
            switch(lex.token())
            {
            case TOK_COMMA:
            case TOK_SEMICOLON:
               continue;
            case TOK_ASSIGNMENT:
               {
                  lex.next();
                  base_t::ptr_t expr = parse<expression_>(lex);
                  var->children.push_back(expr);
                  break;
               }
            case TOK_BRACE_OPEN:
               {
                  base_t::ptr_t args = parse<argument_list_>(lex);
                  base_t::ptr_t foo = std::make_shared<function_call_t>(
                     std::make_shared<variable_t>(type)
                  );
                  foo->children.swap(args->children);
                  var->children.push_back(foo);
                  break;
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
                           TOK_ASSIGNMENT, TOK_COMMA, TOK_SEMICOLON});
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
