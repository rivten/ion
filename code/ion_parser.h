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