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

