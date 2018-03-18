#pragma once

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
