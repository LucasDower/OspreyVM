# OspreyVM

This repro explores a custom toy scripting language, *Osprey*, and its associated VM, *OspreyVM*. Currently this is the only compiler target.

> [!NOTE]
> The current implementation is a vertical slice so the language syntax, tokeniser, parser, and VM are minimally implemented to establish the pipeline and very basic programs.

The following is an example script written in *Osprey*.
```
x = 3;
y = 4;
return x + y + 1;
```

This repro includes a lexer and parser for *Osprey* into an AST. This AST is then converted into *OspreyVM* bytecode.

```cpp
int main()
{
	const std::string script =
		"x = 3;\n"
		"y = 4;\n"
		"return x + y + 1;";

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
```
