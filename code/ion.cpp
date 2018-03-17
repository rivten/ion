#include <stdio.h>

#include <rivten.h>

enum token_type
{
	Token_EndOfStream,
	Token_Integer,

	// NOTE(hugo): Operators
	Token_Mul,
	Token_Div,
	Token_Add,
	Token_BitNot,

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

	Op_Count,
};

struct token
{
	token_type Type;
	u32 TextLength;
	char* Text;
};

struct ast
{
	token Token;
	ast* Left;
	ast* Right;
};

internal ast*
CreateLeaftAstFromInteger(token Token)
{
	Assert(Token.Type = Token_Integer);
	ast* Result = AllocateStruct(ast);
	Result->Token = Token;
	Result->Left = 0;
	Result->Right = 0;

	return(Result);
}

internal ast*
CreateAst(token Token, ast* LeftSubAst, ast* RightSubAst)
{
	ast* Result = AllocateStruct(ast);
	Result->Token = Token;
	Result->Left = LeftSubAst;
	Result->Right = RightSubAst;

	return(Result);
}

internal ast*
CreateAst(token Token, ast* SubAst)
{
	ast* Result = AllocateStruct(ast);
	Result->Token = Token;
	Result->Left = SubAst;
	Result->Right = 0;

	return(Result);
}

internal operator_type
GetOperator(token_type Token)
{
	switch(Token)
	{
		case Token_Mul:
			{
				return(Op_Mul);
			} break;
		case Token_Div:
			{
				return(Op_Div);
			} break;
		case Token_Add:
			{
				return(Op_Add);
			} break;
		case Token_BitNot:
			{
				return(Op_UnaryBitNot);
			} break;
		InvalidDefaultCase;
	}
	Assert(false);
	return(Op_Count);
}

typedef u8 precedence;

enum operator_property
{
	OpProp_Unary,
	OpProp_Binary,
};

global_variable precedence OpPrecedenceTable[Op_Count] =
{
	2, //Op_UnaryMin
	2, //Op_UnaryBitNot

	1, //Op_Mul
	1, //Op_Div
	1, //Op_Mod
	1, //Op_LeftShift
	1, //Op_RightShift
	1, //Op_BitAnd

	0, //Op_Add
	0, //Op_Sub
	0, //Op_BitOr
	0, //Op_BitXor
};

global_variable bool OpLeftAssocTable[Op_Count] =
{
	false, //Op_UnaryMin
	false, //Op_UnaryBitNot

	true, //Op_Mul
	true, //Op_Div
	true, //Op_Mod
	true, //Op_LeftShift
	true, //Op_RightShift
	true, //Op_BitAnd

	true, //Op_Add
	true, //Op_Sub
	true, //Op_BitOr
	true, //Op_BitXor
};

global_variable operator_property OpPropertyTable[Op_Count] =
{
	OpProp_Unary, //Op_UnaryMin
	OpProp_Unary, //Op_UnaryBitNot

	OpProp_Binary, //Op_Mul
	OpProp_Binary, //Op_Div
	OpProp_Binary, //Op_Mod
	OpProp_Binary, //Op_LeftShift
	OpProp_Binary, //Op_RightShift
	OpProp_Binary, //Op_BitAnd

	OpProp_Binary, //Op_Add
	OpProp_Binary, //Op_Sub
	OpProp_Binary, //Op_BitOr
	OpProp_Binary, //Op_BitXor
};

inline precedence
OpPrecedence(operator_type Op)
{
	precedence Result = OpPrecedenceTable[Op];
	return(Result);
}

inline precedence
OpPrecedence(token Token)
{
	operator_type Op = GetOperator(Token.Type);
	precedence Result = OpPrecedence(Op);
	return(Result);
}

inline bool
IsLeftAssociative(operator_type Op)
{
	bool Result = OpLeftAssocTable[Op];
	return(Result);
}

inline bool
IsLeftAssociative(token Token)
{
	operator_type Op = GetOperator(Token.Type);
	bool Result = IsLeftAssociative(Op);
	return(Result);
}

inline bool
IsOperator(token Token)
{
	bool Result = (Token.Type == Token_Mul ||
			Token.Type == Token_Div ||
			Token.Type == Token_Add ||
			Token.Type == Token_BitNot);
	return(Result);
}

inline bool
IsBinary(token Token)
{
	operator_type Op = GetOperator(Token.Type);
	bool Result = (OpPropertyTable[Op] == OpProp_Binary);
	return(Result);
}

inline bool
IsUnary(token Token)
{
	operator_type Op = GetOperator(Token.Type);
	bool Result = (OpPropertyTable[Op] == OpProp_Unary);
	return(Result);
}

struct tokenizer
{
	char* At;
};

#define MAX_STACK_SIZE 16
struct token_stack
{
	u32 Count;
	token Stack[MAX_STACK_SIZE];
};

