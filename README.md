# OspreyVM

This repro explores a custom toy scripting language, *Osprey*, and its associated VM, *OspreyVM*.

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
	std::string Script =
		"x = 3;\n"
		"y = 4;\n"
		"return x + y + 1;";

	std::optional<TTokenBuffer> Tokens = FLexer::Tokenise(Script);
	if (!Tokens)
	{
		std::cout << "Could not tokenise script\n";
		return 1;
	}

	std::optional<FAST> AST = FParser::Parse(*Tokens);
	if (!AST)
	{
		std::cout << "Could not parse tokens\n";
		return 1;
	}

	std::optional<FProgram> CompiledProgram = AST->Compile();

	if (CompiledProgram.has_value())
	{
		FProcessor Processor(*CompiledProgram);
		Processor.Execute();

		Processor.GetDataStack().Dump();
	}

	return 0;
}
```
