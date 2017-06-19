#help_index "Compiler/Internal"
//Intermediate compiler codes
#define IS_0_ARG	0
#define IS_1_ARG	1
#define IS_2_ARG	2
#define IS_V_ARG	3 //Variable Arg Cnt

#define IST_NULL	0
#define IST_DEREF	1
#define IST_ASSIGN	2
#define IST_CMP		3
	
class CIntermediateStruct
{
  U8 arg_cnt,res_cnt,type;
  Bool fpop,not_const,pad[3];
  U8 *name;
};

#define IC_END			0x00
#define IC_NOP1			0x01
#define IC_END_EXP		0x02
#define IC_NOP2			0x03
#define IC_LABEL		0x04
#define IC_CALL_START		0x05
#define IC_CALL_END		0x06
#define IC_CALL_END2		0x07
#define IC_RETURN_VAL		0x08
#define IC_RETURN_VAL2		0x09
#define IC_IMM_I64		0x0A
#define IC_IMM_F64		0x0B
#define IC_STR_CONST		0x0C
#define IC_ABS_ADDR		0x0D
#define IC_ADDR_IMPORT		0x0E
#define IC_HEAP_GLBL		0x0F
#define IC_SIZEOF		0x10
#define IC_TYPE			0x11
#define IC_GET_LABEL		0x12
#define IC_RBP			0x13
#define IC_REG			0x14
#define IC_FS			0x15
#define IC_MOV_FS		0x16
#define IC_GS			0x17
#define IC_MOV_GS		0x18

#define IC_LEA			0x19
#define IC_MOV			0x1A

#define IC_TO_I64		0x1B
#define IC_TO_F64		0x1C
#define IC_TO_BOOL		0x1D
#define IC_TOUPPER		0x1E
#define IC_HOLYC_TYPECAST	0x1F

#define IC_ADDR			0x20
#define IC_COM			0x21
#define IC_NOT			0x22
#define IC_UNARY_MINUS		0x23

#define IC_DEREF		0x24
#define IC_DEREF_PP		0x25
#define IC_DEREF_MM		0x26
#define IC__PP			0x27
#define IC__MM			0x28
#define IC_PP_			0x29
#define IC_MM_			0x2A

#define IC_SHL			0x2B
#define IC_SHR			0x2C
#define IC_SHL_CONST		0x2D
#define IC_SHR_CONST		0x2E
#define IC_POWER		0x2F

#define IC_MUL			0x30
#define IC_DIV			0x31
#define IC_MOD			0x32

#define IC_AND			0x33

#define IC_OR			0x34
#define IC_XOR			0x35

#define IC_ADD			0x36
#define IC_SUB			0x37
#define IC_ADD_CONST		0x38
#define IC_SUB_CONST		0x39

#define IC_EQU_EQU		0x3A
#define IC_NOT_EQU		0x3B
#define IC_LESS			0x3C
#define IC_GREATER_EQU		0x3D
#define IC_GREATER		0x3E
#define IC_LESS_EQU		0x3F
#define IC_PUSH_CMP		0x40

#define IC_AND_AND		0x41

#define IC_OR_OR		0x42
#define IC_XOR_XOR		0x43

#define IC_ASSIGN		0x44
#define IC_ASSIGN_PP		0x45
#define IC_ASSIGN_MM		0x46

#define IC_SHL_EQU		0x47
#define IC_SHR_EQU		0x48
#define IC_MUL_EQU		0x49
#define IC_DIV_EQU		0x4A
#define IC_MOD_EQU		0x4B
#define IC_AND_EQU		0x4C
#define IC_OR_EQU		0x4D
#define IC_XOR_EQU		0x4E
#define IC_ADD_EQU		0x4F
#define IC_SUB_EQU		0x50

#define IC_JMP			0x51
#define IC_SUB_CALL		0x52
#define IC_SWITCH		0x53
#define IC_NOBOUND_SWITCH		0x54

#define IC_ADD_RSP		0x55
#define IC_ADD_RSP1		0x56
#define IC_ENTER		0x57
#define IC_PUSH_REGS		0x58
#define IC_POP_REGS		0x59
#define IC_LEAVE		0x5A
#define IC_RET			0x5B

#define IC_CALL			0x5C
#define IC_CALL_INDIRECT	0x5D
#define IC_CALL_INDIRECT2	0x5E
#define IC_CALL_IMPORT		0x5F
#define IC_CALL_EXTERN		0x60

#define IC_ASM			0x61
#define IC_PUSH			0x62
#define IC_POP			0x63

#define IC_CLFLUSH		0x64
#define IC_INVLPG		0x65

#define IC_IN_U8		0x66
#define IC_IN_U16		0x67
#define IC_IN_U32		0x68
#define IC_OUT_U8		0x69
#define IC_OUT_U16		0x6A
#define IC_OUT_U32		0x6B

