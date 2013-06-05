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
      struct class_statement_;
      struct class_definition_;
      struct function_definition_;
      struct variable_definition_;
      struct variable_initialization_list;
      struct expression_;
      struct expression_atom_;
      struct access_id_;
      struct argument_list_;
      struct argument_definition_list_;
      struct statements_block_;
      struct statement_;
      struct statement_or_block_;
      struct while_stmt_;
      struct for_stmt_;
      struct if_stmt_;
      struct return_stmt_;
      struct break_stmt_;
      struct continue_stmt_;

      FORWARD_PARSER(expression_);

      DEF_PARSER_F(argument_list_, lex, ({TOK_BRACE_OPEN}))
      {
         check_first<argument_list_>(lex);
         base_t::ptr_t seq = std::make_shared<arg_sequence_t>();
         lex.next();
         while(lex.token() != TOK_BRACE_CLOSE)
         {
            base_t::ptr_t arg = parse<expression_>(lex);
            seq->children.push_back(arg);
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
            lex.next(); //consume id
         }
         return var;
      }

      DEF_PARSER_F(expression_atom_, lex,
                   ({TOK_FLOAT, TOK_INT, TOK_ID, TOK_BRACE_OPEN, TOK_MINUS}))
      {
         check_first<expression_atom_>(lex);
         base_t::ptr_t res;
         switch(lex.token())
         {
         case TOK_MINUS:
            lex.next(); //consume -
            res = std::make_shared<binop_t>(binop_t::bo_minus);
            res->children.push_back(parse<expression_atom_>(lex));
            break;
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
         struct h
         {
            static binop_t::bo_t from_token(token_t tok)
            {
               switch(tok)
               {
               case TOK_ASSIGNMENT:
                  return binop_t::bo_assign;
               case TOK_PLUS:
                  return binop_t::bo_plus;
               case TOK_MINUS:
                  return binop_t::bo_minus;
               case TOK_MULT:
                  return binop_t::bo_mult;
               case TOK_DIVIDE:
                  return binop_t::bo_div;
               case TOK_LE:
                  return binop_t::bo_le;
               case TOK_GE:
                  return binop_t::bo_ge;
               case TOK_EQ:
                  return binop_t::bo_eq;
               case TOK_LT:
                  return binop_t::bo_lt;
               case TOK_GT:
                  return binop_t::bo_gt;
               default:
                  throw std::logic_error("should not be");
               }
            }

            static int priority(binop_t::bo_t op)
            {
               switch(op)
               {
               case binop_t::bo_assign:
                  return -1;
               case binop_t::bo_eq:
                  return 2;
               case binop_t::bo_le:
               case binop_t::bo_ge:
               case binop_t::bo_lt:
               case binop_t::bo_gt:
                  return 3;
               case binop_t::bo_plus:
                  return 4;
               case binop_t::bo_minus:
                  return 4;
               case binop_t::bo_mult:
                  return 5;
               case binop_t::bo_div:
                  return 6;
               default:
                  throw std::logic_error("should not be");
               }
            }

            static bool lassoc(binop_t::bo_t op)
            {
               switch(op)
               {
                  case binop_t::bo_assign:
                     return false;
                  default:
                     return true;
               }
            }
         };
         std::vector<base_t::ptr_t> atoms;
         std::vector<binop_t::bo_t> bops;
         atoms.push_back(parse<expression_atom_>(lex));
         static const first_t ops = {TOK_ASSIGNMENT, TOK_PLUS, TOK_MINUS,
                                     TOK_MULT, TOK_DIVIDE, TOK_LE, TOK_GE, TOK_EQ, TOK_LT, TOK_GT};
         while(ops.count(lex.token()))
         {
            binop_t::bo_t op = h::from_token(lex.token());
            int p = h::priority(op);
            //fold
            while(!bops.empty() && (h::priority(bops.back()) > p || (h::priority(bops.back()) == p && h::lassoc(op))))
            {
               binop_t::bo_t top = bops.back();
               bops.pop_back();
               base_t::ptr_t res = std::make_shared<binop_t>(top);
               res->children.push_back(atoms.back());
               atoms.pop_back();
               res->children.push_back(atoms.back());
               std::swap(res->children[0], res->children[1]);
               atoms.pop_back();
               atoms.push_back(res);
            }

            lex.next(); // consume op
            bops.push_back(op);
            atoms.push_back(parse<expression_atom_>(lex));
         }

         base_t::ptr_t res = atoms.back();
         for(int i = bops.size()-1; i >= 0 ; --i)
         {
            base_t::ptr_t new_res = std::make_shared<binop_t>(bops[i]);
            new_res->children.push_back(atoms[i]);
            new_res->children.push_back(res);
            res = new_res;
         }
         return res;
      }

      DEF_PARSER_F(variable_definition_, lex, ({TOK_ID}))
      {
         check_first<variable_definition_>(lex);
         std::string type = lex.text();
         base_t::ptr_t vars = std::make_shared<var_sequence_t>();
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

      DEF_PARSER_F(argument_definition_list_, lex, ({TOK_BRACE_OPEN}))
      {
         check_first<argument_definition_list_>(lex);
         lex.next();
         base_t::ptr_t seq = std::make_shared<arg_sequence_t>();
         while(lex.token() != TOK_BRACE_CLOSE)
         {
            std::string name, type;
            check_expected<argument_definition_list_>(lex, TOK_ID);
            type = lex.text();
            lex.next();
            check_expected<argument_definition_list_>(lex, TOK_ID);
            name = lex.text();
            lex.next();
            base_t::ptr_t arg = std::make_shared<variable_def_t>(type, name);
            seq->children.push_back(arg);
            if(lex.token() == TOK_COMMA)
            {
               lex.next();
               check_expected<argument_definition_list_>(lex, TOK_ID);
            }
            else
               check_expected<argument_definition_list_>(lex, TOK_BRACE_CLOSE);
         }
         check_expected<argument_definition_list_>(lex, TOK_BRACE_CLOSE);
         lex.next();//consume )

         return seq;
      }

      DEF_PARSER_F(while_stmt_, lex, ({TOK_WHILE}))
      {
         check_first<while_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<while_stmt_t>();
         lex.next(); // consume while
         check_expected<while_stmt_>(lex, TOK_BRACE_OPEN);
         lex.next(); // consume (
         stmt->children.push_back(parse<expression_>(lex));
         check_expected<while_stmt_>(lex, TOK_BRACE_CLOSE);
         lex.next(); // consume )
         stmt->children.push_back(parse<statement_or_block_>(lex));
         return stmt;
      }

      DEF_PARSER_F(if_stmt_, lex, ({TOK_IF}))
      {
         check_first<if_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<if_stmt_t>();
         lex.next(); // consume if
         check_expected<if_stmt_>(lex, TOK_BRACE_OPEN);
         lex.next(); // consume (
         stmt->children.push_back(parse<expression_>(lex));
         check_expected<if_stmt_>(lex, TOK_BRACE_CLOSE);
         lex.next(); // consume )
         stmt->children.push_back(parse<statement_or_block_>(lex));
         if(lex.token() == TOK_ELSE)
            {
               lex.next(); // consume else
               stmt->children.push_back(parse<statement_or_block_>(lex));
            }
         else
            stmt->children.push_back(nullptr);
         return stmt;
      }

      DEF_PARSER_F(for_stmt_, lex, ({TOK_FOR}))
      {
         check_first<for_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<for_stmt_t>();
         lex.next(); // consume for
         check_expected<for_stmt_>(lex, TOK_BRACE_OPEN);
         lex.next(); // consume (
         base_t::ptr_t for1;
         if(lex.token() != TOK_SEMICOLON)
         {
            if(lookahead(lex, {TOK_ID, TOK_ID}))
               for1 = parse<variable_definition_>(lex);
            else
               for1 = parse<expression_>(lex);
         }
         check_expected<for_stmt_>(lex, TOK_SEMICOLON);
         lex.next();
         base_t::ptr_t for2;
         if(lex.token() != TOK_SEMICOLON)
            for2 = parse<expression_>(lex);
         check_expected<for_stmt_>(lex, TOK_SEMICOLON);
         lex.next();
         base_t::ptr_t for3;
         if(lex.token() != TOK_BRACE_CLOSE)
            for3 = parse<expression_>(lex);
         check_expected<while_stmt_>(lex, TOK_BRACE_CLOSE);
         lex.next(); // consume )
         stmt->children.push_back(for1);
         stmt->children.push_back(for2);
         stmt->children.push_back(for3);
         stmt->children.push_back(parse<statement_or_block_>(lex));
         return stmt;
      }

      DEF_PARSER_F(return_stmt_, lex, ({TOK_RETURN}))
      {
         check_first<return_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<return_stmt_t>();
         lex.next(); // consume return
         if(lex.token() != TOK_SEMICOLON)
            stmt->children.push_back(parse<expression_>(lex));
         check_expected<return_stmt_>(lex, TOK_SEMICOLON);
         lex.next();
         return stmt;
       }

      DEF_PARSER_F(continue_stmt_, lex, ({TOK_CONTINUE}))
      {
         check_first<continue_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<continue_stmt_t>();
         lex.next(); // consume continue
         check_expected<continue_stmt_>(lex, TOK_SEMICOLON);
         lex.next();
         return stmt;
      }

      DEF_PARSER_F(break_stmt_, lex, ({TOK_BREAK}))
      {
         check_first<break_stmt_>(lex);
         base_t::ptr_t stmt = std::make_shared<break_stmt_t>();
         lex.next(); // consume break
         check_expected<break_stmt_>(lex, TOK_SEMICOLON);
         lex.next();
         return stmt;
      }

      DEF_PARSER(statement_, lex)
      {
         base_t::ptr_t stmt;
         switch(lex.token())
         {
         case TOK_SEMICOLON: //empty statement
            {
               stmt = std::make_shared<stmt_sequence_t>();
               lex.next();
               break;
            }
         case TOK_BLOCK_OPEN: //inner block
            {
               stmt = parse<statements_block_>(lex);
               break;
            }
         case TOK_ID: // expression or variable def
            {
               if(!lookahead(lex, {TOK_ID, TOK_ID}))
               {
                  stmt = parse<expression_>(lex);
                  check_expected<statement_>(lex, TOK_SEMICOLON);
                  lex.next(); //consume ;
               }
               else
               {
                  stmt = parse<variable_definition_>(lex);
                  check_expected<statement_>(lex, TOK_SEMICOLON);
                  lex.next(); //consume ;
               }
               break;
            }
         case TOK_FLOAT:
         case TOK_INT:
            {
               stmt = parse<expression_>(lex);
               check_expected<statement_>(lex, TOK_SEMICOLON);
               lex.next(); //consume ;
            }
         case TOK_WHILE: // while
            {
               stmt = parse<while_stmt_>(lex);
               break;
            }
         case TOK_FOR: // for
            {
               stmt = parse<for_stmt_>(lex);
               break;
            }
         case TOK_IF: // if
            {
               stmt = parse<if_stmt_>(lex);
               break;
            }
         case TOK_RETURN: // return
            {
               stmt = parse<return_stmt_>(lex);
               break;
            }
         case TOK_CONTINUE: // continue
            {
               stmt = parse<continue_stmt_>(lex);
               break;
            }
         case TOK_BREAK: // break
            {
               stmt = parse<break_stmt_>(lex);
               break;
            }
         default:
            throw unexpected_token<statement_>(lex, {});
         }
         return stmt;
      }

      DEF_PARSER_F(statements_block_, lex, ({TOK_BLOCK_OPEN}))
      {
         check_first<statements_block_>(lex);
         lex.next();

         base_t::ptr_t seq = std::make_shared<stmt_sequence_t>();

         while(lex.token() != TOK_BLOCK_CLOSE)
         {
            base_t::ptr_t stmt = parse<statement_>(lex);
            seq->children.push_back(stmt);
         }
         check_expected<statements_block_>(lex, TOK_BLOCK_CLOSE);
         lex.next();
         return seq;
      }

      DEF_PARSER(statement_or_block_, lex)
      {
         if(lex.token() == TOK_BLOCK_OPEN)
            return parse<statements_block_>(lex);
         else
         {
            base_t::ptr_t seq = std::make_shared<stmt_sequence_t>();
            seq->children.push_back(parse<statement_>(lex));
            return seq;
         }
      }

      DEF_PARSER_F(function_definition_, lex, ({TOK_ID}))
      {
         check_first<function_definition_>(lex);
         std::string name, type;
         type = lex.text();
         lex.next();
         check_expected<function_definition_>(lex, TOK_ID);
         name = lex.text();
         base_t::ptr_t foo = std::make_shared<function_def_t>(type, name);
         lex.next();
         foo->children.push_back(parse<argument_definition_list_>(lex));
         if(lex.token() == TOK_BLOCK_OPEN)
            foo->children.push_back(parse<statements_block_>(lex));
         else
         {
            check_expected<function_definition_>(lex, TOK_SEMICOLON);
            lex.next();
            foo->children.push_back(nullptr);
         }
         return foo;
      }

      DEF_PARSER_F(class_definition_, lex, ({TOK_CLASS}))
      {
         check_first<class_definition_>(lex);
         lex.next();
         check_expected<class_definition_>(lex, TOK_ID);
         std::string name = lex.text();
         lex.next();
         check_expected<class_definition_>(lex, TOK_BLOCK_OPEN);
         lex.next();

         base_t::ptr_t cdef = std::make_shared<class_def_t>(name);
         while(lex.token() != TOK_BLOCK_CLOSE)
         {
            cdef->children.push_back(parse<global_statement_>(lex));
            if(cdef->children.back()->node_type() == nt_function_def)
            {
               base_t::ptr_t const & as = cdef->children.back()->children[0];
               as->children.insert(
                   as->children.begin(),
                   std::make_shared<variable_def_t>(name, "this")
               );
            }
         }

         check_expected<class_definition_>(lex, TOK_BLOCK_CLOSE);
         lex.next();
         check_expected<class_definition_>(lex, TOK_SEMICOLON);
         lex.next();
         return cdef;
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
               if(lookahead(lex, {TOK_ID, TOK_ID, TOK_BRACE_OPEN, TOK_BRACE_CLOSE})
                  || lookahead(lex, {TOK_ID, TOK_ID, TOK_BRACE_OPEN, TOK_ID, TOK_ID}))
                  return parse<function_definition_>(lex);
               else
               {
                  base_t::ptr_t tmp = parse<variable_definition_>(lex);
                  check_expected<global_statement_>(lex, TOK_SEMICOLON);
                  lex.next();
                  return tmp;
               }
            }
         }
         throw std::logic_error("should not be");
      }

      DEF_PARSER_F(class_statement_, lex, ({TOK_ID, TOK_CLASS}))
      {
         check_first<class_statement_>(lex);
         switch(lex.token())
         {
         case TOK_ID:
            {
               if(lookahead(lex, {TOK_ID, TOK_ID, TOK_BRACE_OPEN, TOK_BRACE_CLOSE})
                  || lookahead(lex, {TOK_ID, TOK_ID, TOK_BRACE_OPEN, TOK_ID, TOK_ID}))
                  return parse<function_definition_>(lex);
               else
               {
                  base_t::ptr_t tmp = parse<variable_definition_>(lex);
                  check_expected<class_statement_>(lex, TOK_SEMICOLON);
                  lex.next();
                  return tmp;
               }
            }
         }
         throw std::logic_error("should not be");
      }

      DEF_PARSER(global_statement_list_, lex)
      {
         base_t::ptr_t seq = std::make_shared<stmt_sequence_t>();
         lex.next();
         while(lex.token())
         {
            try
            {
               base_t::ptr_t stmt = parse<global_statement_>(lex);
               seq->children.push_back(stmt);
            }
            catch(unexpected_token_base & e)
            {
               std::cerr << "Error: " << e.what() << std::endl;
               lex.next();
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
