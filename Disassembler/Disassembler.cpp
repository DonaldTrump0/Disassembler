#include <stdio.h>
#include <Windows.h>

#define SEG_PREFIX_INDEX 0				// 段前缀指令索引
#define LOCK_REP_PREFIX_INDEX 1			// LOCK、REP、REPNE前缀指令索引
#define OPERAND_SIZE_PREFIX_INDEX 2		// 操作数宽度前缀指令索引
#define ADDR_SIZE_PREFIX_INDEX 3		// 地址宽度前缀指令索引

const char* r8[8] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
const char* r16[8] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI", };
const char* r32[8] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };
const char* jcc[16] = { "JO", "JNO", "JB/JNAE/JC", "JNB/JAE/JNC", "JZ/JE", "JNZ/JNE", "JBE/JNA", "JNBE/JA",
						"JS", "JNS", "JP/JPE", "JNP/JPO", "JL/JNGE", "JNL/JGE", "JLE/JNG", "JNLE/JG" };

char buf[50] = { 0 };

// 前缀指令计数
int prefixCnt[4] = { 0 };

// 段前缀指令存储
char segPrefix[10];
// LOCK、REP、REPNE前缀指令存储
char lockRepPrefix[10];

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

// 获取段前缀
void getSegPrefix(const char* baseStr) {
	if (!prefixCnt[SEG_PREFIX_INDEX]) {
		if (!strcmp(baseStr, "ESP") || !strcmp(baseStr, "EBP")) {
			sprintf(segPrefix, "SS");
		}
		else {
			sprintf(segPrefix, "DS");
		}
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
	getSegPrefix(baseStr);

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

// 获取Gb，Gw，Gv
void getG(char* G, size_t regOpcode, size_t opSize) {
	switch (opSize)
	{
	case 1:
		sprintf(G, "%s", r8[regOpcode]);
		break;
	case 2:
		sprintf(G, "%s", r16[regOpcode]);
		break;
	case 4:
		sprintf(G, "%s", r32[regOpcode]);
		break;
	}
}

// 获取Eb，Ew，Ev
void getE(char* E, size_t mod, size_t RM, size_t opSize) {
	if (mod == 3) {
		switch (opSize)
		{
		case 1:
			sprintf(E, "%s", r8[RM]);
			break;
		case 2:
			sprintf(E, "%s", r16[RM]);
			break;
		case 4:
			sprintf(E, "%s", r32[RM]);
			break;
		}
		return;
	}

	char prefix[20] = { 0 };
	switch (opSize)
	{
	case 1:
		sprintf(prefix, "BYTE PTR");
		break;
	case 2:
		sprintf(prefix, "WORD PTR");
		break;
	case 4:
		sprintf(prefix, "DWORD PTR");
		break;
	}

	char baseStr[50] = { 0 };
	if (mod == 0 && RM == 5) {
		sprintf(baseStr, "disp32");
	}
	else if (RM == 4) {
		getSIB(baseStr, mod);
	}
	else {
		sprintf(baseStr, "%s", r32[RM]);
		getSegPrefix(baseStr);
	}

	switch (mod)
	{
	case 0:
		sprintf(E, "%s %s:[%s]", prefix, segPrefix, baseStr);
		break;
	case 1:
		sprintf(E, "%s %s:[%s + disp8]", prefix, segPrefix, baseStr);
		break;
	case 2:
		sprintf(E, "%s %s:[%s + disp32]", prefix, segPrefix, baseStr);
		break;
	}
}

// 获取EbGb，EwGw，EvGv，GbEb，GwEw，GvEv
void getEG(size_t opSize, bool isEG) {
	size_t modRM;
	scanf("%x", &modRM);

	size_t mod = modRM >> 6;
	size_t regOpcode = (modRM & 0x38) >> 3;
	size_t RM = modRM & 0x07;

	char E[50] = { 0 };
	char G[20] = { 0 };
	getE(E, mod, RM, opSize);
	getG(G, regOpcode, opSize);

	if (isEG) {
		sprintf(buf, "%s, %s", E, G);
	}
	else {
		sprintf(buf, "%s, %s", G, E);
	}
}

// 获取带有group的指令
void getGrpInstruct(size_t opSize, size_t group) {
	size_t modRM;
	scanf("%x", &modRM);

	size_t mod = modRM >> 6;
	size_t regOpcode = (modRM & 0x38) >> 3;
	size_t RM = modRM & 0x07;

	char E[50] = { 0 };
	getE(E, mod, RM, opSize);

	switch (group)
	{

	case 0:
		// group 1A
		switch (regOpcode)
		{
		case 0:
			sprintf(buf, "POP %s", E);
			break;
		default:
			sprintf(buf, "???");
			break;
		}
		break;
	case 1:
		switch (regOpcode)
		{
		case 0:
			sprintf(buf, "ADD %s", E);
			break;
		case 1:
			sprintf(buf, "OR %s", E);
			break;
		case 2:
			sprintf(buf, "ADC %s", E);
			break;
		case 3:
			sprintf(buf, "SBB %s", E);
			break;
		case 4:
			sprintf(buf, "AND %s", E);
			break;
		case 5:
			sprintf(buf, "SUB %s", E);
			break;
		case 6:
			sprintf(buf, "XOR %s", E);
			break;
		case 7:
			sprintf(buf, "CMP %s", E);
			break;
		}
		break;
	case 2:
		switch (regOpcode) 
		{
		case 0:
			sprintf(buf, "ROL %s", E);
			break;
		case 1:
			sprintf(buf, "ROR %s", E);
			break;
		case 2:
			sprintf(buf, "RCL %s", E);
			break;
		case 3:
			sprintf(buf, "RCR %s", E);
			break;
		case 4:
			sprintf(buf, "SHL/SAL %s", E);
			break;
		case 5:
			sprintf(buf, "SHR %s", E);
			break;
		case 6:
			sprintf(buf, "???");
			break;
		case 7:
			sprintf(buf, "SAR %s", E);
			break;
		}
		break;
	case 3:
		switch (regOpcode)
		{
		case 0:
			sprintf(buf, "TEST %s, Ib", E);
			break;
		case 1:
			sprintf(buf, "???");
			break;
		case 2:
			sprintf(buf, "NOT %s", E);
			break;
		case 3:
			sprintf(buf, "NEG %s", E);
			break;
		case 4:
			sprintf(buf, "MUL %s", E);
			break;
		case 5:
			sprintf(buf, "IMUL %s", E);
			break;
		case 6:
			sprintf(buf, "DIV %s", E);
			break;
		case 7:
			sprintf(buf, "IDIV %s", E);
			break;
		}
		break;
	case 11:
		switch (regOpcode)
		{
		case 0:
			sprintf(buf, "MOV %s", E);
			break;
		default:
			sprintf(buf, "???");
			break;
		}
		break;
	}
}

// 获取Ob，Ow，Ov
void getO(size_t opSize) {
	char prefix[20] = { 0 };
	switch (opSize)
	{
	case 1:
		sprintf(prefix, "BYTE PTR");
		break;
	case 2:
		sprintf(prefix, "WORD PTR");
		break;
	case 4:
		sprintf(prefix, "DWORD PTR");
		break;
	}

	getSegPrefix("");

	sprintf(buf, "%s %s:[Id]", prefix, segPrefix);
}

// 处理指令前缀
void disposePrefix(const char* prefixStr) {
	size_t prefixIndex;

	if (!strcmp(prefixStr, "ADDRSIZE")) {
		//prefixIndex = ADDR_SIZE_PREFIX_INDEX;
		// 暂不做处理 ，直接输出
		printf("PREFIX %s:\n", prefixStr);
		return;
	}
	else if (!strcmp(prefixStr, "OPSIZE")) {
		//prefixIndex = OPERAND_SIZE_PREFIX_INDEX;
		// 暂不做处理 ，直接输出
		printf("PREFIX %s:\n", prefixStr);
		return;
	}
	else if (!strcmp(prefixStr, "LOCK") || !strcmp(prefixStr, "REP") || !strcmp(prefixStr, "REPNE")) {
		//prefixIndex = LOCK_REP_PREFIX_INDEX;
		// 暂不做处理 ，直接输出
		printf("PREFIX %s:\n", prefixStr);
		return;
	}
	else {
		prefixIndex = SEG_PREFIX_INDEX;
	}

	switch (prefixIndex)
	{
	case SEG_PREFIX_INDEX:
		// 如果已经有同类型的前缀指令，则打印
		if (prefixCnt[prefixIndex]) {
			printf("PREFIX %s:\n", segPrefix);
		}
		// 保存新的前缀指令
		sprintf(segPrefix, prefixStr);
		break;
	/*case LOCK_REP_PREFIX_INDEX:
		if (prefixCnt[prefixIndex]) {
			printf("PREFIX %s:\n", lockRepPrefix);
		}
		sprintf(lockRepPrefix, prefixStr);
		break;*/
	case OPERAND_SIZE_PREFIX_INDEX:
	case ADDR_SIZE_PREFIX_INDEX:
		if (prefixCnt[prefixIndex]) {
			printf("PREFIX %s:\n", prefixStr);
		}
		break;
	}

	// 更新前缀指令计数
	prefixCnt[prefixIndex] = 2;
	for (size_t i = 0; i < 4; i++) {
		if (prefixCnt[i]) {
			prefixCnt[i] = 2;
		}
	}
}

int main() {
	size_t opcode;

	while (true) {
		scanf("%x", &opcode);

		for (size_t i = 0; i < 4; i++) {
			if (prefixCnt[i]) {
				prefixCnt[i]--;
			}
		}

		switch (opcode)
		{
		case 0x00:
			// ADD Eb, Gb
			getEG(1, true);
			printf("ADD %s\n", buf);
			break;
		case 0x01:
			// ADD Ev, Gv
			getEG(4, true);
			printf("ADD %s\n", buf);
			break;
		case 0x02:
			// ADD Gb, Eb
			getEG(1, false);
			printf("ADD %s\n", buf);
			break;
		case 0x03:
			// ADD Gv, Ev
			getEG(4, false);
			printf("ADD %s\n", buf);
			break;
		case 0x04:
			printf("ADD AL, Ib\n");
			break;
		case 0x05:
			printf("ADD EAX, Id\n");
			break;
		case 0x06:
			printf("PUSH ES\n");
			break;
		case 0x07:
			printf("POP ES\n");
			break;
		case 0x08:
			// OR Eb, Gb
			getEG(1, true);
			printf("OR %s\n", buf);
			break;
		case 0x09:
			// OR Ev, Gv
			getEG(4, true);
			printf("OR %s\n", buf);
			break;
		case 0x0A:
			// OR Gb, Eb
			getEG(1, false);
			printf("OR %s\n", buf);
			break;
		case 0x0B:
			// OR Gv, Ev
			getEG(4, false);
			printf("OR %s\n", buf);
			break;
		case 0x0C:
			printf("OR AL, Ib\n");
			break;
		case 0x0D:
			printf("OR EAX, Id\n");
			break;
		case 0x0E:
			printf("PUSH CS\n");
			break;
		case 0x0F:
			// 2-byte escape (Table A - 3)
			printf("\n");
			break;

		case 0x10:
			// ADC Eb, Gb
			getEG(1, true);
			printf("ADC %s\n", buf);
			break;
		case 0x11:
			// ADC Ev, Gv
			getEG(4, true);
			printf("ADC %s\n", buf);
			break;
		case 0x12:
			// ADC Gb, Eb
			getEG(1, false);
			printf("ADC %s\n", buf);
			break;
		case 0x13:
			// ADC Gv, Ev
			getEG(4, false);
			printf("ADC %s\n", buf);
			break;
		case 0x14:
			printf("ADC AL, Ib\n");
			break;
		case 0x15:
			printf("ADC EAX, Id\n");
			break;
		case 0x16:
			printf("PUSH SS\n");
			break;
		case 0x17:
			printf("POP SS\n");
			break;
		case 0x18:
			// SBB Eb, Gb
			getEG(1, true);
			printf("SBB %s\n", buf);
			break;
		case 0x19:
			// SBB Ev, Gv
			getEG(4, true);
			printf("SBB %s\n", buf);
			break;
		case 0x1A:
			// SBB Gb, Eb
			getEG(1, false);
			printf("SBB %s\n", buf);
			break;
		case 0x1B:
			// SBB Gv, Ev
			getEG(4, false);
			printf("SBB %s\n", buf);
			break;
		case 0x1C:
			printf("SBB AL, Ib\n");
			break;
		case 0x1D:
			printf("SBB EAX, Id\n");
			break;
		case 0x1E:
			printf("PUSH DS\n");
			break;
		case 0x1F:
			printf("POP DS\n");
			break;

		case 0x20:
			// AND Eb, Gb
			getEG(1, true);
			printf("AND %s\n", buf);
			break;
		case 0x21:
			// AND Ev, Gv
			getEG(4, true);
			printf("AND %s\n", buf);
			break;
		case 0x22:
			// AND Gb, Eb
			getEG(1, false);
			printf("AND %s\n", buf);
			break;
		case 0x23:
			// AND Gv, Ev
			getEG(4, false);
			printf("AND %s\n", buf);
			break;
		case 0x24:
			printf("AND AL, Ib\n");
			break;
		case 0x25:
			printf("AND EAX, Id\n");
			break;
		case 0x26:
			// SEG = ES(Prefix)
			disposePrefix("ES");
			break;
		case 0x27:
			printf("DAA\n");
			break;
		case 0x28:
			// SUB Eb, Gb
			getEG(1, true);
			printf("SUB %s\n", buf);
			break;
		case 0x29:
			// SUB Ev, Gv
			getEG(4, true);
			printf("SUB %s\n", buf);
			break;
		case 0x2A:
			// SUB Gb, Eb
			getEG(1, false);
			printf("SUB %s\n", buf);
			break;
		case 0x2B:
			// SUB Gv, Ev
			getEG(4, false);
			printf("SUB %s\n", buf);
			break;
		case 0x2C:
			printf("SUB AL, Ib\n");
			break;
		case 0x2D:
			printf("SUB EAX, Id\n");
			break;
		case 0x2E:
			// SEG = CS(Prefix)
			disposePrefix("CS");
			break;
		case 0x2F:
			printf("DAS\n");
			break;

		case 0x30:
			// XOR Eb, Gb
			getEG(1, true);
			printf("XOR %s\n", buf);
			break;
		case 0x31:
			// XOR Ev, Gv
			getEG(4, true);
			printf("XOR %s\n", buf);
			break;
		case 0x32:
			// XOR Gb, Eb
			getEG(1, false);
			printf("XOR %s\n", buf);
			break;
		case 0x33:
			// XOR Gv, Ev
			getEG(4, false);
			printf("XOR %s\n", buf);
			break;
		case 0x34:
			printf("XOR AL, Ib\n");
			break;
		case 0x35:
			printf("XOR EAX, Id\n");
			break;
		case 0x36:
			// SEG = SS(Prefix)
			disposePrefix("SS");
			break;
		case 0x37:
			printf("AAA\n");
			break;
		case 0x38:
			// CMP Eb, Gb
			getEG(1, true);
			printf("CMP %s\n", buf);
			break;
		case 0x39:
			// CMP Ev, Gv
			getEG(4, true);
			printf("CMP %s\n", buf);
			break;
		case 0x3A:
			// CMP Gb, Eb
			getEG(1, false);
			printf("CMP %s\n", buf);
			break;
		case 0x3B:
			// CMP Gv, Ev
			getEG(4, false);
			printf("CMP %s\n", buf);
			break;
		case 0x3C:
			printf("CMP AL, Ib\n");
			break;
		case 0x3D:
			printf("CMP EAX, Id\n");
			break;
		case 0x3E:
			// SEG = DS(Prefix)
			disposePrefix("DS");
			break;
		case 0x3F:
			printf("AAS\n");
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
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
			// DEC ERX
			printf("DEC %s\n", r32[opcode - 0x48]);
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
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
			// POP ERX
			printf("POP %s\n", r32[opcode - 0x58]);
			break;

		case 0x60:
			// PUSHA/PUSHAD
			printf("PUSHAD\n");
			break;
		case 0x61:
			// POPA/POPAD
			printf("POPAD\n");
			break;
		case 0x62:
			// BOUND Gv, Ma
			printf("BOUND Gv, Ma(未解析)\n");
			break;
		case 0x63:
			// ARPL Ew, Gw
			getEG(2, true);
			printf("ARPL %s\n", buf);
			break;
		case 0x64:
			// SEG = FS(Prefix)
			disposePrefix("FS");
			break;
		case 0x65:
			// SEG = GS(Prefix)
			disposePrefix("GS");
			break;
		case 0x66:
			// Operand Size(Prefix)
			disposePrefix("OPSIZE");
			break;
		case 0x67:
			// Address Size(Prefix)
			disposePrefix("ADDRSIZE");
			break;
		case 0x68:
			printf("PUSH Id\n");
			break;
		case 0x69:
			// IMUL Gv, Ev, Iz
			getEG(4, false);
			printf("IMUL %s, Id\n", buf);
			break;
		case 0x6A:
			printf("PUSH Ib\n");
			break;
		case 0x6B:
			// IMUL Gv, Ev, Ib
			getEG(4, false);
			printf("IMUL %s, Ib\n", buf);
			break;
		case 0x6C:
			// INS/INSB Yb, DX
			printf("INS BYTE PTR ES:[EDI], DX\n");
			break;
		case 0x6D:
			// INS/INSW/INSD Yz, DX
			printf("INS DWORD PTR ES:[EDI], DX\n");
			break;
		case 0x6E:
			// OUTS/OUTSB DX, Xb
			printf("OUTS DX, BYTE PTR ES:[EDI]\n");
			break;
		case 0x6F:
			// OUTS/OUTSW/OUTSD DX, Xz
			printf("OUTS DX, DWORD PTR ES:[EDI]\n");
			break;

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
			// JCC Ib
			printf("%s Ib\n", jcc[opcode - 0x70]);
			break;

		case 0x80:
			// Grp1 Eb, Ib
			getGrpInstruct(1, 1);
			printf("%s, Ib\n", buf);
			break;
		case 0x81:
			// Grp1 Ev, Iz
			getGrpInstruct(4, 1);
			printf("%s, Id\n", buf);
			break;
		case 0x82:
			// Grp1 Eb, Ib
			getGrpInstruct(1, 1);
			printf("%s, Ib\n", buf);
			break;
		case 0x83:
			// Grp1 Ev, Ib
			getGrpInstruct(4, 1);
			printf("%s, Id\n", buf);
			break;
		case 0x84:
			// TEST Eb, Gb
			getEG(1, true);
			printf("TEST %s\n", buf);
			break;
		case 0x85:
			// TEST Ev, Gv
			getEG(4, true);
			printf("TEST %s\n", buf);
			break;
		case 0x86:
			// XCHG Eb, Gb
			getEG(1, true);
			printf("XCHG %s\n", buf);
			break;
		case 0x87:
			// XCHG Ev, Gv
			getEG(4, true);
			printf("XCHG %s\n", buf);
			break;
		case 0x88:
			// MOV Eb, Gb
			getEG(1, true);
			printf("MOV %s\n", buf);
			break;
		case 0x89:
			// MOV Ev, Gv
			getEG(4, true);
			printf("MOV %s\n", buf);
			break;
		case 0x8A:
			// MOV Gb, Eb
			getEG(1, false);
			printf("MOV %s\n", buf);
			break;
		case 0x8B:
			// MOV Gv, Ev
			getEG(4, false);
			printf("MOV %s\n", buf);
			break;
		case 0x8C:
			// MOV Ev, Sw
			printf("MOV Ev, Sw(未解析)\n");
			break;
		case 0x8D:
			// LEA Gv, M
			printf("LEA Gv, M(未解析)\n");
			break;
		case 0x8E:
			// MOV Sw, Ew
			printf("MOV Sw, Ew(未解析)\n");
			break;
		case 0x8F:
			// Grp1A POP Ev
			getGrpInstruct(4, 0);
			printf("%s\n", buf);
			break;

		case 0x90:
			// NOP
			printf("NOP\n");
			break;
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
			// XCHG EAX, ERX
			printf("XCHG EAX, %s\n", r32[opcode - 0x90]);
			break;
		case 0x98:
			// CBW/CWDE/CDQE
			printf("CWDE\n");
			break;
		case 0x99:
			// CWD/CDQ/CQO
			printf("CDQ\n");
			break;
		case 0x9A:
			// CALLF Ap
			printf("CALL FAR Iw:Ib\n");
			break;
		case 0x9B:
			// FWAIT/WAIT
			printf("WAIT\n");
			break;
		case 0x9C:
			// PUSHF/D/Q/Fv
			printf("PUSHFD\n");
			break;
		case 0x9D:
			// POPF/D/Q/Fv
			printf("POPFD\n");
			break;
		case 0x9E:
			printf("SAHF\n");
			break;
		case 0x9F:
			printf("LAHF\n");
			break;

		case 0xA0:
			// MOV AL, Ob
			getO(1);
			printf("MOV AL, %s\n", buf);
			break;
		case 0xA1:
			// MOV rAX, Ov
			getO(4);
			printf("MOV EAX, %s\n", buf);
			break;
		case 0xA2:
			// MOV Ob, AL
			getO(1);
			printf("MOV %s, AL\n", buf);
			break;
		case 0xA3:
			// MOV Ov, rAX
			getO(4);
			printf("MOV %s, EAX\n", buf);
			break;
		case 0xA4:
			// MOVS/B Yb, Xb
			printf("MOVS BYTE PTR ES:[EDI], BYTE PTR DS:[ESI]\n");
			break;
		case 0xA5:
			// MOVS/W/D/Q Yv, Xv
			printf("MOVS DWORD PTR ES:[EDI], DWORD PTR DS:[ESI]\n");
			break;
		case 0xA6:
			// CMPS/B Xb, Yb
			printf("CMPS BYTE PTR DS:[ESI], BYTE PTR ES:[EDI]\n");
			break;
		case 0xA7:
			// CMPS/W/D Xv, Yv
			printf("CMPS DWORD PTR DS:[ESI], DWORD PTR ES:[EDI]\n");
			break;
		case 0xA8:
			printf("TEST AL, Ib\n");
			break;
		case 0xA9:
			printf("TEST EAX, Id\n");
			break;
		case 0xAA:
			// STOS/B Yb, AL
			printf("STOS BYTE PTR ES:[EDI]\n");
			break;
		case 0xAB:
			// STOS/W/D/Q Yv, rAX
			printf("STOS DWORD PTR ES:[EDI]\n");
			break;
		case 0xAC:
			// LODS/B AL, Xb
			printf("LODS BYTE PTR DS:[ESI]\n");
			break;
		case 0xAD:
			// LODS/W/D/Q rAX, Xv
			printf("LODS DWORD PTR DS:[ESI]\n");
			break;
		case 0xAE:
			// SCAS/B AL, Yb
			printf("SCAS BYTE PTR ES:[EDI]\n");
			break;
		case 0xAF:
			// SCAS/W/D/Q rAX, Xv
			printf("SCAS DWORD PTR ES:[EDI]\n");
			break;

		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
			// MOV Rb, Ib
			printf("MOV %s, Ib\n", r8[opcode - 0xB0]);
			break;
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
			// MOV ERX, Id
			printf("MOV %s, Id\n", r32[opcode - 0xB8]);
			break;

		case 0xC0:
			// Shift Grp2 Eb, Ib
			getGrpInstruct(1, 2);
			printf("%s, Ib\n", buf);
			break;
		case 0xC1:
			// Shift Grp2 Ev, Ib
			getGrpInstruct(4, 2);
			printf("%s, Ib\n", buf);
			break;
		case 0xC2:
			// RETN Iw
			printf("RETN Iw\n");
			break;
		case 0xC3:
			// RETN
			printf("RETN\n");
			break;
		case 0xC4:
			// LES Gz, Mp VEX + 2byte
			printf("LES Gz, Mp(未解析)\n");
			break;
		case 0xC5:
			// LDS Gz, Mp VEX + 1byte
			printf("LDS Gz, Mp(未解析)\n");
			break;
		case 0xC6:
			// Grp11 - MOV Eb, Ib
			getGrpInstruct(1, 11);
			printf("%s, Ib\n", buf);
			break;
		case 0xC7:
			// Grp11 - MOV Ev, Iz
			getGrpInstruct(4, 11);
			printf("%s, Id\n", buf);
			break;
		case 0xC8:
			printf("ENTER Iw, Ib\n");
			break;
		case 0xC9:
			printf("LEAVE\n");
			break;
		case 0xCA:
			printf("RETF Iw\n");
			break;
		case 0xCB:
			printf("RETF\n");
			break;
		case 0xCC:
			printf("INT3\n");
			break;
		case 0xCD:
			printf("INT Ib\n");
			break;
		case 0xCE:
			printf("INTO\n");
			break;
		case 0xCF:
			// IRET/D/Q
			printf("IRETD\n");
			break;

		case 0xD0:
			// Shift Grp2 Eb, 1
			getGrpInstruct(1, 2);
			printf("%s, 1\n", buf);
			break;
		case 0xD1:
			// Shift Grp2 Ev, 1
			getGrpInstruct(4, 2);
			printf("%s, 1\n", buf);
			break;
		case 0xD2:
			// Shift Grp2 Eb, CL
			getGrpInstruct(1, 2);
			printf("%s, CL\n", buf);
			break;
		case 0xD3:
			// Shift Grp2 Ev, CL
			getGrpInstruct(4, 2);
			printf("%s, CL\n", buf);
			break;
		case 0xD4:
			// AAM Ib
			printf("AAM Ib\n");
			break;
		case 0xD5:
			// AAD Ib
			printf("AAD Ib\n");
			break;
		case 0xD6:
			printf("???\n");
			break;
		case 0xD7:
			// XLAT/XLATB
			printf("XLAT BYTE PTR DS:[EBX + AL]\n");
			break;
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
			printf("ESC(Escape to coprocessor instruction set)\n");
			break;

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
			printf("JECXZ Ib\n");
			break;
		case 0xE4:
			printf("IN AL, Ib\n");
			break;
		case 0xE5:
			printf("IN EAX, Ib\n");
			break;
		case 0xE6:
			printf("OUT Ib, AL\n");
			break;
		case 0xE7:
			printf("OUT Ib, EAX\n");
			break;
		case 0xE8:
			// CALL Jz
			printf("CALL Id\n");
			break;
		case 0xE9:
			// JMP near Jz
			printf("JMP Id\n");
			break;
		case 0xEA:
			// JMP far Ap
			printf("JMP FAR Iw:Id\n");
			break;
		case 0xEB:
			// JMP short Jb
			printf("JMP SHORT Ib\n");
			break;
		case 0xEC:
			printf("IN AL, DX\n");
			break;
		case 0xED:
			printf("IN EAX, DX\n");
			break;
		case 0xEE:
			printf("OUT AL, DX\n");
			break;
		case 0xEF:
			printf("OUT EAX, DX\n");
			break;

		case 0xF0:
			// LOCK(Prefix)
			disposePrefix("LOCK");
			break;
		case 0xF1:
			printf("???\n");
			break;
		case 0xF2:
			// REPNE(Prefix)
			disposePrefix("REPNE");
			break;
		case 0xF3:
			// REP/REPE(Prefix)
			disposePrefix("REP");
			break;
		case 0xF4:
			printf("HLT\n");
			break;
		case 0xF5:
			printf("CMC\n");
			break;
		case 0xF6:
			// Unary Grp3 Eb
			getGrpInstruct(1, 3);
			printf("%s\n", buf);
			break;
		case 0xF7:
			// Unary Grp3 Ev
			getGrpInstruct(1, 3);
			printf("%s\n", buf);
			break;
		case 0xF8:
			printf("CLC\n");
			break;
		case 0xF9:
			printf("STC\n");
			break;
		case 0xFA:
			printf("CLI\n");
			break;
		case 0xFB:
			printf("STI\n");
			break;
		case 0xFC:
			printf("CLD\n");
			break;
		case 0xFD:
			printf("STD\n");
			break;
		case 0xFE:
			// INC/DEC Grp4
			printf("INC/DEC Grp4(未解析)\n");
			break;
		case 0xFF:
			// INC/DEC Grp5
			printf("INC/DEC Grp5(未解析)\n");
			break;
		}
	}
	return 0;
}