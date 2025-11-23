#include "OspreyAST/Tokeniser.h"
#include "OspreyAST/ASTDump.h"
#include "OspreyAST/Parser.h"
#include "OspreyVM/VMCompiler.h"
#include "OspreyVM/VM.h"

#include <string>
#include <fstream>
#include <sstream>

int main()
{
	const std::string script =
		"add: (a: i32, b: i32) -> i32 \
		{ \
			return a + b; \
		} \
		main: () -> i32 \
		{ \
		x: i32 = add(1, 2); \
		y: i32 = add(x, 3); \
			return y; \
		}";

	std::optional<Osprey::TokenBuffer> tokens = Osprey::Tokenise(script);
	if (!tokens)
	{
		return 1;
	}

	std::optional<Osprey::AST> ast = Osprey::Parse(*tokens);
	if (!ast)
	{
		return 1;
	}

	Osprey::ASTDump ast_dumper;
	ast_dumper.Visit(*ast->GetRoot());

	std::optional<Osprey::VMProgram> program = Osprey::Compile(*ast);
	if (!program)
	{
		return 1;
	}

	std::optional<Osprey::VM> vm = Osprey::VM::Load(*program);
	if (!vm)
	{
		return 1;
	}

	vm->Execute();
	vm->GetStack().Dump();

	return 0;
}