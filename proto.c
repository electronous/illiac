#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "proto.h"

static
byte_t *core_memory;

const
operand_table_t opcodes[NUM_OPCODES] = {
	[ASSIGN_BYTE     | FLAGGED]  = { .num_operands = TWO_OPS, .opcode_impl.two_args = assign_byte },
	[ASSIGN_HALFWORD | FLAGGED]  = { .num_operands = TWO_OPS, .opcode_impl.two_args = assign_halfword },
	[ASSIGN_WORD     | FLAGGED]  = { .num_operands = TWO_OPS, .opcode_impl.two_args = assign_word },

	[NOP     | FLAGGED] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = nop },
	[SPECIFY | FLAGGED] = { .num_operands = MANY_OPS, .opcode_impl.zero_args = nop },

	[IF            | FLAGGED]  = { .num_operands = BIT_OPS,  .opcode_impl.bit_args  = branch },
	[IF_NOT        | FLAGGED]  = { .num_operands = BIT_OPS,  .opcode_impl.bit_args  = branch_not },

	[ABS_SHORT     | FLAGGED]  = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = abs_short },
	[ABS_LONG      | FLAGGED]  = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = abs_long },

	[ONE_BYTE      | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = one_byte },

	[SLUFF_BYTE    | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = sluff_byte },
	[SLUFF_HALFWORD| FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = sluff_halfword },
	[SLUFF_WORD    | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = sluff_word },

	[DUP_BYTE      | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = dup_byte },
	[DUP_HALFWORD  | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = dup_halfword },
	[DUP_WORD      | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = dup_word },

	[XCH_BYTE      | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = xch_byte },
	[XCH_HALFWORD  | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = xch_halfword },
	[XCH_WORD      | FLAGLESS] = { .num_operands = ZERO_OPS, .opcode_impl.zero_args = xch_word }
};

raw_address_t get_address_common(number_format_t p, const cpu_t *cpu)
{
	uint8_t third_flag  = (uint8_t)((get_flag_from_byte(p.pointer_link.low)   << 2u) & 0xFFu);
	uint8_t second_flag = (uint8_t)((get_flag_from_byte(p.pointer_value.high) << 1u) & 0xFFu);
	uint8_t first_flag  = (uint8_t)((get_flag_from_byte(p.pointer_value.low)  << 0u) & 0xFFu);

	br_index_t br_num = (br_index_t)(first_flag | second_flag | third_flag);

	assert(br_num < BR_SIZE);

	return (raw_address_t)((get_data_from_byte(cpu->br[br_num].start_page.high) << 16) |
						   (get_data_from_byte(cpu->br[br_num].start_page.low)  << 8));
}

raw_address_t get_address_from_pointer(number_format_t p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (raw_address_t) get_data_from_byte(p.pointer_value.low);
	ret += (raw_address_t)(get_data_from_byte(p.pointer_value.high) << 8);

	return ret;
}

raw_address_t get_address_from_link(number_format_t p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (raw_address_t) get_data_from_byte(p.pointer_link.low);
	ret += (raw_address_t)(get_data_from_byte(p.pointer_link.high) << 8);

	return ret;
}

number_format_t follow_link(number_format_t p, const cpu_t *cpu)
{
	return get_pointer_register_from_memory(get_address_from_link(p, cpu));
}

number_format_t get_pointer_register_from_memory(raw_address_t addr)
{
	assert(addr + HALFWORD_SIZE > addr);

	return (number_format_t){
		.pointer_link  = get_halfword_from_memory(addr),
		.pointer_value = get_halfword_from_memory(addr + HALFWORD_SIZE)
	};
}

byte_t get_byte_from_memory(raw_address_t addr)
{
	assert(addr < (raw_address_t)NUM_BYTES);
	return core_memory[addr];
}

halfword_t get_halfword_from_memory(raw_address_t addr)
{
	assert(addr + BYTE_SIZE > addr);

	return (halfword_t){
		.high = get_byte_from_memory(addr),
		.low  = get_byte_from_memory(addr + BYTE_SIZE)
	};
}

word_t get_word_from_memory(raw_address_t addr)
{
	assert(addr + HALFWORD_SIZE > addr);

	return (word_t){
		.high = get_halfword_from_memory(addr),
		.low  = get_halfword_from_memory(addr + HALFWORD_SIZE),
	};
}

void put_byte_into_memory(byte_t arg, raw_address_t addr)
{
	assert(addr < (raw_address_t)NUM_BYTES);
	core_memory[addr] = arg;
}

void put_halfword_into_memory(halfword_t arg, raw_address_t addr)
{
	assert(addr + BYTE_SIZE > addr);

	put_byte_into_memory(arg.high, addr);
	put_byte_into_memory(arg.low,  addr + BYTE_SIZE);
}

void put_word_into_memory(word_t arg, raw_address_t addr)
{
	assert(addr + HALFWORD_SIZE > addr);

	put_halfword_into_memory(arg.high, addr);
	put_halfword_into_memory(arg.low,  addr + HALFWORD_SIZE);
}

uint8_t get_data_from_byte(byte_t byte)
{
	return byte.data;
}

uint16_t get_data_from_halfword(halfword_t halfword)
{
	uint8_t high = get_data_from_byte(halfword.high);
	uint8_t low  = get_data_from_byte(halfword.low);
	return (uint16_t)(high << 8 | low);
}

