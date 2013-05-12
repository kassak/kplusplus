#pragma once

struct config_t
{
   config_t();
   config_t(int argc, char **argv);

   bool verbose_lexer;
};
