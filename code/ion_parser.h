#pragma once

// E0 -> E1 ([+, -, |, ^] E1)+
// E1 -> E2 ([*, /, %, <<, >>, &] E2)+
// E2 -> (E0) | E3
// E3 -> [-, ~]+ V
// V -> Number

internal ast*
ParseInteger(lexer* Lexer)
{
	ast* Result = AllocateStruct(ast);
	Result->Token = ExpectToken(Lexer, Token_Integer);
	return(Result);
}

internal ast*
ParseExpr3(lexer* Lexer)
{
	ast* Result = 0;
	if(IsToken(Lexer, Token_Minus) ||
			IsToken(Lexer, Token_BitNot))
	{
		Result = AllocateStruct(ast);
		Result->Token = ReadToken(Lexer);
		switch(Result->Token.Type)
		{
			case Token_Minus:
				{
					Result->Token.Operator = Op_UnaryMin;
				} break;
			case Token_BitNot:
				{
					Result->Token.Operator = Op_UnaryBitNot;
				} break;
			InvalidDefaultCase;
		}

		Result->Left = ParseInteger(Lexer);
	}
	else
	{
		Result = ParseInteger(Lexer);
	}
	Assert(Result);
	return(Result);
}

internal ast* ParseExpr0(lexer* Lexer);

internal ast*
ParseExpr2(lexer* Lexer)
{
	ast* Result = 0;
	if(MatchToken(Lexer, Token_LeftParenthesis))
	{
		Result = ParseExpr0(Lexer);
		ExpectToken(Lexer, Token_RightParenthesis);
	}
	else
	{
		Result = ParseExpr3(Lexer);
	}

	return(Result);
}

internal ast*
ParseExpr1(lexer* Lexer)
{
	ast* Result = ParseExpr2(Lexer);

	while(IsToken(Lexer, Token_Mul) ||
			IsToken(Lexer, Token_Div) ||
			IsToken(Lexer, Token_Mod) ||
			IsToken(Lexer, Token_LeftShift) ||
			IsToken(Lexer, Token_RightShift) ||
			IsToken(Lexer, Token_BitAnd))
	{
		ast* LeftExpr = Result;
		Result = AllocateStruct(ast);
		Result->Token = ReadToken(Lexer);

		switch(Result->Token.Type)
		{
			case Token_Mul:
				{
					Result->Token.Operator = Op_Mul;
				} break;
			case Token_Div:
				{
					Result->Token.Operator = Op_Div;
				} break;
			case Token_Mod:
				{
					Result->Token.Operator = Op_Mod;
				} break;
			case Token_LeftShift:
				{
					Result->Token.Operator = Op_LeftShift;
				} break;
			case Token_RightShift:
				{
					Result->Token.Operator = Op_RightShift;
				} break;
			case Token_BitAnd:
				{
					Result->Token.Operator = Op_BitAnd;
				} break;
			InvalidDefaultCase;
		}

		Result->Left = LeftExpr;
		Result->Right = ParseExpr2(Lexer);
	}
	return(Result);
}

internal ast*
ParseExpr0(lexer* Lexer)
{
	ast* Result = ParseExpr1(Lexer);

	while(IsToken(Lexer, Token_Add) ||
			IsToken(Lexer, Token_Minus) ||
			IsToken(Lexer, Token_BitOr) ||
			IsToken(Lexer, Token_BitXor))
	{
		ast* LeftExpr = Result;
		Result = AllocateStruct(ast);
		Result->Token = ReadToken(Lexer);

		switch(Result->Token.Type)
		{
			case Token_Add:
				{
					Result->Token.Operator = Op_Add;
				} break;
			case Token_Minus:
				{
					Result->Token.Operator = Op_Sub;
				} break;
			case Token_BitOr:
				{
					Result->Token.Operator = Op_BitOr;
				} break;
			case Token_BitXor:
				{
					Result->Token.Operator = Op_BitXor;
				} break;
			InvalidDefaultCase;
		}

		Result->Left = LeftExpr;
		Result->Right = ParseExpr1(Lexer);
	}

	return(Result);
}

internal ast*
IonParse(char* Str)
{
	lexer Lexer = {};
	Lexer.Tokenizer = {};
	Lexer.Tokenizer.At = Str;

	InitLexer(&Lexer);

	ast* Result = ParseExpr0(&Lexer);
	Assert(Lexer.Token.Type == Token_EndOfStream);
	return(Result);
}

internal s32
IonEvaluate(char* Str)
{
	ast* Ast = IonParse(Str);
	s32 Result = AstEvaluate(Ast);
	return(Result);
}

internal u32
IonGetBytecodeByteSize(ast* Ast)
{
	Assert(Ast);
	u32 Result = 0;
	if(Ast->Token.Type == Token_Integer)
	{
		// NOTE(hugo): We need the LIT and the actual number
		Result = sizeof(u32) + sizeof(u8);
	}
	else
	{
		Assert(IsOperator(Ast->Token));
		Result = sizeof(u8);
	}

	if(Ast->Left)
	{
		Result += IonGetBytecodeByteSize(Ast->Left);
	}
	if(Ast->Right)
	{
		Result += IonGetBytecodeByteSize(Ast->Right);
	}

	return(Result);
}

internal u8*
IonWriteBytecode(ast* Ast, u8* Bytecode)
{
	Assert(Ast);
	u8* CurrentBytecodePos = Bytecode;
	if(Ast->Left)
	{
		CurrentBytecodePos = IonWriteBytecode(Ast->Left, CurrentBytecodePos);
	}
	if(Ast->Right)
	{
		CurrentBytecodePos = IonWriteBytecode(Ast->Right, CurrentBytecodePos);
	}

	if(Ast->Token.Type == Token_Integer)
	{
		*CurrentBytecodePos = Opcode_Lit;
		++CurrentBytecodePos;

		u32* IntBytecode = (u32*) CurrentBytecodePos;
		*IntBytecode = Ast->Token.IntegerValue;
		CurrentBytecodePos += sizeof(u32);
	}
	else
	{
		Assert(IsOperator(Ast->Token));
		operator_type Op = Ast->Token.Operator;
		opcode Opcode = GetOpcode(Op);

		*CurrentBytecodePos = (u8)(Opcode);
		++CurrentBytecodePos;
	}

	return(CurrentBytecodePos);
}

internal u8*
IonGenerateBytecode(ast* Ast)
{
	Assert(Ast);
	u32 HaltSize = 1; // NOTE(hugo): Need to add the size to halt

	// TODO(hugo): Here we do another pass on the AST which can be avoided.
	// Everything could theoritically be done directly in the shunting yard algorithm
	// but since there are no perf constraints now, I just did the fastest thing.
	// Maybe improve this in the future.
	u32 BytecodeSize = IonGetBytecodeByteSize(Ast) + HaltSize;
	u8* Bytecode = AllocateArray(u8, BytecodeSize);

	IonWriteBytecode(Ast, Bytecode);
	
	Bytecode[BytecodeSize - 1] = (u8)(Opcode_Halt);

	return(Bytecode);
}