uint32_t get_data_from_word(word_t word)
{
	uint16_t high = get_data_from_halfword(word.high);
	uint16_t low  = get_data_from_halfword(word.low);
	return (uint32_t)(high << 16 | low);
}

byte_t put_data_into_byte(uint8_t data)
{
	return (byte_t){
		.data = data,
		.flag = 0
	};
}

halfword_t put_data_into_halfword(uint16_t data)
{
	return (halfword_t){
		.high = put_data_into_byte((uint8_t)((data >> 8) & 0xFF)),
		.low  = put_data_into_byte((uint8_t)(data        & 0xFF))
	};
}

word_t put_data_into_word(uint32_t data)
{
	return (word_t){
		.high = put_data_into_halfword((uint16_t)((data >> 16) & 0xFFFF)),
		.low  = put_data_into_halfword((uint16_t)(data         & 0xFFFF))
	};
}

bool get_flag_from_byte(byte_t byte)
{
	return byte.flag;
}

void set_flag_byte(byte_t *byte)
{
	byte->flag = true;
}

void clear_flag_byte(byte_t *byte)
{
	byte->flag = false;
}

bool get_flag_from_halfword(halfword_t halfword, size_t byte_num)
{
	assert(byte_num < HALFWORD_SIZE);

	if (byte_num == 0)
	{
		return get_flag_from_byte(halfword.high);
	}
	else
	{
		return get_flag_from_byte(halfword.low);
	}
}

void set_flag_halfword(halfword_t *halfword, size_t byte_num)
{
	assert(byte_num < HALFWORD_SIZE);

	if (byte_num == 0)
	{
		set_flag_byte(&(halfword->high));
	}
	else
	{
		set_flag_byte(&(halfword->low));
	}
}

void clear_flag_halfword(halfword_t *halfword, size_t byte_num)
{
	assert(byte_num < HALFWORD_SIZE);

	if (byte_num == 0)
	{
		clear_flag_byte(&(halfword->high));
	}
	else
	{
		clear_flag_byte(&(halfword->low));
	}
}

bool get_flag_from_word(word_t word, size_t byte_num)
{
	size_t halfword_num = byte_num % HALFWORD_SIZE;

	assert(byte_num < WORD_SIZE);

	if (byte_num < HALFWORD_SIZE)
	{
		return get_flag_from_halfword(word.high, halfword_num);
	}
	else
	{
		return get_flag_from_halfword(word.low, halfword_num);
	}
}

void set_flag_word(word_t *word, size_t byte_num)
{
	size_t halfword_num = byte_num % HALFWORD_SIZE;

	assert(byte_num < WORD_SIZE);

	if (byte_num < HALFWORD_SIZE)
	{
		set_flag_halfword(&(word->high), halfword_num);
	}
	else
	{
		set_flag_halfword(&(word->low), halfword_num);
	}
}

void clear_flag_word(word_t *word, size_t byte_num)
{
	size_t halfword_num = byte_num % HALFWORD_SIZE;

	assert(byte_num < WORD_SIZE);

	if (byte_num < HALFWORD_SIZE)
	{
		clear_flag_halfword(&(word->high), halfword_num);
	}
	else
	{
		clear_flag_halfword(&(word->low), halfword_num);
	}
}

void or_byte_flags(byte_t from, byte_t *to)
{
	if (get_flag_from_byte(from))
	{
		set_flag_byte(to);
	}
}

void or_halfword_flags(halfword_t from, halfword_t *to)
{
	or_byte_flags(from.high, &(to->high));
	or_byte_flags(from.low,  &(to->low));
}

void or_word_flags(word_t from, word_t *to)
{
	or_halfword_flags(from.high, &(to->high));
	or_halfword_flags(from.low,  &(to->low));
}

void copy_byte_flags(byte_t from, byte_t *to)
{
	if (get_flag_from_byte(from))
	{
		set_flag_byte(to);
	}
	else
	{
		clear_flag_byte(to);
	}
}

void copy_halfword_flags(halfword_t from, halfword_t *to)
{
	copy_byte_flags(from.high, &(to->high));
	copy_byte_flags(from.low,  &(to->low));
}

void copy_word_flags(word_t from, word_t *to)
{
	copy_halfword_flags(from.high, &(to->high));
	copy_halfword_flags(from.low,  &(to->low));
}

void set_data_in_byte(uint8_t data, byte_t *to)
{
	to->data = data;
}

void set_data_in_halfword(uint16_t data, halfword_t *to)
{
	halfword_t halfword = put_data_into_halfword(data);
	set_data_in_byte(get_data_from_byte(halfword.high), &(to->high));
	set_data_in_byte(get_data_from_byte(halfword.low),  &(to->low));
}

void set_data_in_word(uint32_t data, word_t *to)
{
	word_t word = put_data_into_word(data);
	set_data_in_halfword(get_data_from_halfword(word.high), &(to->high));
	set_data_in_halfword(get_data_from_halfword(word.low),  &(to->low));
}

byte_t peek_operand_byte(const cpu_t *cpu)
{
	raw_address_t operand_pointer;
	uint16_t old_addr, new_addr;

	old_addr = get_data_from_halfword(cpu->pr[OSP].pointer_value);

	assert(old_addr >= BYTE_SIZE);

	new_addr = (uint16_t)(old_addr - BYTE_SIZE);

	number_format_t temp_addr = cpu->pr[OSP];
	set_data_in_halfword(new_addr, &(temp_addr.pointer_value));

	operand_pointer = get_address_from_pointer(temp_addr, cpu);
	return get_byte_from_memory(operand_pointer);
}