#define IC_GET_RFLAGS		0x6C
#define IC_CARRY		0x6D
#define IC_SET_RFLAGS		0x6E
#define IC_GET_RAX		0x6F
#define IC_SET_RAX		0x70
#define IC_GET_RBP		0x71
#define IC_SET_RBP		0x72
#define IC_GET_RSP		0x73
#define IC_SET_RSP		0x74
#define IC_RIP			0x75

#define IC_RDTSC		0x76

#define IC_BT			0x77
#define IC_BTS			0x78
#define IC_BTR			0x79
#define IC_BTC			0x7A
#define IC_LBTS			0x7B
#define IC_LBTR			0x7C
#define IC_LBTC			0x7D
#define IC_BSF			0x7E
#define IC_BSR			0x7F

#define IC_QUE_INIT		0x80
#define IC_QUE_INS		0x81
#define IC_QUE_INS_REV		0x82
#define IC_QUE_REM		0x83

#define IC_STRLEN		0x84

#define IC_BR_ZERO		0x85
#define IC_BR_NOT_ZERO		0x86
#define IC_BR_CARRY		0x87
#define IC_BR_NOT_CARRY		0x88

#define IC_BR_EQU_EQU		0x89
#define IC_BR_NOT_EQU		0x8A
#define IC_BR_LESS		0x8B
#define IC_BR_GREATER_EQU	0x8C
#define IC_BR_GREATER		0x8D
#define IC_BR_LESS_EQU		0x8E

#define IC_BR_EQU_EQU2		0x8F
#define IC_BR_NOT_EQU2		0x90
#define IC_BR_LESS2		0x91
#define IC_BR_GREATER_EQU2	0x92
#define IC_BR_GREATER2		0x93
#define IC_BR_LESS_EQU2		0x94

#define IC_BR_AND_ZERO		0x95
#define IC_BR_AND_NOT_ZERO	0x96
#define IC_BR_MM_ZERO		0x97
#define IC_BR_MM_NOT_ZERO	0x98
#define IC_BR_AND_AND_ZERO	0x99
#define IC_BR_AND_AND_NOT_ZERO	0x9A
#define IC_BR_OR_OR_ZERO	0x9B
#define IC_BR_OR_OR_NOT_ZERO	0x9C

#define IC_BR_BT		0x9D
#define IC_BR_BTS		0x9E
#define IC_BR_BTR		0x9F
#define IC_BR_BTC		0xA0
#define IC_BR_NOT_BT		0xA1
#define IC_BR_NOT_BTS		0xA2
#define IC_BR_NOT_BTR		0xA3
#define IC_BR_NOT_BTC		0xA4

#define IC_SWAP_U8		0xA5
#define IC_SWAP_U16		0xA6
#define IC_SWAP_U32		0xA7
#define IC_SWAP_I64		0xA8

#define IC_ABS_I64		0xA9
#define IC_SIGN_I64		0xAA
#define IC_MIN_I64		0xAB
#define IC_MIN_U64		0xAC
#define IC_MAX_I64		0xAD
#define IC_MAX_U64		0xAE
#define IC_MOD_U64		0xAF
#define IC_SQR_I64		0xB0
#define IC_SQR_U64		0xB1
#define IC_SQR			0xB2
#define IC_ABS			0xB3
#define IC_SQRT			0xB4
#define IC_SIN			0xB5
#define IC_COS			0xB6
#define IC_TAN			0xB7
#define IC_ATAN			0xB8
#define IC_ICS_NUM		0xB9

#define KW_INCLUDE	0
#define KW_DEFINE	1
#define KW_UNION	2
#define KW_CATCH	3
#define KW_CLASS	4
#define KW_TRY		5
#define KW_IF		6
#define KW_ELSE		7
#define KW_FOR		8
#define KW_WHILE	9
#define KW_EXTERN	10
#define KW__EXTERN	11
#define KW_RETURN	12
#define KW_SIZEOF	13
#define KW__INTERN	14
#define KW_DO		15
#define KW_ASM		16
#define KW_GOTO		17
#define KW_EXE		18
#define KW_BREAK	19
#define KW_SWITCH	20
#define KW_START	21
#define KW_END		22
#define KW_CASE		23
#define KW_DFT		24
#define KW_PUBLIC	25
#define KW_OFFSET	26
#define KW_IMPORT	27
#define KW__IMPORT	28
#define KW_IFDEF	29
#define KW_IFNDEF	30
#define KW_IFAOT	31
#define KW_IFJIT	32
#define KW_ENDIF	33
#define KW_ASSERT	34
#define KW_REG		35
#define KW_NOREG	36
#define KW_LASTCLASS	37
#define KW_NO_WARN	38
#define KW_HELP_INDEX	39
#define KW_HELP_FILE	40
#define KW_STATIC	41
#define KW_LOCK		42
#define KW_DEFINED	43
#define KW_INTERRUPT	44
#define KW_HASERRCODE	45
#define KW_ARGPOP	46
#define KW_NOARGPOP	47

