#include <stdio.h>

#include <rivten.h>

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
	Token_UnaryMinus,
	Token_BinaryMinus,
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
GetOperatorType(token_type Token)
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
		case Token_Mod:
			{
				return(Op_Mod);
			} break;
		case Token_LeftShift:
			{
				return(Op_LeftShift);
			} break;
		case Token_RightShift:
			{
				return(Op_RightShift);
			} break;
		case Token_BitAnd:
			{
				return(Op_BitAnd);
			} break;
		case Token_BitOr:
			{
				return(Op_BitOr);
			} break;
		case Token_BitXor:
			{
				return(Op_BitXor);
			} break;
		case Token_UnaryMinus:
			{
				return(Op_UnaryMin);
			} break;
		case Token_BinaryMinus:
			{
				return(Op_Sub);
			} break;
		case Token_Exp:
			{
				return(Op_Exp);
			} break;
		InvalidDefaultCase;
	}
	Assert(false);
	return(Op_Count);
}

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

internal ion_operator
GetOperator(token_type Token)
{
	ion_operator Result = {};
	operator_type OpType = GetOperatorType(Token);
	Result = OpTable[OpType];

	return(Result);
}

inline precedence
OpPrecedence(token Token)
{
	precedence Result = OpTable[Token.Operator].Precedence;
	return(Result);
}

inline bool
IsLeftAssociative(token Token)
{
	bool Result = OpTable[Token.Operator].IsLeftAssociative;
	return(Result);
}

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
			Token.Type == Token_UnaryMinus ||
			Token.Type == Token_BinaryMinus ||
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

inline bool
IsBinary(token Token)
{
	bool Result = (OpTable[Token.Operator].Property == OpProp_Binary);
	return(Result);
}

inline bool
IsUnary(token Token)
{
	bool Result = (OpTable[Token.Operator].Property == OpProp_Unary);
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
	token PreviousToken;
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
				// NOTE(hugo): IMPORTANT(hugo):
				// This works even for the first token
				// because an uninitialized token will have the type
				// Token_EndOfStream which is not an operator.
				if(IsOperator(Tokenizer->PreviousToken) ||
						IsParenthesis(Tokenizer->PreviousToken))
				{
					Token.Type = Token_UnaryMinus;
				}
				else
				{
					Token.Type = Token_BinaryMinus;
				}
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

	if(IsOperator(Token))
	{
		Token.Operator = GetOperatorType(Token.Type);
	}

	Tokenizer->PreviousToken = Token;
	return(Token);
}

internal u32
GetIntegerValue(token Token)
{
	Assert(Token.Type == Token_Integer);

	u32 Result = Token.IntegerValue;

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
		case Token_Mod:
			{
				printf("%% ");
			} break;
		case Token_LeftShift:
			{
				printf("<< ");
			} break;
		case Token_RightShift:
			{
				printf(">> ");
			} break;
		case Token_BitAnd:
			{
				printf("& ");
			} break;
		case Token_BitOr:
			{
				printf("| ");
			} break;
		case Token_BitXor:
			{
				printf("^ ");
			} break;
		case Token_UnaryMinus:
		case Token_BinaryMinus:
			{
				printf("- ");
			} break;
		case Token_Exp:
			{
				printf("** ");
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

internal bool
ShouldPopStack(token CurrentToken, token TopStackToken)
{
	if(TopStackToken.Type == Token_LeftParenthesis)
	{
		return(false);
	}
	else
	{
		bool Result = (((OpPrecedence(TopStackToken) > OpPrecedence(CurrentToken)) ||
						(OpPrecedence(TopStackToken) == OpPrecedence(CurrentToken) &&
						 IsLeftAssociative(TopStackToken))));
		return(Result);
	}
}

s32 main(s32 Arguments, char** ArgumentCount)
{
	char* InputStr = "12*34 * 77 ^ (7865 * (2 ** 2) | 45)/46 - ~25 + 12 + (- 13 + 1) << 3 + - 100 >> 1";

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
				token TopStackToken = PeekTokenTop(&OperatorStack);

				while(ShouldPopStack(Token, TopStackToken))
				{
					Assert(IsOperator(TopStackToken));
					PopTokenStack(&OperatorStack);
					PushTokenToAstStack(&OutputStack, TopStackToken);

					if(OperatorStack.Count == 0)
					{
						break;
					}
					TopStackToken = PeekTokenTop(&OperatorStack);
				}

			}
			PushTokenStack(&OperatorStack, Token);
		}
		else if(Token.Type == Token_LeftParenthesis)
		{
			PushTokenStack(&OperatorStack, Token);
		}
		else if(Token.Type == Token_RightParenthesis)
		{
			for(token TopStackToken = PopTokenStack(&OperatorStack);
					TopStackToken.Type != Token_LeftParenthesis;
					TopStackToken = PopTokenStack(&OperatorStack))
			{
				Assert(IsOperator(TopStackToken));
				PushTokenToAstStack(&OutputStack, TopStackToken);
			}
		}

	} while(Token.Type != Token_EndOfStream);

	// NOTE(hugo): Flush the remaning of the operator stack
	while(OperatorStack.Count != 0)
	{
		token OperatorToken = PopTokenStack(&OperatorStack);
		PushTokenToAstStack(&OutputStack, OperatorToken);
	}

#if 0
	printf("%u\n", OutputStack.Count);
	for(u32 AstIndex = 0; AstIndex < OutputStack.Count; ++AstIndex)
	{
		ast* Ast = OutputStack.Stack[AstIndex];
		PrintAst(Ast);
		printf("\n");
	}
#endif

	Assert(OutputStack.Count == 1);
	ast* Ast = PopAstStack(&OutputStack);
	PrintAst(Ast);
	printf("\n");

	return(0);
}
