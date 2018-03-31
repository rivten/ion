#pragma once

#define RECURSIVE_DESCENT 1
#if !RECURSIVE_DESCENT
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

internal ast*
IonParse(char* Str)
{
	tokenizer Tokenizer = {};
	Tokenizer.At = Str;

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

	return(Ast);
}

#else // if RECURSIVE_DESCENT

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
	if(IsToken(Lexer, Token_UnaryMinus) ||
			IsToken(Lexer, Token_BitNot))
	{
		Result = AllocateStruct(ast);
		Result->Token = ReadToken(Lexer);
		Assert(Result->Token.Type != Token_LeftParenthesis);

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
		Assert(Result->Token.Type != Token_LeftParenthesis);
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
			IsToken(Lexer, Token_BinaryMinus) ||
			IsToken(Lexer, Token_BitOr) ||
			IsToken(Lexer, Token_BitXor))
	{
		ast* LeftExpr = Result;
		Result = AllocateStruct(ast);
		Result->Token = ReadToken(Lexer);
		Assert(Result->Token.Type != Token_LeftParenthesis);
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
	//ExpectToken(&Lexer, Token_EndOfStream);
	return(Result);
}
#endif

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
