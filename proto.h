#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define b(x)  ((uint8_t)b_(0 ## x ## uL))
#define b_(x) ((x      &  1)|(x >>  2 &  2)|(x >>  4 &  4)|(x >>  6 & 8)| \
			   (x >> 8 & 16)|(x >> 10 & 32)|(x >> 12 & 64)|(x >> 14 & 128))

typedef struct
{
	uint8_t data;
	uint8_t flag;
} byte_t;

typedef struct
{
	byte_t high;
	byte_t low;
} halfword_t;

typedef struct
{
	halfword_t high;
	halfword_t low;
} word_t;

typedef struct
{
	halfword_t pointer_link;
	halfword_t pointer_value;
} number_format_t;
typedef number_format_t nf_t;

typedef struct
{
	halfword_t consecutive_storage_link;
	halfword_t free_list_link;
} available_space_format_t;
typedef available_space_format_t asf_t;

typedef struct
{
	byte_t zero;
	byte_t bounds;
	halfword_t start_page;
} base_pointer_t;

typedef size_t raw_address_t;

#define PR_SIZE 14
#define BR_SIZE 6
#define STATUS_INDICATORS_SIZE 6

#define NUM_BYTES (1 << 24)

#define IP  0
#define OSP 13
#define ASP 14

#define CONDITIONAL_SUBTRACT 0

typedef struct
{
	number_format_t pr[PR_SIZE];
	base_pointer_t br[BR_SIZE];
	asf_t pr_14;
	bool status_indicators[STATUS_INDICATORS_SIZE];
} cpu_t;

typedef struct
{
	bool    is_long;
	uint8_t pointer_register_index;
	bool    pre_push;
	bool    post_pop;
	bool    indirect;
	bool    last;
	byte_t  m[2];
} operand_t;

typedef struct
{
	bool hit_conditional_subtract;
	bool conditional_subtract_result;
	bool changed_IP;
	halfword_t new_IP;
} operand_return_t;

typedef enum
{
	INVALID_OPS,
	ZERO_OPS,
	ONE_OPS,
	TWO_OPS,
	MANY_OPS
} num_operands_t;


typedef union
{
	void (*zero_args)(cpu_t *);
	void (*one_args)(size_t operand_reg, cpu_t *);
	void (*two_args)(size_t operand_reg_1, size_t operand_reg_2, cpu_t *);
} opcode_impl_t;

typedef struct
{
	num_operands_t num_operands;
	opcode_impl_t opcode_impl;
} operand_table_t;

#define FLAGLESS (0 << 8)
#define FLAGED   (1 << 8)

typedef enum
{
	ABS_SHORT  = b(10010100),
	ABS_LONG
} flaged_opcode_t;

typedef enum
{
	ZERO_BYTE,
	ZERO_HALFWORD,
	ZERO_WORD,
	ZERO_DOUBLEWORD,
	ONE_BYTE,
	ONE_HALFWORD,
	ONE_WORD,
	ONE_DOUBLEWORD,
	COUNT_BYTE,
	COUNT_HALFWORD,
	COUNT_WORD,
	COUNT_DOUBLEWORD,
	BIT_BYTE,
	BIT_HALFWORD,
	BIT_WORD,
	BIT_DOUBLEWORD,
	AND_BYTE,
	AND_HALFWORD,
	AND_WORD,
	AND_DOUBLEWORD,
	OR_BYTE,
	OR_HALFWORD,
	OR_WORD,
	OR_DOUBLEWORD,
	XOR_BYTE,
	XOR_HALFWORD,
	XOR_WORD,
	XOR_DOUBLEWORD,
	EQV_BYTE,
	EQV_HALFWORD,
	EQV_WORD,
	EQV_DOUBLEWORD,
	SLUFF_BYTE,
	SLUFF_HALFWORD,
	SLUFF_WORD,
	SLUFF_DOUBLEWORD,
	DUP_BYTE,
	DUP_HALFWORD,
	DUP_WORD,
	DUP_DOUBLEWORD,
	XCH_BYTE,
	XCH_HALFWORD,
	XCH_WORD,
	XCH_DOUBLEWORD,
	CPRL_BYTE,
	CPRL_HALFWORD,
	CPRL_WORD,
	CPRL_DOUBLEWORD,
	NOT_BYTE,
	NOT_HALFWORD,
	NOT_WORD,
	NOT_DOUBLEWORD,
	ACTP_BYTE,
	ACTP_HALFWORD,
	ACTP_WORD,
	ACTP_DOUBLEWORD,
	INRT,
	LTR = b(00111010),
	STR,
	SVC,
	SVR,
	WHO = b(00111111),
	PUSH_BYTE,
	PUSH_HALFWORD,
	PUSH_WORD,
	PUSH_DOUBLEWORD,
	LD_BYTE,
	LD_HALFWORD,
	LD_WORD,
	LD_DOUBLEWORD,
	POP_BYTE,
	POP_HALFWORD,
	POP_WORD,
	POP_DOUBLEWORD,
	ST_BYTE,
	ST_HALFWORD,
	ST_WORD,
	ST_DOUBLEWORD,
	SET_BYTE,
	SET_HALFWORD,
	SET_WORD,
	SET_DOUBLEWORD,
	RESET_BYTE,
	RESET_HALFWORD,
	RESET_WORD,
	RESET_DOUBLEWORD,
	TEST_BYTE,
	TEST_HALFWORD,
	TEST_WORD,
	TEST_DOUBLEWORD,
	TESTM_BYTE,
	TESTM_HALFWORD,
	TESTM_WORD,
	TESTM_DOUBLEWORD,
	LS_BYTE,
	LS_HALFWORD,
	LS_WORD,
	RS_BYTE = b(01100100),
	RS_HALFWORD,
	RS_WORD,
	SLEEP = b(01111100),
	INCK,
	SIM
} flagless_opcode_t;