halfword_t peek_operand_halfword(const cpu_t *cpu)
{
	halfword_t halfword;
	halfword.low  = peek_operand_byte(cpu);
	halfword.high = peek_operand_byte(cpu);
	return halfword;
}

byte_t pop_operand_byte(cpu_t *cpu)
{
	raw_address_t operand_pointer;
	uint16_t old_addr, new_addr;

	old_addr = get_data_from_halfword(cpu->pr[OSP].pointer_value);

	assert(old_addr >= BYTE_SIZE);

	new_addr = (uint16_t)(old_addr - BYTE_SIZE);

	set_data_in_halfword(new_addr, &(cpu->pr[OSP].pointer_value));

	operand_pointer = get_address_from_pointer(cpu->pr[OSP], cpu);
	return get_byte_from_memory(operand_pointer);
}

halfword_t pop_operand_halfword(cpu_t *cpu)
{
	halfword_t halfword;
	halfword.low  = pop_operand_byte(cpu);
	halfword.high = pop_operand_byte(cpu);
	return halfword;
}

word_t pop_operand_word(cpu_t *cpu)
{
	word_t word;
	word.low  = pop_operand_halfword(cpu);
	word.high = pop_operand_halfword(cpu);
	return word;
}

void push_operand_byte(byte_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer;
	uint16_t old_addr, new_addr;

	operand_pointer = get_address_from_pointer(cpu->pr[OSP], cpu);
	old_addr = get_data_from_halfword(cpu->pr[OSP].pointer_value);
	new_addr = (uint16_t)(old_addr + BYTE_SIZE);

	assert(new_addr > old_addr);

	set_data_in_halfword(new_addr, &(cpu->pr[OSP].pointer_value));

	put_byte_into_memory(arg, operand_pointer);
}

void push_operand_halfword(halfword_t arg, cpu_t *cpu)
{
	push_operand_byte(arg.high, cpu);
	push_operand_byte(arg.low,  cpu);
}

void push_operand_word(word_t arg, cpu_t *cpu)
{
	push_operand_halfword(arg.high, cpu);
	push_operand_halfword(arg.low,  cpu);
}

bool assign_byte(operand_t operand_reg_1, operand_t operand_reg_2, cpu_t *cpu)
{
	bool changed_pr0 = false;
	if (operand_reg_1.last || operand_reg_2.last)
	{
		uint16_t source_data;
		if (operand_reg_2.last)
		{
			source_data = get_data_from_halfword(cpu->pr[operand_reg_2.pointer_register_index].pointer_value);
		}
		else
		{
			raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
			source_data = get_data_from_halfword(get_halfword_from_memory(source_addr));
		}

		if (operand_reg_1.last)
		{
			set_data_in_halfword(source_data, &(cpu->pr[operand_reg_1.pointer_register_index].pointer_value));
			if (operand_reg_1.pointer_register_index == IP)
			{
				changed_pr0 = true;
			}
		}
		else
		{
			raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);
			halfword_t temp = get_halfword_from_memory(dest_addr);
			set_data_in_halfword(source_data, &temp);
			put_halfword_into_memory(temp, dest_addr);
		}
	}
	else
	{
		raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
		raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);

		byte_t source_data = get_byte_from_memory(source_addr);
		put_byte_into_memory(source_data, dest_addr);
	}
	return changed_pr0;
}

bool assign_halfword(operand_t operand_reg_1, operand_t operand_reg_2, cpu_t *cpu)
{
	bool changed_pr0 = false;
	if (operand_reg_1.last || operand_reg_2.last)
	{
		uint16_t source_data;
		if (operand_reg_2.last)
		{
			source_data = get_data_from_halfword(cpu->pr[operand_reg_2.pointer_register_index].pointer_link);
		}
		else
		{
			raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
			source_data = get_data_from_halfword(get_halfword_from_memory(source_addr));
		}

		if (operand_reg_1.last)
		{
			set_data_in_halfword(source_data, &(cpu->pr[operand_reg_1.pointer_register_index].pointer_link));
			if (operand_reg_1.pointer_register_index == IP)
			{
				changed_pr0 = true;
			}
		}
		else
		{
			raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);
			halfword_t temp = get_halfword_from_memory(dest_addr);
			set_data_in_halfword(source_data, &temp);
			put_halfword_into_memory(temp, dest_addr);
		}
	}
	else
	{
		raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
		raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);

		halfword_t source_data = get_halfword_from_memory(source_addr);
		put_halfword_into_memory(source_data, dest_addr);
	}
	return changed_pr0;
}

bool assign_word(operand_t operand_reg_1, operand_t operand_reg_2, cpu_t *cpu)
{
	bool changed_pr0 = false;
	if (operand_reg_1.last || operand_reg_2.last)
	{
		word_t source_data;
		if (operand_reg_2.last)
		{
			source_data.high = cpu->pr[operand_reg_2.pointer_register_index].pointer_link;
			source_data.low  = cpu->pr[operand_reg_2.pointer_register_index].pointer_value;
		}
		else
		{
			raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
			source_data = get_word_from_memory(source_addr);
		}

		if (operand_reg_1.last)
		{
			cpu->pr[operand_reg_1.pointer_register_index].pointer_link  = source_data.high;
			cpu->pr[operand_reg_1.pointer_register_index].pointer_value = source_data.low;
			if (operand_reg_1.pointer_register_index == IP)
			{
				changed_pr0 = true;
			}
		}
		else
		{
			raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);
			put_word_into_memory(source_data, dest_addr);
		}
	}
	else
	{
		raw_address_t source_addr = get_address_from_pointer(cpu->pr[operand_reg_2.pointer_register_index], cpu);
		raw_address_t dest_addr = get_address_from_pointer(cpu->pr[operand_reg_1.pointer_register_index], cpu);

		word_t source_data = get_word_from_memory(source_addr);
		put_word_into_memory(source_data, dest_addr);
	}
	return changed_pr0;
}

