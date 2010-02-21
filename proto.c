#include <stdlib.h>
#include <stdio.h>

#include "proto.h"

static
byte_t *memory;

raw_address_t get_address_common(const number_format_t *p, const cpu_t *cpu)
{
	uint8_t third_flag  = get_flag_from_byte(p->pointer_link.low);
	uint8_t second_flag = get_flag_from_byte(p->pointer_value.high);
	uint8_t first_flag = get_flag_from_byte(p->pointer_value.low);

	uint8_t br_num = (uint8_t)(first_flag | second_flag << 0 | third_flag << 2);

	return (raw_address_t)((cpu->br[br_num].start_page.high.data << 16) |
						   (cpu->br[br_num].start_page.low.data  << 8));

}

raw_address_t get_address_from_pointer(const number_format_t *p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= get_data_from_byte(p->pointer_value.low);
	ret += get_data_from_byte(p->pointer_value.high) << 8;

	return ret;
}

raw_address_t get_address_from_link(const number_format_t *p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= get_data_from_byte(p->pointer_link.low);
	ret += get_data_from_byte(p->pointer_link.high) << 8;

	return ret;
}

number_format_t get_pointer_register_from_memory(raw_address_t addr)
{
	number_format_t ret = {
		.pointer_link = get_halfword_from_memory(addr),
		.pointer_value = get_halfword_from_memory(addr + 2)
	};

	return ret;
}

byte_t get_byte_from_memory(raw_address_t addr)
{
	return memory[addr];
}

halfword_t get_halfword_from_memory(raw_address_t addr)
{
	halfword_t halfword = {
		.high = get_byte_from_memory(addr),
		.low  = get_byte_from_memory(addr + 1)
	};
	return halfword;
}

word_t get_word_from_memory(raw_address_t addr)
{
	word_t word = {
		.high = get_halfword_from_memory(addr),
		.low  = get_halfword_from_memory(addr + 2),
	};
	return word;
}

void put_byte_into_memory(byte_t arg, raw_address_t addr)
{
	memory[addr] = arg;
}

void put_halfword_into_memory(halfword_t arg, raw_address_t addr)
{
	put_byte_into_memory(arg.high, addr);
	put_byte_into_memory(arg.low, addr + 1);
}

void put_word_into_memory(word_t arg, raw_address_t addr)
{
	put_halfword_into_memory(arg.high, addr);
	put_halfword_into_memory(arg.low, addr + 2);
}

uint8_t get_data_from_byte(byte_t byte)
{
	return byte.data;
}

bool get_flag_from_byte(byte_t byte)
{
	return byte.flag;
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
	byte_t byte = {
		.data = data,
		.flag = 0
	};
	return byte;
}

halfword_t put_data_into_halfword(uint16_t data)
{
	halfword_t halfword = {
		.high = put_data_into_byte((uint8_t)((data >> 8) & 0xFF)),
		.low  = put_data_into_byte((uint8_t)(data        & 0xFF))
	};
	return halfword;
}

word_t put_data_into_word(uint32_t data)
{
	word_t word = {
		.high = put_data_into_halfword((uint16_t)((data >> 16) & 0xFFFF)),
		.low  = put_data_into_halfword((uint16_t)(data         & 0xFFFF))
	};
	return word;
}

void copy_byte_flags(const byte_t *from, byte_t *to)
{
	to->flag = get_flag_from_byte(*from);
}

void copy_halfword_flags(const halfword_t *from, halfword_t *to)
{
	copy_byte_flags(&(from->low), &(to->low));
	copy_byte_flags(&(from->high), &(to->high));
}

void copy_word_flags(const word_t *from, word_t *to)
{
	copy_halfword_flags(&(from->low), &(to->low));
	copy_halfword_flags(&(from->high), &(to->high));
}

byte_t pop_operand_byte(cpu_t *cpu)
{
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data -= 1;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_byte_from_memory(operand_pointer);
}

halfword_t pop_operand_halfword(cpu_t *cpu)
{
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data -= 2;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_halfword_from_memory(operand_pointer);
}

word_t pop_operand_word(cpu_t *cpu)
{
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data -= 4;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_word_from_memory(operand_pointer);
}

void push_operand_byte(byte_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data += 1;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	put_byte_into_memory(arg, operand_pointer);
}

void push_operand_halfword(halfword_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data += 2;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;
	put_halfword_into_memory(arg, operand_pointer);
}

void push_operand_word(word_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data += 4;

	halfword_t halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;
	put_word_into_memory(arg, operand_pointer);
}

void abs_long(cpu_t *cpu)
{
	word_t operand = pop_operand_word(cpu);
	uint32_t data = get_data_from_word(operand);
	bool overflow = false;

	if (data >> 31)
	{
		data = ~data + 1;
	}

	if (data >> 31)
	{
		overflow = true;
	}

	word_t word = put_data_into_word(data);
	copy_word_flags(&operand, &word);

	if (overflow)
	{
		word.high.low.flag = true;
	}

	push_operand_word(word, cpu);
}

void execute(byte_t opcode, cpu_t *cpu)
{
	if (get_flag_from_byte(opcode))
	{
		switch (opcode.data)
		{
			case 0b10010101:
				abs_long(cpu);
				break;
		}
	}
}

int main(void)
{
	cpu_t cpu;
	memory = malloc(sizeof(byte_t) * (1 << 24));
	if (!memory)
	{
		printf("oh god help!!");
		exit(-1);
	}
	for (;;)
	{
		nf_t current_instruction = cpu.pr[0];

		raw_address_t instruction = get_address_from_pointer(&current_instruction, &cpu);

		byte_t opcode = get_byte_from_memory(instruction);
		execute(opcode, &cpu);

		raw_address_t next = get_address_from_link(&current_instruction, &cpu);
		cpu.pr[0] = get_pointer_register_from_memory(next);
	}
}
