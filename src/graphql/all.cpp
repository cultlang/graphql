
#include "graphql/common.h"

#include "graphqlparser/AstNode.h"
#include "graphqlparser/AstVisitor.h"
#include "graphqlparser/GraphQLParser.h"

#include "all.h"

using namespace craft;
using namespace craft::lisp;
using namespace craft::types;
using namespace cultlang::graphql;

using namespace facebook;
using namespace facebook::graphql;
using namespace facebook::graphql::ast;

#define lMM sem->builtin_implementMultiMethod
#define MoD "graphql"

typedef std::stack<instance<Sexpr>> ParseStack;


class CultGraphqlSchemaVisitor
	: public facebook::graphql::ast::visitor::AstVisitor
{
	ParseStack ps;

	instance<Symbol> typenameAsCultType(std::string s) {
		if (s == "Int") return Symbol::makeSymbol("Int32");
		else if (s == "Float") return Symbol::makeSymbol("Float64");
		else if (s == "String") return Symbol::makeSymbol("String");
		else if (s == "Boolean") return Symbol::makeSymbol("Boolean");
		else if (s == "ID") return Symbol::makeSymbol("Id");
		else return Symbol::makeSymbol(s);
	}
public:
	CultGraphqlSchemaVisitor()
	{
		
	}



	virtual bool visitDocument(const Document &document) override { 
		auto root = instance<Sexpr>::make();
		ps.push(root);
		return true; 
	}

	virtual void endVisitDocument(const Document &document) override { 

	}

	virtual bool visitObjectTypeDefinition(const ObjectTypeDefinition &objectTypeDefinition) {

		auto define = instance<Sexpr>::make();
		size_t s;

		define->cells.push_back(Symbol::makeSymbol("define"));
		auto pos = objectTypeDefinition.getLocation().begin;
		s = size_t(pos.column) << 32;
		s += size_t(pos.line);
		define->cell_locs.push_back(s);

		define->cells.push_back(Symbol::makeSymbol(objectTypeDefinition.getName().getValue()));
		pos = objectTypeDefinition.getLocation().begin;
		s = size_t(pos.column) << 32;
		s += size_t(pos.line);
		define->cell_locs.push_back(s);

		auto type = instance<Sexpr>::make();
		type->cells.push_back(Symbol::makeSymbol("type"));
		pos = objectTypeDefinition.getLocation().begin;
		s = size_t(pos.column) << 32;
		s += size_t(pos.line);
		type->cell_locs.push_back(s);

		for (auto& i : objectTypeDefinition.getFields()) 
		{
			auto field = instance<Sexpr>::make();
			
			auto name = Symbol::makeSymbol(i->getName().getValue());
			auto typename_ = (NamedType*)&i->getType();
			auto tstr = typename_->getName().getValue();

			field->cells.push_back(name);
			field->cells.push_back(typenameAsCultType(tstr));

			type->cells.push_back(field);
		}

		define->cells.push_back(type);

		ps.top()->cells.push_back(define);
		return true; 
	}

	virtual void endVisitObjectTypeDefinition(const ObjectTypeDefinition &objectTypeDefinition) {
	}


	instance<Sexpr> getRoot() {
		return ps.top();
	}
};

instance<Module> cultlang::graphql::make_bindings(instance<lisp::Namespace> ns, instance<> loader)
{
  auto ret = instance<Module>::make(ns, loader);
  auto sem = instance<CultSemantics>::make(ret);
  ret->builtin_setSemantics(sem);

  lMM(MoD"/parse", [](instance<std::string> s) {
	  const char* err = 0;
	  auto parse = facebook::graphql::parseStringWithExperimentalSchemaSupport(s->c_str(), &err);
	  if (err)
	  {
		  throw stdext::exception(err);
	  }
		
	  CultGraphqlSchemaVisitor vis;
	  parse->accept(&vis);

	  auto res = vis.getRoot();

	  return instance<std::string>::make(res->toRepr());
	  //res->accept
  });
  
  return ret;
}

BuiltinModuleDescription cultlang::graphql::BuiltinGraphql("cultlang/graphql", cultlang::graphql::make_bindings);


#include "types/dll_entry.inc"
