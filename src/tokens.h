#pragma once

enum token_t
{
   TOK_EOF = 0,

   TOK_INT = 10,
   TOK_FLOAT,
   TOK_ID,

   TOK_BREAK,
   TOK_CONTINUE,
   TOK_RETURN,
   TOK_FOR,
   TOK_WHILE,
   TOK_IF,
   TOK_ELSE,
   TOK_CLASS,

   TOK_DOT,
   TOK_COMMA,
   TOK_SEMICOLON,

   TOK_LE,
   TOK_GE,
   TOK_EQ,
   TOK_NEQ,
   TOK_GT,
   TOK_LT,
   TOK_ASSIGNMENT,
   TOK_PLUS,
   TOK_MINUS,
   TOK_MULT,
   TOK_DIVIDE,

   TOK_BLOCK_OPEN,
   TOK_BLOCK_CLOSE,
   TOK_BRACE_OPEN,
   TOK_BRACE_CLOSE,

   TOK_CHAR_LITERAL,
   TOK_STRING_LITERAL
};

inline std::string repr(token_t tok)
{
   static std::unordered_map<int, std::string> mp = {
      {TOK_EOF,   "<EOF>"},

      {TOK_INT,   "<integer number>"},
      {TOK_FLOAT, "<floating point number>"},
      {TOK_ID,    "<identifier>"},

      {TOK_DOT,       "."},
      {TOK_COMMA,     ","},
      {TOK_SEMICOLON, ";"},

      {TOK_GE,         ">="},
      {TOK_LE,         "<="},
      {TOK_EQ,         "=="},
      {TOK_NEQ,        "!="},
      {TOK_LT,         "<"},
      {TOK_GT,         ">"},
      {TOK_ASSIGNMENT, "="},
      {TOK_PLUS,       "+"},
      {TOK_MINUS,      "-"},
      {TOK_MULT,       "*"},
      {TOK_DIVIDE,     "/"},

      {TOK_BLOCK_OPEN, "{"},
      {TOK_BLOCK_CLOSE,"}"},
      {TOK_BRACE_OPEN, "("},
      {TOK_BRACE_CLOSE,")"},

      {TOK_BREAK,    "<break statement>"},
      {TOK_CONTINUE, "<continue statement>"},
      {TOK_RETURN,   "<return statement>"},
      {TOK_FOR,      "<for statement>"},
      {TOK_WHILE,    "<while statement>"},
      {TOK_IF,       "<if statement>"},
      {TOK_ELSE,     "<else statement>"},
      {TOK_CLASS,    "<class statement>"},
   };
   return mp[tok];
}
