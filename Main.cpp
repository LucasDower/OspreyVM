#include "Tokeniser.h"
#include "Parser.h"
#include "VMCompiler.h"
#include "VM.h"

#include <string>
#include <fstream>
#include <sstream>

int main()
{
	std::ifstream file("Script.osp");
	if (!file)
	{
		return 1;
	}

	std::ostringstream ss;
	ss << file.rdbuf();
	const std::string script = ss.str();

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