bool nop(cpu_t *cpu)
{
	return false;
}

bool branch(byte_t byte, operand_t operand, cpu_t *cpu)
{
	bool did_branch = false;
	uint8_t data = get_data_from_byte(byte);
	if (cpu->status_indicators[CONDITIONAL_SUBTRACT] && (data & (1 << CONDITIONAL_SUBTRACT)))
	{
		did_branch = true;
	}
	if (cpu->status_indicators[OVERFLOW] && (data & (1 << OVERFLOW)))
	{
		did_branch = true;
	}
	if (cpu->status_indicators[GREATER_THAN] && (data & (1 << GREATER_THAN)))
	{
		did_branch = true;
	}
	if (cpu->status_indicators[EQUAL] && (data & (1 << EQUAL)))
	{
		did_branch = true;
	}
	if (cpu->status_indicators[LESS_THAN] && (data & (1 << LESS_THAN)))
	{
		did_branch = true;
	}
	if (cpu->status_indicators[FLAGS_MATCH] && (data & (1 << FLAGS_MATCH)))
	{
		did_branch = true;
	}

	if (did_branch)
	{
		uint16_t new_pointer_addr = get_data_from_halfword(cpu->pr[operand.pointer_register_index].pointer_value);
		set_data_in_halfword(new_pointer_addr, &(cpu->pr[IP].pointer_value));
		return true;
	}
	return false;
}

bool branch_not(byte_t byte, operand_t operand, cpu_t *cpu)
{
	bool did_branch = false;
	uint8_t data = get_data_from_byte(byte);
	if (!(cpu->status_indicators[CONDITIONAL_SUBTRACT]) && (data & (1 << CONDITIONAL_SUBTRACT)))
	{
		did_branch = true;
	}
	if (!(cpu->status_indicators[OVERFLOW]) && (data & (1 << OVERFLOW)))
	{
		did_branch = true;
	}
	if (!(cpu->status_indicators[GREATER_THAN]) && (data & (1 << GREATER_THAN)))
	{
		did_branch = true;
	}
	if (!(cpu->status_indicators[EQUAL]) && (data & (1 << EQUAL)))
	{
		did_branch = true;
	}
	if (!(cpu->status_indicators[LESS_THAN]) && (data & (1 << LESS_THAN)))
	{
		did_branch = true;
	}
	if (!(cpu->status_indicators[FLAGS_MATCH]) && (data & (1 << FLAGS_MATCH)))
	{
		did_branch = true;
	}

	if (did_branch)
	{
		uint16_t new_pointer_addr = get_data_from_halfword(cpu->pr[operand.pointer_register_index].pointer_value);
		set_data_in_halfword(new_pointer_addr, &(cpu->pr[IP].pointer_value));
		return true;
	}
	return false;
}

bool abs_short(cpu_t *cpu)
{
	halfword_t operand, new_stack_value;
	int16_t abs_data;

	operand = pop_operand_halfword(cpu);
	abs_data = (int16_t)get_data_from_halfword(operand);
	if (abs_data < 0)
	{
		abs_data = (int16_t)-abs_data;
	}

	new_stack_value = put_data_into_halfword((uint16_t)abs_data);
	copy_halfword_flags(operand, &new_stack_value);

	cpu->status_indicators[OVERFLOW] = false;
	if (abs_data < 0)
	{
		cpu->status_indicators[OVERFLOW] = true;
		set_flag_halfword(&new_stack_value, 0);
	}

	push_operand_halfword(new_stack_value, cpu);
	return false;
}

bool abs_long(cpu_t *cpu)
{
	word_t operand, new_stack_value;
	int32_t abs_data;

	operand = pop_operand_word(cpu);
	abs_data = (int32_t)get_data_from_word(operand);
	if (abs_data < 0)
	{
		abs_data = -abs_data;
	}

	new_stack_value = put_data_into_word((int32_t)abs_data);
	copy_word_flags(operand, &new_stack_value);

	cpu->status_indicators[OVERFLOW] = false;
	if (abs_data < 0)
	{
		cpu->status_indicators[OVERFLOW] = true;
		set_flag_word(&new_stack_value, 0);
	}

	push_operand_word(new_stack_value, cpu);
	return false;
}

bool add_short(cpu_t *cpu)
{
	halfword_t operand1, operand2, new_stack_value;
	int16_t data1, data2, result;
	bool has_overflowed;

	operand1 = pop_operand_halfword(cpu);
	operand2 = pop_operand_halfword(cpu);
	data1 = (int16_t)get_data_from_halfword(operand1);
	data2 = (int16_t)get_data_from_halfword(operand2);

	result = (int16_t)(data1 + data2);

	has_overflowed = true;
	if (data1 >= 0 && result < data2)
	{
		has_overflowed = true;
	}

	if (data1 < 0 && result > data2)
	{
		has_overflowed = true;
	}

	new_stack_value = put_data_into_halfword((uint16_t)result);
	copy_halfword_flags(operand2, &new_stack_value);

	if (has_overflowed)
	{
		set_flag_halfword(&new_stack_value, 0);
	}

	push_operand_halfword(new_stack_value, cpu);
	return false;
}

