dig [0-9]
dig1 [1-9]
let [a-zA-Z]
let_ ({let}|_)
hex [0-9a-fA-F]
no_id [^0-9a-zA-Z_]
ws [ \t\r\n]

%option c++
%option yyclass="klexer_t"
		//%option batch
%option yylineno
%option noyywrap

%{
#undef yyFlexLexer
#include <stdlib.h>
#include <klexer.hpp>
#include "tokens.h"

#define YY_USER_INIT yyin = input_;
		//#define YY_DECL int yylex()
%}
%%

0[xX]{hex}+|[+-]?{dig}+/{no_id} {
//yylval.iValue = strtol(yytext, 0, 0);
//std::cout << "he " << yytext << std::endl;
    value_ = strtol(yytext, 0, 0);
    return TOK_INT;
}

{dig}+.{dig}*([eE][+-]?{dig}+)?/{no_id} {
    value_ = strtod(yytext, 0);
    return TOK_FLOAT;
}

\"([^\\\"]|\\.)*\" {
    return TOK_STRING_LITERAL;
}

'([^'\\]|\\'|\\[^']+)' {
    return TOK_CHAR_LITERAL;
}

"break" {
	return TOK_BREAK;
}

"class"|"struct" {
	return TOK_CLASS;
}

"continue" {
	return TOK_CONTINUE;
}

"if" {
    return TOK_IF;
}

"while" {
    return TOK_WHILE;
}

"for" {
    return TOK_FOR;
}

"return" {
    return TOK_RETURN;
}

"." {
    return TOK_DOT;
}

"," {
    return TOK_COLON;
}

";" {
    return TOK_SEMICOLON;
}

"=" {
    return TOK_ASSIGNMENT;
}

"(" {
    return TOK_BRACE_OPEN;
}

")" {
    return TOK_BRACE_CLOSE;
}

"{" {
    return TOK_BLOCK_OPEN;
}

"}" {
    return TOK_BLOCK_CLOSE;
}

{let_}({let_}|{dig})*/{no_id} {
		std::cout << "id " << yytext << std::endl;
		return TOK_ID;
}

{ws} {
		std::cout << "ws " << yytext << std::endl;
}

/*
.* {

 std::cout << "sk " << yytext << std::endl;
}
*/

%%
