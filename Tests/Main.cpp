#include "OspreyAST/Tokeniser.h"
#include "OspreyAST/ASTDump.h"
#include "OspreyAST/Parser.h"
#include "OspreyVM/VMCompiler.h"
#include "OspreyVM/VM.h"

#include <print>
#include <filesystem>
#include <vector>
#include <fstream>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::println(stderr, "No test file(s) provided to run");
		return 1;
	}

	std::vector<std::filesystem::path> test_files_to_run;
	const std::string location = argv[1];
	constexpr std::string_view filetype_extension = ".osp";
	constexpr std::string_view test_fail_prefix = "\033[31m(Fail)\033[0m";
	constexpr std::string_view test_pass_prefix = "\033[32m(Pass)\033[0m";

	if (!std::filesystem::exists(location))
	{
		std::println(stderr, "The path '{}' does not exist", location);
		return 1;
	}

	if (std::filesystem::is_directory(location))
	{
		for (const auto& entry : std::filesystem::directory_iterator(location))
		{
			if (entry.is_regular_file() && entry.path().extension() == filetype_extension)
			{
				test_files_to_run.push_back(entry);
			}
		}
	}
	else if (std::filesystem::is_regular_file(location))
	{
		if (!location.ends_with(filetype_extension))
		{
			std::println(stderr, "The file '{}' is not an .osp file", location);
			return 1;
		}

		test_files_to_run.push_back(location);
	}

	std::println(stderr, "Running {} test(s)", test_files_to_run.size());

	for (const std::filesystem::path& file_path : test_files_to_run)
	{
		const auto ReportError = [&](const std::string& message)
			{
				std::println("[{}]: {} {}", file_path.filename().string(), test_fail_prefix, message);
			};

		std::ifstream file(file_path, std::ios::binary);

		file.seekg(0, std::ios::end);
		std::string file_data;
		file_data.resize(file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(file_data.data(), file_data.size());

		std::expected<Osprey::TokenBuffer, Osprey::ErrorMessage> tokens = Osprey::Tokenise(file_data);
		if (!tokens)
		{
			ReportError(std::format("Tokeniser Error: {}", tokens.error()));
			continue;
		}

		std::expected<Osprey::AST, Osprey::ErrorMessage> ast = Osprey::Parse(*tokens);
		if (!ast)
		{
			ReportError(std::format("Parser Error: {}", ast.error()));
			continue;
		}

		std::optional<Osprey::VMProgram> program = Osprey::Compile(*ast);
		if (!program)
		{
			ReportError("Compile Error");
			continue;
		}

		//program->Dump();

		std::optional<Osprey::VM> vm = Osprey::VM::Load(*program);
		if (!vm)
		{
			ReportError("VM Error");
			continue;
		}

		vm->Execute();

		const Osprey::VMStack& stack = vm->GetStack();

		if (stack.GetSize() == 0)
		{
			ReportError(std::format("Expected test to produce a value, received {}", stack.GetSize()));
			continue;
		}

		if (stack.GetFromTop(0) != 0)
		{
			ReportError("Test failed");
			continue;
		}

		std::println("[{}]: {}", file_path.filename().string(), test_pass_prefix);
	}

	return 0;
}