bool add_long(cpu_t *cpu)
{
	word_t operand1, operand2, new_stack_value;
	int32_t data1, data2, result;
	bool has_overflowed;

	operand1 = pop_operand_word(cpu);
	operand2 = pop_operand_word(cpu);
	data1 = (int32_t)get_data_from_word(operand1);
	data2 = (int32_t)get_data_from_word(operand2);

	result = (int32_t)(data1 + data2);

	has_overflowed = false;
	if (data1 >= 0 && result < data2)
	{
		has_overflowed = true;
	}

	if (data1 < 0 && result > data2)
	{
		has_overflowed = true;
	}

	new_stack_value = put_data_into_word((uint32_t)result);
	copy_word_flags(operand2, &new_stack_value);

	if (has_overflowed)
	{
		set_flag_word(&new_stack_value, 0);
	}

	push_operand_word(new_stack_value, cpu);
	return false;
}

bool one_byte(cpu_t *cpu)
{
	byte_t operand;
	pop_operand_byte(cpu);

	operand = put_data_into_byte(b(11111111));
	set_flag_byte(&operand);
	push_operand_byte(operand, cpu);
	return false;
}

bool dup_byte(cpu_t *cpu)
{
	byte_t operand;

	operand = pop_operand_byte(cpu);
	push_operand_byte(operand, cpu);
	push_operand_byte(operand, cpu);
	return false;
}

bool dup_halfword(cpu_t *cpu)
{
	halfword_t operand;

	operand = pop_operand_halfword(cpu);
	push_operand_halfword(operand, cpu);
	push_operand_halfword(operand, cpu);
	return false;
}

bool dup_word(cpu_t *cpu)
{
	word_t operand;

	operand = pop_operand_word(cpu);
	push_operand_word(operand, cpu);
	push_operand_word(operand, cpu);
	return false;
}

bool sluff_byte(cpu_t *cpu)
{
	pop_operand_byte(cpu);
	return false;
}

bool sluff_halfword(cpu_t *cpu)
{
	pop_operand_halfword(cpu);
	return false;
}

bool sluff_word(cpu_t *cpu)
{
	pop_operand_word(cpu);
	return false;
}

bool xch_byte(cpu_t *cpu)
{
	byte_t operand1, operand2;

	operand1 = pop_operand_byte(cpu);
	operand2 = pop_operand_byte(cpu);
	push_operand_byte(operand1, cpu);
	push_operand_byte(operand2, cpu);
	return false;
}

bool xch_halfword(cpu_t *cpu)
{
	halfword_t operand1, operand2;

	operand1 = pop_operand_halfword(cpu);
	operand2 = pop_operand_halfword(cpu);
	push_operand_halfword(operand1, cpu);
	push_operand_halfword(operand2, cpu);
	return false;
}

bool xch_word(cpu_t *cpu)
{
	word_t operand1, operand2;

	operand1 = pop_operand_word(cpu);
	operand2 = pop_operand_word(cpu);
	push_operand_word(operand1, cpu);
	push_operand_word(operand2, cpu);
	return false;
}

bool hcf(byte_t opcode, cpu_t *cpu)
{
	size_t i;
	uint16_t temp16;
	uint8_t temp8;
	printf("HCF Instruction caught!\n");
	printf("Illegal Opcode: %d%02hhX\n",
		get_flag_from_byte(opcode),
		get_data_from_byte(opcode));

	for (i = 0; i < PR_SIZE; i++)
	{
		printf("Pointer Register: %zu\n", i);
		temp16 = get_data_from_halfword(cpu->pr[i].pointer_link);
		printf("\tPointer Link: %hX\n", temp16);
		temp16 = get_data_from_halfword(cpu->pr[i].pointer_value);
		printf("\tPointer Value: %hX\n", temp16);
		printf("\tFlags: ");
		printf("%d", get_flag_from_halfword(cpu->pr[i].pointer_link,  1));
		printf("%d", get_flag_from_halfword(cpu->pr[i].pointer_link,  0));
		printf("%d", get_flag_from_halfword(cpu->pr[i].pointer_value, 1));
		printf("%d", get_flag_from_halfword(cpu->pr[i].pointer_value, 0));
		printf("\n\n");
	}

	for (i = 0; i < BR_SIZE; i++)
	{
		printf("Base Register: %zu\n", i);
		temp8 = get_data_from_byte(cpu->br[i].bounds);
		printf("\tBase Bounds: %hhX\n", temp8);
		temp16 = get_data_from_halfword(cpu->br[i].start_page);
		printf("\tBase Start Page: %hX\n", temp16);
		printf("\tFlags: ");
		printf("%d", get_flag_from_byte(cpu->br[i].zero));
		printf("%d", get_flag_from_byte(cpu->br[i].bounds));
		printf("%d", get_flag_from_halfword(cpu->br[i].start_page, 1));
		printf("%d", get_flag_from_halfword(cpu->br[i].start_page, 0));
		printf("\n\n");
	}

	printf("ASF Register\n");
	temp16 = get_data_from_halfword(cpu->pr_14.consecutive_storage_link);
	printf("\tConsecutive Storage Link: %hX\n", temp16);
	temp16 = get_data_from_halfword(cpu->pr_14.free_list_link);
	printf("\tFree List Link: %hX\n\n", temp16);

	printf("Stack values (top to bottom)\n");
	for (i = 0; i < 6; i++)
	{
		byte_t byte = pop_operand_byte(cpu);
		printf("\tvalue: %hhX\n", get_data_from_byte(byte));
		printf("\t\tflag: %d\n", get_flag_from_byte(byte));
	}
	printf("\n");
	exit(EXIT_FAILURE);
	return true;
}

