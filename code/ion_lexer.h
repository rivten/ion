#pragma once

enum token_type
{
	Token_EndOfStream,
	Token_Integer,

	Token_LeftParenthesis,
	Token_RightParenthesis,

	// NOTE(hugo): Operators
	Token_Mul,
	Token_Div,
	Token_Add,
	Token_BitNot,
	Token_Mod,
	Token_LeftShift,
	Token_RightShift,
	Token_BitAnd,
	Token_BitOr,
	Token_BitXor,
	Token_Minus,
	Token_Exp,

	Token_Count,
};

enum operator_type
{
	Op_UnaryMin,
	Op_UnaryBitNot,

	Op_Mul,
	Op_Div,
	Op_Mod,
	Op_LeftShift,
	Op_RightShift,
	Op_BitAnd,

	Op_Add,
	Op_Sub,
	Op_BitOr,
	Op_BitXor,

	Op_Exp,

	Op_Count,
};

typedef u8 precedence;

enum operator_property
{
	OpProp_Unary,
	OpProp_Binary,
};

struct ion_operator
{
	operator_type Type;
	precedence Precedence;
	operator_property Property;
	bool IsLeftAssociative;
};

struct token
{
	token_type Type;
	u32 TextLength;
	char* Text;

	union
	{
		u32 IntegerValue;

		operator_type Operator;
	};
};

global_variable ion_operator OpTable[Op_Count] = 
{
	{Op_UnaryMin,    2, OpProp_Unary, false},
	{Op_UnaryBitNot, 2, OpProp_Unary, false},

	{Op_Mul,         1, OpProp_Binary, true},
	{Op_Div,         1, OpProp_Binary, true},
	{Op_Mod,         1, OpProp_Binary, true},
	{Op_LeftShift,   1, OpProp_Binary, true},
	{Op_RightShift,  1, OpProp_Binary, true},
	{Op_BitAnd,      1, OpProp_Binary, true},

	{Op_Add,         0, OpProp_Binary, true},
	{Op_Sub,         0, OpProp_Binary, true},
	{Op_BitOr,       0, OpProp_Binary, true},
	{Op_BitXor,      0, OpProp_Binary, true},

	{Op_Exp,         1, OpProp_Binary, false},
};

inline bool
IsOperator(token Token)
{
	bool Result = (Token.Type == Token_Mul ||
			Token.Type == Token_Div ||
			Token.Type == Token_Add ||
			Token.Type == Token_BitNot ||
			Token.Type == Token_Mod ||
			Token.Type == Token_LeftShift ||
			Token.Type == Token_RightShift ||
			Token.Type == Token_BitAnd ||
			Token.Type == Token_BitOr ||
			Token.Type == Token_BitXor ||
			Token.Type == Token_Minus ||
			Token.Type == Token_Exp);
	return(Result);
}

inline bool
IsParenthesis(token Token)
{
	bool Result = (Token.Type == Token_LeftParenthesis ||
			Token.Type == Token_RightParenthesis);
	return(Result);
}

struct tokenizer
{
	char* At;

	// NOTE(hugo): Note very clean but will
	// do the trick. The main use is to
	// differentiate between unary minus
	// and binary minus by remembering our
	// current context with the grammar :
	// E -> E { B E }
	// E -> U E
	// E -> Integer
	// B -> any binary operator
	// U -> any unary operator

	// TODO(hugo): Get rid of that
	// since now I have a lexer
	token PreviousToken;
};

internal u32
GetIntegerValue(token Token)
{
	Assert(Token.Type == Token_Integer);

	u32 Result = Token.IntegerValue;

	return(Result);
}

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
				if(Tokenizer->At &&
						Tokenizer->At[0] == '*')
				{
					Token.Type = Token_Exp;
					++Token.TextLength;
					++Tokenizer->At;
				}
				else
				{
					Token.Type = Token_Mul;
				}
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
		case '%':
			{
				Token.Type = Token_Mod;
			} break;
		case '<':
			{
				Assert(Tokenizer->At);
				if(Tokenizer->At[0] == '<')
				{
					Token.Type = Token_LeftShift;
					++Token.TextLength;
					++Tokenizer->At;
				}
				else
				{
					InvalidCodePath;
				}
			} break;
		case '>':
			{
				Assert(Tokenizer->At);
				if(Tokenizer->At[0] == '>')
				{
					Token.Type = Token_RightShift;
					++Token.TextLength;
					++Tokenizer->At;
				}
				else
				{
					InvalidCodePath;
				}
			} break;
		case '&':
			{
				Token.Type = Token_BitAnd;
			} break;
		case '|':
			{
				Token.Type = Token_BitOr;
			} break;
		case '^':
			{
				Token.Type = Token_BitXor;
			} break;
		case '-':
			{
				Token.Type = Token_Minus;
			} break;
		case '(':
			{
				Token.Type = Token_LeftParenthesis;
			} break;
		case ')':
			{
				Token.Type = Token_RightParenthesis;
			} break;

		default:
			{
				if(IsNumeric(C))
				{
					Token.Type = Token_Integer;
					Token.IntegerValue = (C - '0');
					while(IsNumeric(Tokenizer->At[0]))
					{
						Token.IntegerValue *= 10;
						Token.IntegerValue += (Tokenizer->At[0] - '0');
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

struct lexer
{
	token Token;
	tokenizer Tokenizer;
};

internal token
ReadToken(lexer* Lexer)
{
	token Result = Lexer->Token;
	Lexer->Token = GetToken(&Lexer->Tokenizer);
	return(Result);
}

internal bool
MatchToken(lexer* Lexer, token_type TokenType)
{
	bool Match = (TokenType == Lexer->Token.Type);
	if(Match)
	{
		ReadToken(Lexer);
	}

	return(Match);
}

internal token
ExpectToken(lexer* Lexer, token_type TokenType)
{
	token Result = Lexer->Token;
	Assert(Result.Type == TokenType);
	ReadToken(Lexer);
	return(Result);
}

internal void
InitLexer(lexer* Lexer)
{
	ReadToken(Lexer);
}

internal bool
IsToken(lexer* Lexer, token_type TokenType)
{
	bool Result = (Lexer->Token.Type == TokenType);
	return(Result);
}
