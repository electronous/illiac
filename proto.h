#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define b(x)  ((uint8_t)b_(0 ## x ## uL))
#define b_(x) ((x      &  1)|(x >>  2 &  2)|(x >>  4 &  4)|(x >>  6 & 8)| \
			   (x >> 8 & 16)|(x >> 10 & 32)|(x >> 12 & 64)|(x >> 14 & 128))

typedef struct
{
	uint8_t data;
	bool    flag;
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
#define NUM_BYTES (1 << 24)

typedef struct
{
	number_format_t pr[PR_SIZE];
	base_pointer_t br[BR_SIZE];
	asf_t pr_14;
} cpu_t;

raw_address_t get_address_common(const number_format_t *p, const cpu_t *cpu);

raw_address_t get_address_from_pointer(const number_format_t *p, const cpu_t *cpu);

raw_address_t get_address_from_link(const number_format_t *p, const cpu_t *cpu);

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

void or_byte_flags(const byte_t *from, byte_t *to);

void or_halfword_flags(const halfword_t *from, halfword_t *to);

void or_word_flags(const word_t *from, word_t *to);

void copy_byte_flags(const byte_t *from, byte_t *to);

void copy_halfword_flags(const halfword_t *from, halfword_t *to);

void copy_word_flags(const word_t *from, word_t *to);

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

void dup_byte(cpu_t *cpu);

void dup_halfword(cpu_t *cpu);

void dup_word(cpu_t *cpu);

void sluff_byte(cpu_t *cpu);

void sluff_halfword(cpu_t *cpu);

void sluff_word(cpu_t *cpu);

void xch_byte(cpu_t *cpu);

void xch_halfword(cpu_t *cpu);

void xch_word(cpu_t *cpu);

void hcf(cpu_t *cpu, byte_t opcode);

uint16_t increment_ip(cpu_t *cpu, uint16_t increment);

void execute(byte_t opcode, cpu_t *cpu);

void cpu_ctor(cpu_t *cpu);