size_t decode_byte_t(byte_t byte)
{
	return (size_t)(get_data_from_byte(byte) | (get_flag_from_byte(byte) << 8));
}

operand_t decode_operand(raw_address_t operand_address, const cpu_t *cpu)
{
	byte_t operand_preamble = get_byte_from_memory(operand_address);
	bool    flag = get_flag_from_byte(operand_preamble);
	uint8_t data = get_data_from_byte(operand_preamble);
	operand_t operand = {
		.is_long  = flag,
		.pointer_register_index = (pr_index_t)(data >> 4),
		.pre_push = (data >> 3) & 1,
		.post_pop = (data >> 2) & 1,
		.indirect = (data >> 1) & 1,
		.last     = (data >> 0) & 1
	};
	assert(operand.pointer_register_index < PR_SIZE);

	if (operand.is_long)
	{
		raw_address_t m0_addr = operand_address + 1;
		raw_address_t m1_addr = operand_address + 2;

		byte_t m0_byte = get_byte_from_memory(m0_addr);
		byte_t m1_byte = get_byte_from_memory(m1_addr);
		if (operand.indirect)
		{
			uint8_t m0_data, m1_data;
			pr_index_t second_pr_index = (pr_index_t)(get_data_from_byte(m0_byte) >> 4);
			assert(second_pr_index <= 15);
			if (second_pr_index == 15)
			{
				/* Note that peek is idempotent, unlike other stack
				 * operations, so we may do it twice. */
				m0_data = get_data_from_byte(peek_operand_halfword(cpu).high);
				m1_data = get_data_from_byte(peek_operand_halfword(cpu).low);
			}
			else if (second_pr_index == ASP)
			{
				m0_data = get_data_from_byte(cpu->pr_14.free_list_link.high);
				m1_data = get_data_from_byte(cpu->pr_14.free_list_link.low);
			}
			else
			{
				m0_data = get_data_from_byte(cpu->pr[second_pr_index].pointer_value.high);
				m1_data = get_data_from_byte(cpu->pr[second_pr_index].pointer_value.low);
			}
			set_data_in_byte(m0_data, &m0_byte);
			set_data_in_byte(m1_data, &m1_byte);
		}
		operand.m[0] = m0_byte;
		operand.m[1] = m1_byte;
	}

	return operand;
}

void handle_postslash(operand_t operand, cpu_t *cpu)
{
	if (!operand.post_pop)
	{
		return;
	}


	pr_index_t pointer_register_index = operand.pointer_register_index;
	assert(pointer_register_index < PR_SIZE);

	number_format_t old_pr = cpu->pr[pointer_register_index];
	cpu->pr[pointer_register_index] = follow_link(old_pr, cpu);

	uint16_t csl_data = get_data_from_halfword(cpu->pr_14.consecutive_storage_link);
	uint16_t fll_data = get_data_from_halfword(cpu->pr_14.free_list_link);

	if (fll_data == csl_data && get_data_from_halfword(old_pr.pointer_link) == fll_data + WORD_SIZE)
	{
		csl_data += WORD_SIZE;
		fll_data += WORD_SIZE;
	}
	else
	{
		raw_address_t new_cell_addr = get_address_from_link(old_pr, cpu);
		halfword_t temp = get_halfword_from_memory(new_cell_addr);

		set_data_in_halfword(fll_data, &temp);

		put_halfword_into_memory(temp, new_cell_addr);

		fll_data = get_data_from_halfword(old_pr.pointer_link);
	}

	set_data_in_halfword(csl_data, &(cpu->pr_14.consecutive_storage_link));
	set_data_in_halfword(fll_data, &(cpu->pr_14.free_list_link));
}

void handle_preslash(operand_t operand, cpu_t *cpu)
{
	if (!operand.pre_push)
	{
		return;
	}

	number_format_t temp = {
		.pointer_link  = cpu->pr_14.consecutive_storage_link,
		.pointer_value = cpu->pr_14.free_list_link
	};
	raw_address_t new_cell_addr = get_address_from_pointer(temp, cpu);

	pr_index_t pointer_register_index = operand.pointer_register_index;
	assert(pointer_register_index < PR_SIZE);

	halfword_t old_data = get_halfword_from_memory(new_cell_addr);

	put_halfword_into_memory(cpu->pr[pointer_register_index].pointer_link, new_cell_addr);

	set_data_in_halfword((uint16_t)(get_data_from_halfword(temp.pointer_value) + HALFWORD_SIZE), &(temp.pointer_value));

	new_cell_addr = get_address_from_pointer(temp, cpu);

	put_halfword_into_memory(cpu->pr[pointer_register_index].pointer_value, new_cell_addr);

	uint16_t csl_data = get_data_from_halfword(cpu->pr_14.consecutive_storage_link);
	uint16_t fll_data = get_data_from_halfword(cpu->pr_14.free_list_link);

	set_data_in_halfword(fll_data, &(cpu->pr[pointer_register_index].pointer_link));

	if (fll_data == csl_data)
	{
		assert(csl_data >= WORD_SIZE);
		csl_data = (uint16_t)(csl_data - WORD_SIZE);
		fll_data = csl_data;

		set_data_in_halfword(csl_data, &(cpu->pr_14.consecutive_storage_link));
		set_data_in_halfword(fll_data, &(cpu->pr_14.free_list_link));
	}
	else
	{
		copy_halfword_flags(cpu->pr_14.free_list_link, &old_data);
		cpu->pr_14.free_list_link = old_data;
	}
}