#define AKW_ALIGN	64
#define AKW_ORG		65
#define AKW_I0		66
#define AKW_I8		67
#define AKW_I16		68
#define AKW_I32		69
#define AKW_I64		70
#define AKW_U0		71
#define AKW_U8		72
#define AKW_U16		73
#define AKW_U32		74
#define AKW_U64		75
#define AKW_F64		76
#define AKW_DU8		77
#define AKW_DU16	78
#define AKW_DU32	79
#define AKW_DU64	80
#define AKW_DUP		81
#define AKW_USE16	82
#define AKW_USE32	83
#define AKW_USE64	84
#define AKW_IMPORT	85
#define AKW_LIST	86
#define AKW_NOLIST	87
#define AKW_BINFILE	88
#define KW_KWS_NUM	89

#define CMP_TEMPLATE_INC	0x00
#define CMP_TEMPLATE_DEC	0x01
#define CMP_TEMPLATE_MOD	0x02
#define CMP_TEMPLATE_LESS	0x03
#define CMP_TEMPLATE_GREATER	0x04
#define CMP_TEMPLATE_LESS_EQU	0x05
#define CMP_TEMPLATE_GREATER_EQU 0x06
#define CMP_TEMPLATE_STRLEN	0x07
#define CMP_TEMPLATE_RDTSC	0x08
#define CMP_TEMPLATE_SIGN_I64	0x09
#define CMP_TEMPLATE_FSTP	0x0A
#define CMP_TEMPLATE_FLD	0x0B
#define CMP_TEMPLATE_SQR	0x0C
#define CMP_TEMPLATE_ABS	0x0D
#define CMP_TEMPLATE_SQRT	0x0E
#define CMP_TEMPLATE_SIN	0x0F
#define CMP_TEMPLATE_COS	0x10
#define CMP_TEMPLATE_TAN	0x11
#define CMP_TEMPLATE_ATAN	0x12

#define ASSOCF_LEFT	1
#define ASSOCF_RIGHT	2
#define ASSOC_MASK	3

#define PREC_NULL		0x00
#define PREC_TERM		0x04
#define PREC_UNARY_POST		0x08
#define PREC_UNARY_PRE		0x0C
#define PREC_EXP		0x10
#define PREC_MUL		0x14
#define PREC_AND		0x18
#define PREC_XOR		0x1C
#define PREC_OR			0x20
#define PREC_ADD		0x24
#define PREC_CMP		0x28
#define PREC_CMP2		0x2C
#define PREC_AND_AND		0x30
#define PREC_XOR_XOR		0x34
#define PREC_OR_OR		0x38
#define PREC_ASSIGN		0x3C

#define PREC_MAX		0x40

//Function/static/public flags
#define FSF_PUBLIC		0x01
#define FSF_ASM			0x02
#define FSF_STATIC		0x04
#define FSF__			0x08
#define FSF_INTERRUPT		(1<<Ff_INTERRUPT)
#define FSF_HASERRCODE		(1<<Ff_HASERRCODE)
#define FSF_ARGPOP		(1<<Ff_ARGPOP)
#define FSF_NOARGPOP		(1<<Ff_NOARGPOP)
#define FSG_FUN_FLAGS1 (FSF_INTERRUPT|FSF_HASERRCODE|FSF_ARGPOP|FSF_NOARGPOP)
#define FSG_FUN_FLAGS2 (FSG_FUN_FLAGS1|FSF_PUBLIC)

//Byte 0
#define PRS0_NULL		0x000000
#define PRS0__EXTERN		0x000001
#define PRS0__INTERN		0x000002
#define PRS0__IMPORT		0x000003
#define PRS0_EXTERN		0x000004
#define PRS0_IMPORT		0x000005
#define PRS0_TYPECAST		0x000006

//Byte 1
#define PRS1B_NULL		0
#define PRS1_NULL		0x000000
#define PRS1B_LOCAL_VAR		1
#define PRS1_LOCAL_VAR		0x000100
#define PRS1B_FUN_ARG		2
#define PRS1_FUN_ARG		0x000200
#define PRS1B_CLASS		3
#define PRS1_CLASS		0x000300
#define PRS1B_STATIC_LOCAL_VAR	4
#define PRS1_STATIC_LOCAL_VAR	0x000400
#define PRS1B__EXTERN_IMPORT	5
#define PRS1_NOT_REALLY__EXTERN	0x000500

#define PRSF_UNION		0x010000

class COptReg
{
  I64	offset;
  CMemberLst *m;
};

#define CMPF_ASM_BLK		1
#define CMPF_ONE_ASM_INS	2
#define CMPF_LEX_FIRST		4
#define CMPF_PRS_SEMICOLON	8

#help_index ""
