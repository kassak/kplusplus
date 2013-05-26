#include "config.hpp"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

config_t::config_t()
   : verbose_lexer(false)
{
}

config_t::config_t(int argc, char **argv)
{
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("verbose_lexer",  "makes lexer print tokens")
      ("verbose_parser", "makes parser print rules")
      ("dump_ast",       "dumps ast in dot format after parsing")
      ("ir",             "emits llvm ir code")
      ;

   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);

   if (vm.count("help")) {
      std::cout << desc << std::endl;
      std::exit(1);
   }

   verbose_lexer = vm.count("verbose_lexer");
   verbose_parser = vm.count("verbose_parser");
   dump_ast = vm.count("dump_ast");
   emit_ir = vm.count("ir");
}