operand_return_t canonicalize_operand(operand_t operand, cpu_t *cpu)
{
	operand_return_t operand_return = {
		.hit_conditional = false,
		.conditional_subtract_result = false,
		.changed_IP = false,
	};

	if (!operand.is_long && operand.indirect)
	{
		pr_index_t pointer_register_index = operand.pointer_register_index;
		assert(pointer_register_index < PR_SIZE);

		raw_address_t indirect_address = get_address_from_pointer(cpu->pr[pointer_register_index], cpu);

		halfword_t indirect_memory = get_halfword_from_memory(indirect_address);
		copy_halfword_flags(cpu->pr[pointer_register_index].pointer_value, &indirect_memory);

		if (operand.pointer_register_index != IP)
		{
			cpu->pr[pointer_register_index].pointer_value = indirect_memory;
		}
		else
		{
			operand_return.changed_IP = true;
			operand_return.new_IP = indirect_memory;
		}
	}
	else if (operand.is_long)
	{
		pr_index_t pointer_register_index = operand.pointer_register_index;
		assert(pointer_register_index < PR_SIZE);

		bool m0_flag = get_flag_from_byte(operand.m[0]);
		bool m1_flag = get_flag_from_byte(operand.m[1]);

		halfword_t new_halfword = {
			.high = operand.m[0],
			.low  = operand.m[1]
		};

		if (m0_flag)
		{
			if (m1_flag)
			{
				uint16_t count = get_data_from_halfword(new_halfword);
				uint16_t pointer = get_data_from_halfword(cpu->pr[pointer_register_index].pointer_value);
				for (uint16_t i = 0; (i < count) && (pointer != 0); ++i)
				{
					raw_address_t indirect_address = get_address_from_pointer(cpu->pr[pointer_register_index], cpu);
					pointer = get_data_from_halfword(get_halfword_from_memory(indirect_address));

					new_halfword = put_data_into_halfword(pointer);
					copy_halfword_flags(cpu->pr[pointer_register_index].pointer_value, &new_halfword);
					if (pointer_register_index != IP)
					{
						cpu->pr[pointer_register_index].pointer_value = new_halfword;
					}
					else
					{
						operand_return.changed_IP = true;
						operand_return.new_IP = new_halfword;
					}
				}
			}
			else
			{
				uint16_t new_data = get_data_from_halfword(new_halfword);
				uint16_t old_data = get_data_from_halfword(cpu->pr[pointer_register_index].pointer_value);
				new_data = (uint16_t)(old_data - new_data);
				new_halfword = put_data_into_halfword(new_data);

				operand_return.hit_conditional = true;
				if ((new_data >> 15) == 0 && new_data > 0)
				{
					copy_halfword_flags(cpu->pr[pointer_register_index].pointer_value, &new_halfword);
					if (pointer_register_index != IP)
					{
						cpu->pr[pointer_register_index].pointer_value = new_halfword;
					}
					else
					{
						operand_return.changed_IP = true;
						operand_return.new_IP = new_halfword;
					}
					operand_return.conditional_subtract_result = false;
				}
				else
				{
					operand_return.conditional_subtract_result = true;
				}
			}
		}
		else
		{
			if (m1_flag)
			{
				uint16_t new_data = get_data_from_halfword(new_halfword);
				uint16_t old_data = get_data_from_halfword(cpu->pr[pointer_register_index].pointer_value);
				new_halfword = put_data_into_halfword((uint16_t)(old_data + new_data));
			}

			copy_halfword_flags(cpu->pr[pointer_register_index].pointer_value, &new_halfword);
			if (pointer_register_index != IP)
			{
				cpu->pr[pointer_register_index].pointer_value = new_halfword;
			}
			else
			{
				operand_return.changed_IP = true;
				operand_return.new_IP = new_halfword;
			}
		}
	}
	return operand_return;
}

