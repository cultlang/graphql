
#pragma once
#include "graphql/common.h"

namespace cultlang {
namespace graphql 
{
  extern craft::lisp::BuiltinModuleDescription BuiltinGraphql;

  CULTLANG_GRAPHQL_EXPORTED craft::instance<craft::lisp::Module> make_bindings(craft::instance<craft::lisp::Namespace> ns, craft::instance<> loader);
}}  