internal void
PushTokenStack(token_stack* Stack, token Token)
{
	Assert(Stack->Count < ArrayCount(Stack->Stack));
	Stack->Stack[Stack->Count] = Token;
	++Stack->Count;
}

internal token
PopTokenStack(token_stack* Stack)
{
	Assert(Stack->Count != 0);
	token Result = Stack->Stack[Stack->Count - 1];
	--Stack->Count;

	return(Result);
}

internal token
PeekTokenTop(token_stack* Stack)
{
	Assert(Stack->Count != 0);
	token Result = Stack->Stack[Stack->Count - 1];

	return(Result);
}

struct ast_stack
{
	u32 Count;
	ast* Stack[MAX_STACK_SIZE];
};

internal void
PushAstStack(ast_stack* Stack, ast* Ast)
{
	Assert(Stack->Count < ArrayCount(Stack->Stack));
	Stack->Stack[Stack->Count] = Ast;
	++Stack->Count;
}

internal ast*
PopAstStack(ast_stack* Stack)
{
	Assert(Stack->Count != 0);
	ast* Result = Stack->Stack[Stack->Count - 1];
	--Stack->Count;

	return(Result);
}

internal ast*
PeekAstTop(ast_stack* Stack)
{
	Assert(Stack->Count != 0);
	ast* Result = Stack->Stack[Stack->Count - 1];

	return(Result);
}

internal void
PushTokenToAstStack(ast_stack* Stack, token Token)
{
	ast* Ast = 0;
	if(IsBinary(Token))
	{
		ast* RightSubAst = PopAstStack(Stack);
		ast* LeftSubAst = PopAstStack(Stack);

		Ast = CreateAst(Token, LeftSubAst, RightSubAst);
	}
	else if(IsUnary(Token))
	{
		ast* SubAst = PopAstStack(Stack);
		Ast = CreateAst(Token, SubAst);
	}
	else
	{
		InvalidCodePath;
	}

	Assert(Ast);
	PushAstStack(Stack, Ast);
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

internal u32
GetIntegerValue(token Token)
{
	Assert(Token.Type == Token_Integer);

	u32 Result = 0;
	for(u32 Index = 0; Index < Token.TextLength; ++Index)
	{
		u8 Figure = Token.Text[Index] - '0';
		Result *= 10;
		Result += Figure;
	}

	return(Result);
}

internal void
PrintAst(ast* Ast)
{
	token Token = Ast->Token;
	printf("(");
	switch(Token.Type)
	{
		case Token_Integer:
			{
				u32 IntValue = GetIntegerValue(Token);
				printf("%u", IntValue);
			} break;
		case Token_Mul:
			{
				printf("* ");
			} break;
		case Token_Div:
			{
				printf("/ ");
			} break;
		case Token_Add:
			{
				printf("+ ");
			} break;
		case Token_BitNot:
			{
				printf("~ ");
			} break;
		InvalidDefaultCase;
	}

	if(Ast->Left)
	{
		PrintAst(Ast->Left);
	}
	if(Ast->Right)
	{
		PrintAst(Ast->Right);
	}
	printf(")");
}

s32 main(s32 Arguments, char** ArgumentCount)
{
	char* InputStr = "12*34 + 45/46 + ~25";

	tokenizer Tokenizer = {};
	Tokenizer.At = InputStr;

	token_stack OperatorStack = {};
	ast_stack OutputStack = {};

	token Token = {};
	do
	{
		Token = GetToken(&Tokenizer);

		if(Token.Type == Token_Integer)
		{
			ast* Leaf = CreateLeaftAstFromInteger(Token);
			PushAstStack(&OutputStack, Leaf);
		}
		else if(IsOperator(Token))
		{
			if(OperatorStack.Count != 0)
			{
				token TopStackOperator = PeekTokenTop(&OperatorStack);
				while((OpPrecedence(TopStackOperator) > OpPrecedence(Token)) ||
						(OpPrecedence(TopStackOperator) == OpPrecedence(Token) &&
						 IsLeftAssociative(TopStackOperator)))
				{
					PopTokenStack(&OperatorStack);
					PushTokenToAstStack(&OutputStack, TopStackOperator);

					if(OperatorStack.Count == 0)
					{
						break;
					}
					TopStackOperator = PeekTokenTop(&OperatorStack);
				}

			}
			PushTokenStack(&OperatorStack, Token);
		}

	} while(Token.Type != Token_EndOfStream);

	// NOTE(hugo): Flush the remaning of the operator stack
	while(OperatorStack.Count != 0)
	{
		token OperatorToken = PopTokenStack(&OperatorStack);
		PushTokenToAstStack(&OutputStack, OperatorToken);
	}

	Assert(OutputStack.Count == 1);
	ast* Ast = PopAstStack(&OutputStack);
	PrintAst(Ast);
	printf("\n");

	return(0);
}
