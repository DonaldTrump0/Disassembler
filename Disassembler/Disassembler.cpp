#include <stdio.h>
#include <Windows.h>

const char* r8[8] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
const char* r32[8] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };
const char* jcc[16] = { "JO", "JNO", "JB/JNAE/JC", "JNB/JAE/JNC", "JZ/JE", "JNZ/JNE", "JBE/JNA", "JNBE/JA",
						"JS", "JNS", "JP/JPE", "JNP/JPO", "JL/JNGE", "JNL/JGE", "JLE/JNG", "JNLE/JG" };

char seg[10] = { 0 };
int segCnt = 0;

void getBase(char* baseStr, size_t mod, size_t base) {
	if (mod == 0 && base == 5) {
		sprintf(baseStr, "disp32");
	}
	else {
		sprintf(baseStr, "%s", r32[base]);
	}
}

void getScaledIndex(char* scaledIndex, size_t scale, size_t index) {
	if (index == 4) {
		return;
	}

	switch (scale)
	{
	case 0:
		sprintf(scaledIndex, "%s", r32[index]);
		break;
	case 1:
		sprintf(scaledIndex, "%s * 2", r32[index]);
		break;
	case 2:
		sprintf(scaledIndex, "%s * 4", r32[index]);
		break;
	case 3:
		sprintf(scaledIndex, "%s * 8", r32[index]);
		break;
	}
}

void getSIB(char* sibStr, size_t mod) {
	size_t sib;
	scanf("%x", &sib);

	size_t scale = sib >> 6;
	size_t index = (sib & 0x38) >> 3;
	size_t base = sib & 0x07;

	char baseStr[20] = { 0 };
	char scaledIndex[20] = { 0 };
	getBase(baseStr, mod, base);
	getScaledIndex(scaledIndex, scale, index);

	if (!*scaledIndex) {
		sprintf(sibStr, "%s", baseStr);
	}
	else {
		if (strcmp(baseStr, "disp32") == 0) {
			sprintf(sibStr, "%s + %s", scaledIndex, baseStr);
		}
		else {
			sprintf(sibStr, "%s + %s", baseStr, scaledIndex);
		}
	}
}

// 获取Gb或Gv
void getG(char* G, size_t regOpcode, bool isByte) {
	if (isByte) {
		sprintf(G, "%s", r8[regOpcode]);
	}
	else {
		sprintf(G, "%s", r32[regOpcode]);
	}
}

// 获取Eb或Ev
void getE(char* E, size_t mod, size_t RM, bool isByte) {
	if (mod == 3) {
		if (isByte) {
			sprintf(E, "%s", r8[RM]);
		}
		else {
			sprintf(E, "%s", r32[RM]);
		}
		return;
	}

	char prefix[20] = { 0 };
	if (isByte) {
		sprintf(prefix, "BYTE PTR");
	}
	else {
		sprintf(prefix, "DWORD PTR");
	}

	char memAddr[50] = { 0 };
	if (mod == 0 && RM == 5) {
		sprintf(memAddr, "disp32");
	}
	else if (RM == 4) {
		getSIB(memAddr, mod);
	}
	else {
		sprintf(memAddr, "%s", r32[RM]);
	}

	if (!segCnt) {
		memcpy(seg, memAddr, 3);
		seg[3] = 0;
		if (!strcmp(seg, "ESP") || !strcmp(seg, "EBP")) {
			sprintf(seg, "SS");
		}
		else {
			sprintf(seg, "DS");
		}
	}

	switch (mod)
	{
	case 0:
		sprintf(E, "%s %s:[%s]", prefix, seg, memAddr);
		break;
	case 1:
		sprintf(E, "%s %s:[%s + disp8]", prefix, seg, memAddr);
		break;
	case 2:
		sprintf(E, "%s %s:[%s + disp32]", prefix, seg, memAddr);
		break;
	}
}

// 获取EbGb或EvGv或GbEb或GvEv
void getEG(char* EG, bool isByte, bool isEG) {
	size_t modRM;
	scanf("%x", &modRM);

	size_t mod = modRM >> 6;
	size_t regOpcode = (modRM & 0x38) >> 3;
	size_t RM = modRM & 0x07;

	char E[50] = { 0 };
	char G[20] = { 0 };
	getE(E, mod, RM, isByte);
	getG(G, regOpcode, isByte);

	if (isEG) {
		sprintf(EG, "%s, %s", E, G);
	}
	else {
		sprintf(EG, "%s, %s", G, E);
	}
}

