#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t data;
	bool flag;
} byte_t;

typedef struct
{
	byte_t low;
	byte_t high;
} halfword_t;

typedef struct
{
	halfword_t low;
	halfword_t high;
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

typedef uint32_t raw_address_t;

typedef struct
{
	number_format_t pr[14];
	base_pointer_t br[6];
	asf_t pr_14;
} cpu_t;

byte_t *memory;

static void
exec(nf_t instruction_word)
{
	return;
}

raw_address_t get_address_common(number_format_t *p, cpu_t *cpu)
{
	uint8_t br_num;
	br_num = (p->pointer_link.low.flag << 2);
	br_num |= (p->pointer_value.high.flag << 1);
	br_num |= (p->pointer_value.low.flag);

	return (raw_address_t)((cpu->br[br_num].start_page.high.data << 16) |
						   (cpu->br[br_num].start_page.low.data  << 8));

}

raw_address_t get_address_from_pointer(number_format_t *p, cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (p->pointer_value.low.data);
	ret += (p->pointer_value.high.data << 8);

	return ret;
}

raw_address_t get_address_from_link(number_format_t *p, cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (p->pointer_link.low.data);
	ret += (p->pointer_link.high.data << 8);

	return ret;
}

number_format_t get_pointer_register_from_memory(raw_address_t addr)
{
	number_format_t ret;
	ret.pointer_link.high = memory[addr];
	ret.pointer_link.low = memory[addr + 1];
	ret.pointer_value.high = memory[addr + 2];
	ret.pointer_value.low = memory[addr + 3];

	return ret;
}

byte_t get_byte_from_memory(raw_address_t addr)
{
	return memory[addr];
}

halfword_t get_halfword_from_memory(raw_address_t addr)
{
	halfword_t halfword = {
		.low = memory[addr + 1],
		.high = memory[addr]
	};
	return halfword;
}

word_t get_word_from_memory(raw_address_t addr)
{
	word_t word = {
		.low  = get_halfword_from_memory(addr + 2),
		.high = get_halfword_from_memory(addr),
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

uint8_t byte_to_data(byte_t byte)
{
	return byte.data;
}

uint16_t halfword_to_data(halfword_t halfword)
{
	return (halfword.high.data << 8) | (halfword.low.data);
}

uint32_t word_to_data(word_t word)
{
	return (halfword_to_data(word.high) << 16) | halfword_to_data(word.low);
}

byte_t data_to_byte(uint8_t data)
{
	byte_t byte = { .data = data, .flag = 0 };
	return byte;
}

halfword_t data_to_halfword(uint16_t data)
{
	halfword_t halfword = {
		.high = data_to_byte((data >> 8) & 0xFF),
		.low  = data_to_byte(data        & 0xFF)
	};
	return halfword;
}

word_t data_to_word(uint32_t data)
{
	word_t word = {
		.high = data_to_halfword((data >> 16) & 0xFFFF),
		.low  = data_to_halfword(data         & 0xFFFF)
	};
	return word;
}

void copy_byte_flags(byte_t *from, byte_t *to)
{
	to->flag = from->flag;
}

void copy_halfword_flags(halfword_t *from, halfword_t *to)
{
	copy_byte_flags(&(from->low), &(to->low));
	copy_byte_flags(&(from->high), &(to->high));
}

void copy_word_flags(word_t *from, word_t *to)
{
	copy_halfword_flags(&(from->low), &(to->low));
	copy_halfword_flags(&(from->high), &(to->high));
}

byte_t pop_operand_byte(cpu_t *cpu)
{
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data -= 1;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_byte_from_memory(operand_pointer);
}

halfword_t pop_operand_halfword(cpu_t *cpu)
{
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data -= 2;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_halfword_from_memory(operand_pointer);
}

word_t pop_operand_word(cpu_t *cpu)
{
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data -= 4;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_word_from_memory(operand_pointer);
}

void push_operand_byte(byte_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data += 1;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	put_byte_into_memory(arg, operand_pointer);
}

void push_operand_halfword(halfword_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data += 2;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;
	put_halfword_into_memory(arg, operand_pointer);
}

void push_operand_word(word_t arg, cpu_t *cpu)
{
	raw_address_t operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	uint16_t data = halfword_to_data(cpu->pr[13].pointer_value);
	data += 4;

	halfword_t halfword = data_to_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;
	put_word_into_memory(arg, operand_pointer);
}

void abs_long(cpu_t *cpu)
{
	word_t operand = pop_operand_word(cpu);
	uint32_t data = word_to_data(operand);
	bool overflow = false;
	if ((1 << 31) & data)
	{
		data = -data;
		if ((1 << 31) & data)
		{
			overflow = true;
		}
	}

	word_t word = data_to_word(data);
	copy_word_flags(&operand, &word);

	if (overflow)
	{
		word.high.low.flag = true;
	}

	push_operand_word(word, cpu);
}

void execute(byte_t opcode, cpu_t *cpu)
{
	if (opcode.flag)
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
	return 0;
}
