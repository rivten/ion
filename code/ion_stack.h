#pragma once

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

