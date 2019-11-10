#include <stdio.h>
#include <Windows.h>

const char* r8[8] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
const char* r32[8] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };
const char* JCC[16] = { "JO", "JNO", "JB/JNAE/JC", "JNB/JAE/JNC", "JZ/JE", "JNZ/JNE", "JBE/JNA", "JNBE/JA",
						"JS", "JNS", "JP/JPE", "JNP/JPO", "JL/JNGE", "JNL/JGE", "JLE/JNG", "JNLE/JG" };

void getBase(char* BaseStr, size_t Mod, size_t Base) {
	if (Mod == 0 && Base == 5) {
		sprintf(BaseStr, "disp32");
	}
	else {
		sprintf(BaseStr, "%s", r32[Base]);
	}
}

void getScaledIndex(char* ScaledIndex, size_t Scale, size_t Index) {
	if (Index == 4) {
		return;
	}

	switch (Scale)
	{
	case 0:
		sprintf(ScaledIndex, "%s", r32[Index]);
		break;
	case 1:
		sprintf(ScaledIndex, "%s*2", r32[Index]);
		break;
	case 2:
		sprintf(ScaledIndex, "%s*4", r32[Index]);
		break;
	case 3:
		sprintf(ScaledIndex, "%s*8", r32[Index]);
		break;
	}
}

void getSIB(char* sibStr, size_t Mod) {
	size_t sib;
	scanf("%x", &sib);

	size_t Scale = sib & 0xC0;
	size_t Index = sib & 0x38;
	size_t Base = sib & 0x07;

	char BaseStr[20] = { 0 };
	char ScaledIndex[20] = { 0 };
	getBase(BaseStr, Mod, Base);
	getScaledIndex(ScaledIndex, Scale, Index);

	if (!ScaledIndex) {
		sprintf(sibStr, "%s", BaseStr);
	}
	else {
		sprintf(sibStr, "%s+%s", BaseStr, ScaledIndex);
	}
}

// 获取Gb或Gv
void getG(char* G, size_t RegOpcode, bool isByte) {
	if (isByte) {
		sprintf(G, "%s", r8[RegOpcode]);
	}
	else {
		sprintf(G, "%s", r32[RegOpcode]);
	}
}

// 获取Eb或Ev
void getE(char* E, size_t Mod, size_t RM, bool isByte) {
	if (Mod == 3) {
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

	char addr[50] = { 0 };
	if (Mod == 0 && RM == 5) {
		sprintf(addr, "disp32");
	}
	else if (RM == 4) {
		getSIB(addr, Mod);
	}
	else {
		sprintf(addr, "%s", r32[RM]);
	}

	switch (Mod)
	{
	case 0:
		sprintf(E, "%s DS:[%s]", prefix, addr);
		break;
	case 1:
		sprintf(E, "%s DS:[%s+disp8]", prefix, addr);
		break;
	case 2:
		sprintf(E, "%s DS:[%s+disp32]", prefix, addr);
		break;
	}
}

// 获取EbGb或EvGv或GbEb或GvEv
void getEG(char* EG, bool isByte, bool isEG) {
	size_t ModRM;
	scanf("%x", &ModRM);

	size_t Mod = ModRM & 0xC0;
	size_t RegOpcode = ModRM & 0x38;
	size_t RM = ModRM & 0x07;

	char E[50] = { 0 };
	char G[20] = { 0 };
	getE(E, Mod, RM, isByte);
	getG(G, RegOpcode, isByte);

	if (isEG) {
		sprintf(EG, "%s,%s", E, G);
	}
	else {
		sprintf(EG, "%s,%s", G, E);
	}
}

int main() {
	size_t Opcode;
	char EG[50] = { 0 };

	while (true) {
		scanf("%x", &Opcode);
		switch (Opcode)
		{
		case 0x00:
			// ADD Eb, Gb
			getEG(EG, true, true);
			printf("ADD %s\n", EG);
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
			printf("PUSH %s\n", r32[Opcode - 0x50]);
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
			printf("POP %s\n", r32[Opcode - 0x58]);
			break;

			// INC ERX
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
			printf("INC %s\n", r32[Opcode - 0x40]);
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
			printf("DEC %s\n", r32[Opcode - 0x48]);
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
			printf("MOV %s,Ib\n", r8[Opcode - 0xB0]);
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
			printf("MOV %s,Id\n", r32[Opcode - 0xB8]);
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
			printf("XCHG EAX,%s\n", r32[Opcode - 0x90]);
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
			printf("%s Ib\n", JCC[Opcode - 0x70]);
			break;

		case 0x0F:
			scanf("%x", &Opcode);
			switch (Opcode)
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
				printf("%s Id\n", JCC[Opcode - 0x80]);
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