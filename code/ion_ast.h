#pragma once

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
	Assert(LeftSubAst);
	Assert(RightSubAst);
	ast* Result = AllocateStruct(ast);
	Result->Token = Token;
	Result->Left = LeftSubAst;
	Result->Right = RightSubAst;

	return(Result);
}

internal ast*
CreateAst(token Token, ast* SubAst)
{
	Assert(SubAst);
	ast* Result = AllocateStruct(ast);
	Result->Token = Token;
	Result->Left = SubAst;
	Result->Right = 0;

	return(Result);
}

internal s32
AstEvaluate(ast* Ast)
{
	Assert(Ast);
	token Token = Ast->Token;
	if(Token.Type == Token_Integer)
	{
		return(Token.IntegerValue);
	}
	else if(IsOperator(Token))
	{
		switch(Token.Operator)
		{
			case Op_UnaryMin:
				{
					s32 SubValue = AstEvaluate(Ast->Left);
					return(-SubValue);
				} break;
			case Op_UnaryBitNot:
				{
					s32 SubValue = AstEvaluate(Ast->Left);
					return(~SubValue);
				} break;
			case Op_Mul:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue * RightValue);
				} break;
			case Op_Div:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue / RightValue);
				} break;
			case Op_Mod:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue % RightValue);
				} break;
			case Op_LeftShift:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue << RightValue);
				} break;
			case Op_RightShift:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue >> RightValue);
				} break;
			case Op_BitAnd:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue & RightValue);
				} break;
			case Op_Add:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue + RightValue);
				} break;
			case Op_Sub:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue - RightValue);
				} break;
			case Op_BitOr:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue | RightValue);
				} break;
			case Op_BitXor:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					return(LeftValue ^ RightValue);
				} break;
			case Op_Exp:
				{
					s32 LeftValue = AstEvaluate(Ast->Left);
					s32 RightValue = AstEvaluate(Ast->Right);
					s32 Result = (s32)(pow(LeftValue, RightValue));
					return(Result);
				} break;
			InvalidDefaultCase;
		}
	}
	InvalidCodePath;
	return(0);
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