int main() {
	size_t opcode;
	char EG[50] = { 0 };

	while (true) {
		scanf("%x", &opcode);

		if (segCnt) {
			segCnt--;
		}

		switch (opcode)
		{
		case 0x00:
			// ADD Eb, Gb
			getEG(EG, true, true);
			printf("ADD %s\n", EG);
			break;
		case 0x01:
			// ADD Ev, Gv
			getEG(EG, false, true);
			printf("ADD %s\n", EG);
			break;
		case 0x02:
			// ADD Gb, Eb
			getEG(EG, true, false);
			printf("ADD %s\n", EG);
			break;
		case 0x03:
			// ADD Gv, Ev
			getEG(EG, false, false);
			printf("ADD %s\n", EG);
			break;
		case 0x04:
			// ADD AL, Ib
			printf("ADD AL, Ib\n");
			break;
		case 0x05:
			// ADD rAX, Iz
			printf("ADD EAX, Id\n");
			break;
		case 0x06:
			// PUSH ES
			printf("PUSH ES\n");
			break;
		case 0x07:
			// POP ES
			printf("POP ES\n");
			break;

		case 0x10:
			// ADC Eb, Gb
			getEG(EG, true, true);
			printf("ADC %s\n", EG);
			break;
		case 0x11:
			// ADC Ev, Gv
			getEG(EG, false, true);
			printf("ADC %s\n", EG);
			break;
		case 0x12:
			// ADC Gb, Eb
			getEG(EG, true, false);
			printf("ADC %s\n", EG);
			break;
		case 0x13:
			// ADC Gv, Ev
			getEG(EG, false, false);
			printf("ADC %s\n", EG);
			break;
		case 0x14:
			// ADC AL, Ib
			printf("ADC AL, Ib\n");
			break;
		case 0x15:
			// ADC rAX, Iz
			printf("ADC EAX, Id\n");
			break;
		case 0x16:
			// PUSH SS
			printf("PUSH SS\n");
			break;
		case 0x17:
			// POP SS
			printf("POP SS\n");
			break;

		case 0x20:
			// AND Eb, Gb
			getEG(EG, true, true);
			printf("AND %s\n", EG);
			break;
		case 0x21:
			// AND Ev, Gv
			getEG(EG, false, true);
			printf("AND %s\n", EG);
			break;
		case 0x22:
			// AND Gb, Eb
			getEG(EG, true, false);
			printf("AND %s\n", EG);
			break;
		case 0x23:
			// AND Gv, Ev
			getEG(EG, false, false);
			printf("AND %s\n", EG);
			break;
		case 0x24:
			// AND AL, Ib
			printf("AND AL, Ib\n");
			break;
		case 0x25:
			// AND rAX, Iz
			printf("AND EAX, Id\n");
			break;
		case 0x26:
			// SEG = ES(Prefix)
			sprintf(seg, "ES");
			segCnt = 2;
			break;
		case 0x27:
			// DAA
			printf("DAA\n");
			break;

		case 0x30:
			// XOR Eb, Gb
			getEG(EG, true, true);
			printf("XOR %s\n", EG);
			break;
		case 0x31:
			// XOR Ev, Gv
			getEG(EG, false, true);
			printf("XOR %s\n", EG);
			break;
		case 0x32:
			// XOR Gb, Eb
			getEG(EG, true, false);
			printf("XOR %s\n", EG);
			break;
		case 0x33:
			// XOR Gv, Ev
			getEG(EG, false, false);
			printf("XOR %s\n", EG);
			break;
		case 0x34:
			// XOR AL, Ib
			printf("XOR AL, Ib\n");
			break;
		case 0x35:
			// XOR rAX, Iz
			printf("XOR EAX, Id\n");
			break;
		case 0x36:
			// SEG = SS(Prefix)
			sprintf(seg, "SS");
			segCnt = 2;
			break;
		case 0x37:
			// AAA
			printf("AAA\n");
			break;

		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			// INC ERX
			printf("INC %s\n", r32[opcode - 0x40]);
			break;

		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
			// PUSH ERX
			printf("PUSH %s\n", r32[opcode - 0x50]);
			break;

		case 0x60:
			// PUSHA/PUSHAD
			printf("PUSHA/PUSHAD\n");
			break;
		case 0x61:
			// POPA/POPAD
			printf("POPA/POPAD\n");
			break;
		case 0x62:
			// BOUND Gv, Ma
			printf("BOUND Gv, Ma(未解析)\n");
			break;
		case 0x63:
			// ARPL Ew, Gw
			printf("BOUND Gv, Ma(未解析)\n");
			break;


			// POP ERX
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
			printf("POP %s\n", r32[opcode - 0x58]);
			break;

			// DEC ERX
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
			printf("DEC %s\n", r32[opcode - 0x48]);
			break;

			// MOV Rb, Ib
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
			printf("MOV %s,Ib\n", r8[opcode - 0xB0]);
			break;

			// MOV ERX, Id
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
			printf("MOV %s,Id\n", r32[opcode - 0xB8]);
			break;

			// XCHG EAX, ERX
		case 0x90:
			printf("NOP\n");
			break;
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
			printf("XCHG EAX,%s\n", r32[opcode - 0x90]);
			break;

			// JCC Ib
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
			printf("%s Ib\n", jcc[opcode - 0x70]);
			break;

		case 0x0F:
			scanf("%x", &opcode);
			switch (opcode)
			{
				// JCC Id
			case 0x80:
			case 0x81:
			case 0x82:
			case 0x83:
			case 0x84:
			case 0x85:
			case 0x86:
			case 0x87:
			case 0x88:
			case 0x89:
			case 0x8A:
			case 0x8B:
			case 0x8C:
			case 0x8D:
			case 0x8E:
			case 0x8F:
				printf("%s Id\n", jcc[opcode - 0x80]);
				break;
			}

		case 0xE0:
			printf("LOOPNE/LOOPNZ Ib\n");
			break;
		case 0xE1:
			printf("LOOPE/LOOPZ Ib\n");
			break;
		case 0xE2:
			printf("LOOP Ib\n");
			break;
		case 0xE3:
			printf("JrCXZ Ib\n");
			break;

		case 0xE8:
			printf("CALL Id\n");
			break;
		case 0xE9:
			printf("JMP Id\n");
			break;
		case 0xEA:
			printf("JMP CS:Id\n");
			break;
		case 0xEB:
			printf("JMP Ib\n");
			break;

		case 0xC3:
			printf("RET\n");
			break;
		case 0xC2:
			printf("RET Iw\n");
			break;
		case 0xCB:
			printf("RETF\n");
			break;
		case 0xCA:
			printf("RETF Iw\n");
			break;


			/*case 0xCA:
				printf("RETF Iw\n");
				break;
			case 0xCA:
				printf("RETF Iw\n");
				break;*/

		}
	}
	return 0;
}