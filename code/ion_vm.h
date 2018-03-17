#pragma once

#define MAX_VM_STACK_COUNT 1024

struct ion_vm_stack
{
	u32 Count;
	s32 Stack[MAX_VM_STACK_COUNT];
};

internal void
PushS32Stack(ion_vm_stack* Stack, s32 Value)
{
	Assert(Stack->Count <= ArrayCount(Stack->Stack));
	Stack->Stack[Stack->Count] = Value;
	++Stack->Count;
}

internal s32
PopS32Stack(ion_vm_stack* Stack)
{
	Assert(Stack->Count != 0);
	s32 Result = Stack->Stack[Stack->Count - 1];
	--Stack->Count;

	return(Result);
}

internal s32
IonVMExecute(u8* Bytecode)
{
	Assert(Bytecode);
	bool Running = true;

	ion_vm_stack Stack = {};
	
	while(Running)
	{
		u8 Op = *Bytecode;
		++Bytecode;

		switch(Op)
		{
			case Opcode_Lit:
				{
					u32 Value = *((u32*)Bytecode);
					Bytecode += sizeof(u32);
					PushS32Stack(&Stack, (s32)Value);
				} break;
			case Opcode_Halt:
				{
					Running = false;
				} break;

			case Opcode_UnaryMin:
				{
					s32 Value = PopS32Stack(&Stack);
					PushS32Stack(&Stack, -Value);
				} break;
			case Opcode_UnaryBitNot:
				{
					s32 Value = PopS32Stack(&Stack);
					PushS32Stack(&Stack, ~Value);
				} break;
			case Opcode_Mul:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue + RightValue);
				} break;
			case Opcode_Div:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue / RightValue);
				} break;
			case Opcode_Mod:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue % RightValue);
				} break;
			case Opcode_LeftShift:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue << RightValue);
				} break;
			case Opcode_RightShift:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue >> RightValue);
				} break;
			case Opcode_BitAnd:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue & RightValue);
				} break;

			case Opcode_Add:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue + RightValue);
				} break;
			case Opcode_Sub:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue - RightValue);
				} break;
			case Opcode_BitOr:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue | RightValue);
				} break;
			case Opcode_BitXor:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					PushS32Stack(&Stack, LeftValue ^ RightValue);
				} break;
			case Opcode_Exp:
				{
					s32 RightValue = PopS32Stack(&Stack);
					s32 LeftValue = PopS32Stack(&Stack);
					s32 Value = (s32)(pow(LeftValue, RightValue));
					PushS32Stack(&Stack, Value);
				} break;
			InvalidDefaultCase;
		}
	}

	Assert(Stack.Count == 1);
	s32 Result = PopS32Stack(&Stack);
	return(Result);
}