void instruction_fetch_loop(cpu_t *cpu)
{
	for (;;)
	{
		raw_address_t instruction  = get_address_from_pointer(cpu->pr[IP], cpu);
		byte_t opcode              = get_byte_from_memory(instruction);

		operand_table_t decoded_opcode = opcodes[decode_byte_t(opcode)];

		raw_address_t cur_ip_addr = instruction;

		byte_t bitmask;

		size_t num_operands = 0;
		switch (decoded_opcode.num_operands)
		{
			case ZERO_OPS:
				num_operands = 0;
				break;
			case ONE_OPS:
				num_operands = 1;
				break;
			case TWO_OPS:
				num_operands = 2;
				break;
			case BIT_OPS:
				cur_ip_addr += BYTE_SIZE;
				{
					bitmask = get_byte_from_memory(cur_ip_addr);
				}

				num_operands = 1;
				break;
			case MANY_OPS:
				for (raw_address_t temp_ip_addr = cur_ip_addr + BYTE_SIZE; ;)
				{
					operand_t operand = decode_operand(temp_ip_addr, cpu);
					num_operands++;

					temp_ip_addr += BYTE_SIZE;
					if (operand.is_long)
					{
						temp_ip_addr += HALFWORD_SIZE;
					}
					if (!operand.last)
					{
						break;
					}
				}
				break;
			default:
				hcf(opcode, cpu);
				break;
		}

		operand_t *operands = (operand_t *) malloc(sizeof(*operands) * num_operands);
		if (operands == NULL)
		{
			perror("error allocating operands");
			exit(EXIT_FAILURE);
		}

		cur_ip_addr += BYTE_SIZE;
		for (size_t i = 0; i < num_operands; i++)
		{
			operands[i] = decode_operand(cur_ip_addr, cpu);

			cur_ip_addr += BYTE_SIZE;
			if (operands[i].is_long)
			{
				cur_ip_addr += HALFWORD_SIZE;
			}
		}

		for (size_t i = 0; i < num_operands; i++)
		{
			handle_preslash(operands[i], cpu);
		}

		operand_return_t result = {
			.conditional_subtract_result = false,
			.changed_IP = false
		};

		bool cond_result_or = false;

		for (size_t i = 0; i < num_operands; i++)
		{
			operand_return_t operand_ret = canonicalize_operand(operands[i], cpu);

			cond_result_or |= result.hit_conditional;

			if (result.conditional_subtract_result == false)
			{
				result.conditional_subtract_result = operand_ret.conditional_subtract_result;
			}

			if (operand_ret.changed_IP == true)
			{
				result.changed_IP = operand_ret.changed_IP;
				result.new_IP = operand_ret.new_IP;
			}
		}

		if (cond_result_or)
		{
			cpu->status_indicators[CONDITIONAL_SUBTRACT] = result.conditional_subtract_result;
		}

		bool did_branch = false;
		if (result.conditional_subtract_result == false)
		{
			switch (decoded_opcode.num_operands)
			{
				case ZERO_OPS:
					did_branch = decoded_opcode.opcode_impl.zero_args(cpu);
					break;
				case ONE_OPS:
					did_branch = decoded_opcode.opcode_impl.one_args(operands[0], cpu);
					break;
				case TWO_OPS:
					assert(!(operands[0].is_long && operands[1].is_long));
					did_branch = decoded_opcode.opcode_impl.two_args(operands[0], operands[1], cpu);
					break;
				case BIT_OPS:
					did_branch = decoded_opcode.opcode_impl.bit_args(bitmask, operands[0], cpu);
					break;
				default:
					did_branch = hcf(opcode, cpu);
					break;
			}
		}

		uint16_t new_pointer_addr = increment_ip(cur_ip_addr - instruction, cpu);
		if (result.conditional_subtract_result == false)
		{
			if (result.changed_IP)
			{
				new_pointer_addr = get_data_from_halfword(result.new_IP);
			}

			if (did_branch)
			{
				new_pointer_addr = get_data_from_halfword(cpu->pr[IP].pointer_value);
			}
		}

		set_data_in_halfword(new_pointer_addr, &(cpu->pr[IP].pointer_value));

		for (size_t i = 0; i < num_operands; i++)
		{
			handle_postslash(operands[i], cpu);
		}

		free(operands);
	}
}

uint16_t increment_ip(uint16_t increment, const cpu_t *cpu)
{
	uint16_t old_pointer_value = get_data_from_halfword(cpu->pr[IP].pointer_value);
	uint16_t new_pointer_value = (uint16_t)(old_pointer_value + increment);
	assert(new_pointer_value > old_pointer_value);
	return new_pointer_value;
}

void cpu_ctor(cpu_t *cpu)
{
	memset(cpu, 0, sizeof(*cpu));

	/* Set pr[OSP] to 0x1000 */
	cpu->pr[OSP].pointer_value = put_data_into_halfword(0x1000);
}

void core_memory_ctor(void)
{
	core_memory = (byte_t *)malloc(sizeof(byte_t [NUM_BYTES]));
	if (core_memory == NULL)
	{
		perror("error allocating core_memory");
		exit(EXIT_FAILURE);
	}
}

void core_memory_dtor(void)
{
	free(core_memory);
}

void load_object_file(const char *objfile)
{
	raw_address_t addr = 0;

	FILE *fp = fopen(objfile, "rb");
	if (fp == NULL)
	{
		perror("Could not open objfile");
		exit(EXIT_FAILURE);
	}

	while (!feof(fp))
	{
		byte_t byte;
		if (fread(&byte, sizeof(byte), 1, fp) == 1)
		{
			put_byte_into_memory(byte, addr);
			addr += 1;
		}
		else if (ferror(fp))
		{
			perror("Bad file read");
			exit(EXIT_FAILURE);
		}
	}

	if (fclose(fp))
	{
		perror("Error closing file handle");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, const char *argv[])
{
	const char *objfile;
	cpu_t cpu;

	objfile = argv[1];

	if (objfile == NULL || argc != 2)
	{
		printf("%s objfile\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	cpu_ctor(&cpu);

	core_memory_ctor();
	if (atexit(core_memory_dtor))
	{
		perror("could not register atexit for core_memory_dtor");
	}

	load_object_file(objfile);

	instruction_fetch_loop(&cpu);

	return EXIT_SUCCESS;
}
