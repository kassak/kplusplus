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

0[xX]{hex}+|{dig}+/{no_id} {
    value_ = strtol(yytext, 0, 0);
    return TOK_INT;
}

{dig}+\.{dig}*([eE][+-]?{dig}+)?/{no_id} {
    value_ = strtod(yytext, 0);
    return TOK_FLOAT;
}

\"([^\\\"]|\\.)*\" {
    return TOK_STRING_LITERAL;
}

'([^'\\]|\\'|\\[^']+)' {
    return TOK_CHAR_LITERAL;
}

"break"/{no_id} {
	return TOK_BREAK;
}

("class"|"struct")/{no_id} {
	return TOK_CLASS;
}

"continue"/{no_id} {
	return TOK_CONTINUE;
}

"if"/{no_id} {
    return TOK_IF;
}

"else"/{no_id} {
    return TOK_ELSE;
}

"while"/{no_id} {
    return TOK_WHILE;
}

"for"/{no_id} {
    return TOK_FOR;
}

"return"/{no_id} {
    return TOK_RETURN;
}

"." {
    return TOK_DOT;
}

"," {
    return TOK_COMMA;
}

";" {
    return TOK_SEMICOLON;
}

"<=" {
    return TOK_LE;
}

">=" {
    return TOK_GE;
}

"==" {
    return TOK_EQ;
}

"<" {
    return TOK_LT;
}

">" {
    return TOK_GT;
}

"=" {
    return TOK_ASSIGNMENT;
}

"+" {
    return TOK_PLUS;
}

"-" {
    return TOK_MINUS;
}

"*" {
    return TOK_MULT;
}

"/" {
    return TOK_DIVIDE;
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
		return TOK_ID;
}

{ws}


%%
