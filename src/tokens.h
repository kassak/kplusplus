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
   TOK_CLASS,

   TOK_DOT,
   TOK_COLON,
   TOK_SEMICOLON,

   TOK_ASSIGNMENT,

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
      {TOK_COLON,     ","},
      {TOK_SEMICOLON, ";"},

      {TOK_ASSIGNMENT, "="},

      {TOK_BLOCK_OPEN, "{"},
      {TOK_BLOCK_CLOSE,"}"},
      {TOK_BRACE_OPEN, "("},
      {TOK_BRACE_CLOSE,")"},
   };
   return mp[tok];
}