raw_address_t get_address_common(number_format_t p, const cpu_t *cpu);

raw_address_t get_address_from_pointer(number_format_t p, const cpu_t *cpu);

raw_address_t get_address_from_link(number_format_t p, const cpu_t *cpu);

number_format_t get_pointer_register_from_memory(raw_address_t addr);

byte_t get_byte_from_memory(raw_address_t addr);

halfword_t get_halfword_from_memory(raw_address_t addr);

word_t get_word_from_memory(raw_address_t addr);

void put_byte_into_memory(byte_t arg, raw_address_t addr);

void put_halfword_into_memory(halfword_t arg, raw_address_t addr);

void put_word_into_memory(word_t arg, raw_address_t addr);

uint8_t get_data_from_byte(byte_t byte);

bool get_flag_from_byte(byte_t byte);

void set_flag_byte(byte_t *byte);

void clear_flag_byte(byte_t *byte);

bool get_flag_from_halfword(halfword_t halfword, size_t byte_num);

void set_flag_halfword(halfword_t *halfword, size_t byte_num);

void clear_flag_halfword(halfword_t *halfword, size_t byte_num);

bool get_flag_from_word(word_t word, size_t byte_num);

void set_flag_word(word_t *word, size_t byte_num);

void clear_flag_word(word_t *word, size_t byte_num);

uint16_t get_data_from_halfword(halfword_t halfword);

uint32_t get_data_from_word(word_t word);

byte_t put_data_into_byte(uint8_t data);

halfword_t put_data_into_halfword(uint16_t data);

word_t put_data_into_word(uint32_t data);

void or_byte_flags(byte_t from, byte_t *to);

void or_halfword_flags(halfword_t from, halfword_t *to);

void or_word_flags(word_t from, word_t *to);

void copy_byte_flags(byte_t from, byte_t *to);

void copy_halfword_flags(halfword_t from, halfword_t *to);

void copy_word_flags(word_t from, word_t *to);

void set_data_in_byte(uint8_t data, byte_t *to);

void set_data_in_halfword(uint16_t data, halfword_t *to);

void set_data_in_word(uint32_t data, word_t *to);

byte_t peek_operand_byte(const cpu_t *cpu);

halfword_t peek_operand_halfword(const cpu_t *cpu);

byte_t pop_operand_byte(cpu_t *cpu);

halfword_t pop_operand_halfword(cpu_t *cpu);

word_t pop_operand_word(cpu_t *cpu);

void push_operand_byte(byte_t arg, cpu_t *cpu);

void push_operand_halfword(halfword_t arg, cpu_t *cpu);

void push_operand_word(word_t arg, cpu_t *cpu);

void abs_short(cpu_t *cpu);

void abs_long(cpu_t *cpu);

void add_short(cpu_t *cpu);

void add_long(cpu_t *cpu);

void one_byte(cpu_t *cpu);

void sluff_byte(cpu_t *cpu);

void sluff_halfword(cpu_t *cpu);

void sluff_word(cpu_t *cpu);

void dup_byte(cpu_t *cpu);

void dup_halfword(cpu_t *cpu);

void dup_word(cpu_t *cpu);

void xch_byte(cpu_t *cpu);

void xch_halfword(cpu_t *cpu);

void xch_word(cpu_t *cpu);

void hcf(byte_t opcode, cpu_t *cpu);

size_t decode_byte_t(byte_t byte);

operand_t decode_operand(raw_address_t operand_address, const cpu_t *cpu);

void handle_preslash(operand_t operand, cpu_t *cpu);

operand_return_t handle_operand(operand_t operand, cpu_t *cpu);

void instruction_fetch_loop(cpu_t *cpu);

uint16_t increment_ip(uint16_t increment, const cpu_t *cpu);

void cpu_ctor(cpu_t *cpu);

void core_memory_ctor(void);

void core_memory_dtor(void);

void load_object_file(const char *objfile);

extern const
operand_table_t opcodes[512];
