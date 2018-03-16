#include <stdio.h>

#include <rivten.h>

enum token_type
{
	Token_EndOfStream,
	Token_Integer,
	Token_Mul,
	Token_Div,
	Token_Add,
	Token_BitNot,

	Token_Count,
};

struct token
{
	token_type Type;
	u32 TextLength;
	char* Text;
};

struct tokenizer
{
	char* At;
};

inline bool
IsEndOfLine(char C)
{
	bool Result = (C == '\n') ||
		(C == '\r');

	return(Result);
}

inline bool
IsWhitespace(char C)
{
	bool Result = (C == ' ') ||
		(C == '\t') ||
		(C == '\v') ||
		(C == '\f') ||
		IsEndOfLine(C);

	return(Result);
}

inline bool
IsNumeric(char C)
{
	bool Result = (C >= '0') && (C <= '9');

	return(Result);
}

internal void
EatAllWhitespace(tokenizer* Tokenizer)
{
	while(IsWhitespace(Tokenizer->At[0]))
	{
		++Tokenizer->At;
	}
}

internal token
GetToken(tokenizer* Tokenizer)
{
	EatAllWhitespace(Tokenizer);

	token Token = {};
	Token.TextLength = 1;
	Token.Text = Tokenizer->At;

	char C = Tokenizer->At[0];
	++Tokenizer->At;

	switch(C)
	{
		case 0:
			{
				Token.Type = Token_EndOfStream;
			} break;
		case '*':
			{
				Token.Type = Token_Mul;
			} break;
		case '/':
			{
				Token.Type = Token_Div;
			} break;
		case '~':
			{
				Token.Type = Token_BitNot;
			} break;
		case '+':
			{
				Token.Type = Token_Add;
			} break;

		default:
			{
				if(IsNumeric(C))
				{
					Token.Type = Token_Integer;
					while(IsNumeric(Tokenizer->At[0]))
					{
						++Token.TextLength;
						++Tokenizer->At;
					}
				}
				else
				{
					printf("%c\n", C);
					InvalidCodePath;
				}
			} break;
	}

	return(Token);
}

s32 main(s32 Arguments, char* ArgumentCount)
{
	char* InputStr = "12*34 + 45/46 + ~25";

	tokenizer Tokenizer = {};
	Tokenizer.At = InputStr;

	token Token = {};
	do
	{
		Token = GetToken(&Tokenizer);
		switch(Token.Type)
		{
			case Token_EndOfStream:
				{
					printf("End of stream\n");
				} break;
			case Token_Integer:
				{
					printf("Integer\n");
				} break;
			case Token_Mul:
				{
					printf("Mul\n");
				} break;
			case Token_Div:
				{
					printf("Div\n");
				} break;
			case Token_Add:
				{
					printf("Add\n");
				} break;
			case Token_BitNot:
				{
					printf("BitNot\n");
				} break;
			InvalidDefaultCase;
		}

	} while(Token.Type != Token_EndOfStream);

	return(0);
}
