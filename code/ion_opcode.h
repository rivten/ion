#pragma once

enum opcode
{
	Opcode_Halt,
	Opcode_Lit,

	Opcode_UnaryMin,
	Opcode_UnaryBitNot,
	Opcode_Mul,
	Opcode_Div,
	Opcode_Mod,
	Opcode_LeftShift,
	Opcode_RightShift,
	Opcode_BitAnd,

	Opcode_Add,
	Opcode_Sub,
	Opcode_BitOr,
	Opcode_BitXor,

	Opcode_Exp,

	Opcode_Count,
};

internal opcode
GetOpcode(operator_type Op)
{
	switch(Op)
	{
		case Op_UnaryMin:
			{
				return(Opcode_UnaryMin);
			} break;
		case Op_UnaryBitNot:
			{
				return(Opcode_UnaryBitNot);
			} break;
		case Op_Mul:
			{
				return(Opcode_Mul);
			} break;
		case Op_Div:
			{
				return(Opcode_Div);
			} break;
		case Op_Mod:
			{
				return(Opcode_Mod);
			} break;
		case Op_LeftShift:
			{
				return(Opcode_LeftShift);
			} break;
		case Op_RightShift:
			{
				return(Opcode_RightShift);
			} break;
		case Op_BitAnd:
			{
				return(Opcode_BitAnd);
			} break;
		case Op_Add:
			{
				return(Opcode_Add);
			} break;
		case Op_Sub:
			{
				return(Opcode_Sub);
			} break;
		case Op_BitOr:
			{
				return(Opcode_BitOr);
			} break;
		case Op_BitXor:
			{
				return(Opcode_BitXor);
			} break;
		case Op_Exp:
			{
				return(Opcode_Exp);
			} break;
		InvalidDefaultCase;
	}
	InvalidCodePath;
	return(Opcode_Count);
}

