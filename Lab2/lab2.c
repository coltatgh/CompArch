#define WORDS_IN_MEM 0x08000
#define LC_3b_REGS 8
#define OPCODE_MASK 0xF000


typedef struct System_Latches_Struct{
	int PC,/* program counter */
	N, /* n condition bit */
	Z, /* z condition bit */
	P; /* p condition bit */
	int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

enum OPCODES {
	BR_NOP, /*BR, NOP*/
	ADD,
	LDB,
	STB,
	JSR_R,
	AND,
	LDW,
	STW,
	RTI,
	XOR_NOT,
	UNUSED,
	UNUSED2,
	JMP,
	SHF,
	LEA,
	TRAP
};





int MEMORY[WORDS_IN_MEM][2];

/*
returns a bitmask of instruction from startBit to endBit (inclusive)
*/
uint16_t maskInstruction(uint16_t instruction, uint16_t startBit, uint16_t endBit) {
	int i = 0;
	int result = 0;
	for(i = 0; i <= (endBit-startBit); i++) {
		result <<=1;
		result |= 1;
	}
	result <<= startBit;
	return result & instruction;
}

uint16_t readRegister(uint16_t registerNumber) {
	return System_Latches.REGS[registerNumber];
}

void writeRegister(uint16_t registerNumber, uint16_t newValue) {
	System_Latches.REGS[registerNumber] = newValue;
}

void execute(uint16_t instruction) {
	uint8_t opcode = (instruction & OPCODE_MASK) >> 12;
	uint16_t r1 = readRegister(maskInstruction(instruction, 9,11));
	uint16_t r2 = readRegister(maskInstruction(instruction, 6,8));
	uint16_t r3 = readRegister(maskInstruction(instruction, 0,2));
	uint16_t bit5Mask = maskInstruction(instruction, 5,5);
	uint16_t bit4Mask = maskInstruction(instruction, 4,4);
	uint16_t bit11Mask = maskInstruction(instruction, 11,11);



	switch(opcode) {
		case ADD:
			if()
		break;
		case AND:
		break;
		case BR_NOP:
		break;
		case JMP:
		break;
		case JSR_R:
		break;
		case LDB:
		break;
		case LDW:
		break;
		case LEA:
		break;
		case RTI:
		break;
		case SHF:
		break;
		case STB:
		break;
		case STW:
		break;
		case TRAP:
		break;
		case XOR_NOT:
		break;
		default:
		break;



		case JSR_R:
		TEMP = PC;
		if (bit(11)==0)
			PC = BaseR;
		else {
			PC = PC† + LSHF(SEXT(PCoffset11), 1);
			R7 = TEMP;
		}
		break;
	}
}