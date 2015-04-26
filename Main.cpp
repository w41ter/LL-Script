#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>

#include "lexer.h"
#include "syntax.h"
#include "Interpret.h"

using namespace std;
//using namespace ScriptCompile;
namespace ScriptCompile
{
	class PrintPlugin : public Plugin
	{
	public:
		PrintPlugin() { name = L"print"; length = -1; }

		virtual Variable Execute(vector<wstring>& params)
		{
			for (auto i : params)
			{
				wcout << i;
			}
			wcout << endl;

			return{ ScriptCompile::TK_END, L"END" };
		}
	};

	class InputPlugin : public Plugin
	{
	public:
		InputPlugin() { name = L"input"; length = 0; }

		virtual Variable Execute(vector<wstring>& params)
		{
			wstring temp;
			wcin >> temp;

			return ConstructVariable(temp);
		}
	};
}


int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "Chinese-simplified");

	if (argc < 2)
	{
		wcout << L"Use: Script.exe input.txt" << endl;
		_getch();
		return 0;
	}

	wstring Code;
	{
		FILE* f = _wfopen(argv[1], L"rb");
		if (f == NULL)
		{
			wcout << L"打不开文件" << argv[1] << endl;
			_getch();
			return 0;
		}

		fpos_t fsize;
		size_t size;
		fseek(f, 0, SEEK_END);
		fgetpos(f, &fsize);
		size = (size_t)fsize;
		fseek(f, 0, SEEK_SET);
		char* AnsiBuffer = new char[size + 1];
		fread(AnsiBuffer, 1, size, f);
		AnsiBuffer[size] = '\0';
		fclose(f);

		size_t wsize = mbstowcs(0, AnsiBuffer, size);
		wchar_t* Buffer = new wchar_t[wsize + 1];
		mbstowcs(Buffer, AnsiBuffer, size);
		Buffer[size] = L'\0';
		Code = Buffer;

		delete[] Buffer;
		delete[] AnsiBuffer;
	}

	ScriptCompile::Lexer lexer(Code);
	ScriptCompile::Token token = lexer.Get();
	int index = 0;

	//while (token.kind != ScriptCompile::TK_EOF)
	//{
	//	index++;
	//	wcout << L"第 " << index << L"个：\n"
	//		<< L"Kind:" << token.kind
	//		<< L"\tLine:" << token.line
	//		<< L"\tValue：" << token.value << endl;

	//	if (token.kind == ScriptCompile::TK_ERROR)
	//		break;

	//	token = lexer.Get();
	//}
	//try {
	//	ScriptCompile::Program* program = ScriptCompile::Parser(Code);
	//}
	//catch (ScriptCompile::ASTError e)
	//{
	//	wcout << L"Line:" << e.GetToken().line << L" 错误信息：" << e.GetMsg() << endl;
	//}

	ScriptCompile::Program program;
	ScriptCompile::PrintPlugin print;
	ScriptCompile::InputPlugin input;
	program.AddPlugin(&print);
	program.AddPlugin(&input);

	try
	{
		ScriptCompile::Parser(Code, program);
	}
	catch (ScriptCompile::ASTError e)
	{
		wcout << L"Line:" << e.GetToken().line << L" 错误信息：" << e.GetMsg() << endl;
		_getch();
		return 0;
	}

	try
	{
		ScriptCompile::Run(&program);
	}
	catch (wstring e)
	{
		wcout << L"错误信息：" << e << endl;
	}
	
	_getch();
	return 0;
}