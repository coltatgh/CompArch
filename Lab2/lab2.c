#define WORDS_IN_MEM 0x08000
#define LC_3b_REGS 8
#define OPCODE_MASK 0xF000
#define N 0x0800
#define Z 0x0400
#define P 0x0200


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

uint16_t maskAndShiftDown(uint16_t instruction, uint16_t startBit, uint16_t endBit) {
	instruction = maskInstruction(instruction, startBit, endBit);
	instruction >>= startBit;
	return instruction;
}

uint16_t readRegister(uint16_t registerNumber) {
	return System_Latches.REGS[registerNumber];
}

void writeRegister(uint16_t registerNumber, uint16_t newValue) {
	System_Latches.REGS[registerNumber] = newValue;
}


int16_t signExtend(uint16_t signedNumber, uint16_t bitsOccupied) {
	int16_t result = 0;
	if(signedNumber & (1 << (bitsOccupied-1))) { /*negative*/
		/*take 2s compliement*/
		signedNumber ~= signedNumber;
		signedNumber += 1;
	}
	return (int16_t) signedNumber;
}

void *readMemory(uint16_t startAddress, uint16_t length) {
	/*TODO: implement, might want to make the return a void* so the user can cast the result
	  Fairly certain this read has to be word-aligned, but not 100%.
	  Address should only not be word-aligned if the person is looking for bytes, not words.
	*/
}

/* likely need to handle doing operations with numbers that I SEXT, but are cast as uint16_t*/
void execute(uint16_t instruction) {
	uint8_t opcode = (instruction & OPCODE_MASK) >> 12;
	uint16_t r1 = readRegister(maskAndShiftDown(instruction, 9,11));
	uint16_t r2 = readRegister(maskAndShiftDown(instruction, 6,8));
	uint16_t r3 = readRegister(maskAndShiftDown(instruction, 0,2));
	uint16_t bit5Mask = maskAndShiftDown(instruction, 5,5);
	uint16_t bit4Mask = maskAndShiftDown(instruction, 4,4);
	uint16_t bit11Mask = maskAndShiftDown(instruction, 11,11);
	uint16_t imm5 = maskAndShiftDown(instruction, 0,4);
	uint16_t PCOffset9 = maskAndShiftDown(instruction, 0, 8);
	uint16_t result;
	switch(opcode) {
		case ADD:
			if(bit5Mask) {
				result = r2 + signExtend(imm5);
			} else {
				result = r2 + r3;
			}
			writeRegister(r1, result);
		break;
		case AND:
			if(bit5Mask) {
				result = r2 & signExtend(imm5);
			} else {
				result = r2 & r3;
			}
			writeRegister(r1, result);
		break;
		case BR_NOP:
			/*r1 contains nzp bits*/
			if(r1) {
				/*BR*/
				bool branch = false;
				if(instruction & N) {
					if(System_Latches.N) {
						branch = true;
					}
				}
				if(instruction & Z) {
					if(System_Latches.Z) {
						branch = true;
					}
				}
				if(instruction & P) {
					if(System_Latches.P) {
						branch = true;
					}
				}
				int16_t offset = signExtend(PCOffset9);
				System_Latches.PC += (offset +1);
			} else {
				/*NOP*/
			}
		break;
		case JMP: /*works for JMP & RET b/c in RET r2 is just R7*/
			System_Latches.PC = r2;
		break;
		case JSR_R:
			System_Latches.PC++;
			writeRegister(7, System_Latches.PC);
			if(instruction & (1<<11)) {
				/*JSR*/
				int16_t PCOffset11 = signExtend(maskAndShiftDown(instruction, 0, 10));
				System_Latches.PC += (PCOffset11 << 1);
			} else {
				/*JSRR*/				
				System_Latches.PC = r2;
			}
		break;
		case LDB:
		/*8-bit contents of memory at this address are sign-extended and stored into DR*/
			int16_t offset = signExtend(maskAndShiftDown(instruction, 0,5));
			uint16_t startAddress = r2 + offset;
			uint8_t byte = 0;
			uint8_t *memory = readMemory(startAddress, 1);
			byte = memory[0];	//FIXME: probably has alignment issues
			writeRegister(maskAndShiftDown(instruction, 9,11), byte);
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