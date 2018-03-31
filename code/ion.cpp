#include <stdio.h>
#include <math.h>

#include <rivten.h>

#include "ion_lexer.h"
#include "ion_ast.h"
#include "ion_opcode.h"
#include "ion_parser.h"
#include "ion_vm.h"

s32 main(s32 Arguments, char** ArgumentCount)
{
	//char* InputStr = "12*34 * 77 ^ (7865 * (2 ** 2) | 45)/46 - ~25 + 12 + (- 13 + 1) << 3 + - 100 >> 1";
	char* InputStr = "12*34 * 77 ^ (7865 * (2 * 2) | 45)/46 - ~25 + 12 + (- 13 + 1) << 3 + - 100 >> 1";

	ast* Ast = IonParse(InputStr);

#if 0
	PrintAst(Ast);
	printf("\n");
#else
	s32 AstValue = AstEvaluate(Ast);
	printf("Value = %i\n", AstValue);
#endif

	char* Str2 = "1 + (2 + 3) * 2";
	printf("%s = %i\n", Str2, IonEvaluate(Str2));

	char* Str3 = "5 - 1 - 2";
	printf("%s = %i\n", Str3, IonEvaluate(Str3));

	u8* Bytecode = IonGenerateBytecode(Ast);
	s32 BytecodeValue = IonVMExecute(Bytecode);

	printf("Value = %i\n", BytecodeValue);

	return(0);
}
