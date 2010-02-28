#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "proto.h"

byte_t *core_memory;

raw_address_t get_address_common(const number_format_t *p, const cpu_t *cpu)
{
	uint8_t third_flag  = (uint8_t)((get_flag_from_byte(p->pointer_link.low)   << 2) & 0xFF);
	uint8_t second_flag = (uint8_t)((get_flag_from_byte(p->pointer_value.high) << 1) & 0xFF);
	uint8_t first_flag  = (uint8_t)((get_flag_from_byte(p->pointer_value.low)  << 0) & 0xFF);

	uint8_t br_num = (uint8_t)(first_flag | second_flag | third_flag);

	assert(br_num < sizeof(cpu->br)/sizeof(cpu->br[0]));

	return (raw_address_t)((cpu->br[br_num].start_page.high.data << 16) |
						   (cpu->br[br_num].start_page.low.data  << 8));

}

raw_address_t get_address_from_pointer(const number_format_t *p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (raw_address_t) get_data_from_byte(p->pointer_value.low);
	ret += (raw_address_t)(get_data_from_byte(p->pointer_value.high) << 8);

	return ret;
}

raw_address_t get_address_from_link(const number_format_t *p, const cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret |= (raw_address_t) get_data_from_byte(p->pointer_link.low);
	ret += (raw_address_t)(get_data_from_byte(p->pointer_link.high) << 8);

	return ret;
}

number_format_t get_pointer_register_from_memory(raw_address_t addr)
{
	number_format_t ret = {
		.pointer_link  = get_halfword_from_memory(addr),
		.pointer_value = get_halfword_from_memory(addr + 2)
	};

	return ret;
}

byte_t get_byte_from_memory(raw_address_t addr)
{
	assert(addr < (raw_address_t)(1 << 24));
	return core_memory[addr];
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
	assert(addr < (raw_address_t)(1 << 24));
	core_memory[addr] = arg;
}

void put_halfword_into_memory(halfword_t arg, raw_address_t addr)
{
	put_byte_into_memory(arg.high, addr);
	put_byte_into_memory(arg.low,  addr + 1);
}

void put_word_into_memory(word_t arg, raw_address_t addr)
{
	put_halfword_into_memory(arg.high, addr);
	put_halfword_into_memory(arg.low,  addr + 2);
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
	assert(byte_num <= 1);

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
	assert(byte_num <= 1);

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
	assert(byte_num <= 1);

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
	size_t halfword_num = byte_num % 2;

	assert(byte_num <= 3);

	if (byte_num <= 1)
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
	size_t halfword_num = byte_num % 2;

	assert(byte_num <= 3);

	if (byte_num <= 1)
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
	size_t halfword_num = byte_num % 2;

	assert(byte_num <= 3);

	if (byte_num <= 1)
	{
		clear_flag_halfword(&(word->high), halfword_num);
	}
	else
	{
		clear_flag_halfword(&(word->low), halfword_num);
	}
}

void copy_byte_flags(const byte_t *from, byte_t *to)
{
	to->flag = get_flag_from_byte(*from);
}

void copy_halfword_flags(const halfword_t *from, halfword_t *to)
{
	copy_byte_flags(&(from->high), &(to->high));
	copy_byte_flags(&(from->low),  &(to->low));
}

void copy_word_flags(const word_t *from, word_t *to)
{
	copy_halfword_flags(&(from->high), &(to->high));
	copy_halfword_flags(&(from->low),  &(to->low));
}

byte_t pop_operand_byte(cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data - 1);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_byte_from_memory(operand_pointer);
}

halfword_t pop_operand_halfword(cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data - 2);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_halfword_from_memory(operand_pointer);
}

word_t pop_operand_word(cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data - 4);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	return get_word_from_memory(operand_pointer);
}

void push_operand_byte(byte_t arg, cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data + 1);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	put_byte_into_memory(arg, operand_pointer);
}

void push_operand_halfword(halfword_t arg, cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data + 2);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	put_halfword_into_memory(arg, operand_pointer);
}

void push_operand_word(word_t arg, cpu_t *cpu)
{
	halfword_t halfword;
	raw_address_t operand_pointer;
	uint16_t data;

	operand_pointer = get_address_from_pointer(&(cpu->pr[13]), cpu);
	data = get_data_from_halfword(cpu->pr[13].pointer_value);
	data = (uint16_t)(data + 4);

	halfword = put_data_into_halfword(data);
	copy_halfword_flags(&(cpu->pr[13].pointer_value), &halfword);

	cpu->pr[13].pointer_value = halfword;

	put_word_into_memory(arg, operand_pointer);
}

void dup_byte(cpu_t *cpu)
{
	byte_t operand;

	operand = pop_operand_byte(cpu);
	push_operand_byte(operand, cpu);
	push_operand_byte(operand, cpu);
}

void dup_halfword(cpu_t *cpu)
{
	halfword_t operand;

	operand = pop_operand_halfword(cpu);
	push_operand_halfword(operand, cpu);
	push_operand_halfword(operand, cpu);
}

void dup_word(cpu_t *cpu)
{
	word_t operand;

	operand = pop_operand_word(cpu);
	push_operand_word(operand, cpu);
	push_operand_word(operand, cpu);
}

void abs_short(cpu_t *cpu)
{
	halfword_t operand, new_stack_value;
	uint16_t abs_data;
	bool has_overflowed;

	operand = pop_operand_halfword(cpu);
	abs_data = get_data_from_halfword(operand);
	if (abs_data >> 15)
	{
		abs_data = (uint16_t)(~abs_data + 1);
	}

	new_stack_value = put_data_into_halfword(abs_data);
	copy_halfword_flags(&operand, &new_stack_value);

	has_overflowed = abs_data >> 15;
	if (has_overflowed)
	{
		set_flag_halfword(&new_stack_value, 0);
	}

	push_operand_halfword(new_stack_value, cpu);
}

void abs_long(cpu_t *cpu)
{
	word_t operand, new_stack_value;
	uint32_t abs_data;
	bool has_overflowed;

	operand = pop_operand_word(cpu);
	abs_data = get_data_from_word(operand);
	if (abs_data >> 31)
	{
		abs_data = ~abs_data + 1;
	}

	new_stack_value = put_data_into_word(abs_data);
	copy_word_flags(&operand, &new_stack_value);

	has_overflowed = abs_data >> 31;
	if (has_overflowed)
	{
		set_flag_word(&new_stack_value, 0);
	}

	push_operand_word(new_stack_value, cpu);
}

void hcf(cpu_t *cpu, byte_t opcode)
{
	size_t i;
	uint16_t temp16;
	uint8_t temp8;
	printf("HCF Instruction caught!\n");
	printf("Illegal Opcode: %hhu%02hhX\n", get_flag_from_byte(opcode),
				get_data_from_byte(opcode));
	for(i = 0; i < 14; i++) {
		printf("Pointer Register: %zu\n", i);
		temp16 = get_data_from_halfword(cpu->pr[i].pointer_link);
		printf("\tPointer Link: %hX\n", temp16);
		temp16 = get_data_from_halfword(cpu->pr[i].pointer_value);
		printf("\tPointer Value: %hX\n", temp16);
		printf("\tFlags: ");
		printf("%hu", get_flag_from_halfword(cpu->pr[i].pointer_link, 1));
		printf("%hu", get_flag_from_halfword(cpu->pr[i].pointer_link, 0));
		printf("%hu", get_flag_from_halfword(cpu->pr[i].pointer_value, 1));
		printf("%hu\n\n", get_flag_from_halfword(cpu->pr[i].pointer_value, 0));
	}

	for(i = 0; i < 6; i++) {
		printf("Base Register: %zu\n", i);
		temp8 = get_data_from_byte(cpu->br[i].bounds);
		printf("\tBase Bounds: %hhX\n", temp8);
		temp16 = get_data_from_halfword(cpu->br[i].start_page);
		printf("\tBase Start Page: %hX\n", temp16);
		printf("\tFlags: ");
		printf("%hhu", get_flag_from_byte(cpu->br[i].zero));
		printf("%hhu", get_flag_from_byte(cpu->br[i].bounds));
		printf("%hu", get_flag_from_halfword(cpu->br[i].start_page, 1));
		printf("%hu\n\n", get_flag_from_halfword(cpu->br[i].start_page, 0));
	}

	printf("ASF Register\n");
	temp16 = get_data_from_halfword(cpu->pr_14.consecutive_storage_link);
	printf("\tConsecutive Storage Link: %hX\n", temp16);
	temp16 = get_data_from_halfword(cpu->pr_14.free_list_link);
	printf("\tFree List Link: %hX\n\n", temp16);

	printf("Stack values (top to bottom)\n");
	printf("\t%hhX\n",   get_data_from_byte(pop_operand_byte(cpu)));
	printf("\t%hhX\n",   get_data_from_byte(pop_operand_byte(cpu)));
	printf("\t%hhX\n",   get_data_from_byte(pop_operand_byte(cpu)));
	printf("\t%hhX\n",   get_data_from_byte(pop_operand_byte(cpu)));
	printf("\t%hhX\n",   get_data_from_byte(pop_operand_byte(cpu)));
	printf("\t%hhX\n\n", get_data_from_byte(pop_operand_byte(cpu)));
	exit(EXIT_FAILURE);
}

void execute(byte_t opcode, cpu_t *cpu)
{
	uint16_t new_pointer_value;
	uint16_t old_pointer_value = get_data_from_halfword(cpu->pr[0].pointer_value);

	if (get_flag_from_byte(opcode))
	{
		switch (opcode.data)
		{
			case b(10010100):
				abs_short(cpu);
				new_pointer_value = (uint16_t)(old_pointer_value + 1);
				break;
			case b(10010101):
				abs_long(cpu);
				new_pointer_value = (uint16_t)(old_pointer_value + 1);
				break;
			default:
				hcf(cpu, opcode);
				new_pointer_value = 0;
				break;
		}
	}
	else
	{
		switch (opcode.data)
		{
			case b(00100100):
				dup_byte(cpu);
				break;
			case b(00100101):
				dup_halfword(cpu);
				break;
			case b(00100110):
				dup_word(cpu);
				break;
			default:
				hcf(cpu, opcode);
				new_pointer_value = 0;
				break;
		}
	}

	cpu->pr[0].pointer_value = put_data_into_halfword(new_pointer_value);
}

static
void load_basic_program_1(cpu_t *cpu)
{
	/* Program 1 does the following
				* Load abs_long into 0x0
				* Set pointer link of pr[0] to 0x100
				* Put PR contents into 0x100 with pointer value = 0x1
				* Put HCF into 0x1
				* Set Stack pointer (pr[13]) to 0x1000
				* Load Stack contents
	*/
	raw_address_t addr;
	byte_t opcode;
	halfword_t pointer_link_0;
	halfword_t pointer_link_1;
	halfword_t pointer_value;
	nf_t next_instruction_pr;
	
	/* Set pr[13] to 0x1000 */
	cpu->pr[13].pointer_value = put_data_into_halfword(0x1000);
	
	/* Put abs_long into 0x0 */
	addr = get_address_from_pointer(&(cpu->pr[0]), cpu);
	opcode = put_data_into_byte(b(10010101));
	//opcode = put_data_into_byte(0);
	set_flag_byte(&opcode);
	put_byte_into_memory(opcode, addr);

	/* Set pointer link of pr[0] */
	pointer_link_0 = put_data_into_halfword(0x100);
	cpu->pr[0].pointer_link = pointer_link_0;
	
	/* Put PR into 0x100 with pointer value 0x1 */
	pointer_link_1 = put_data_into_halfword(0x104);
	/* Note that there's nothing actually at 0x104, but that's
					ok because of the fetch decode execute order */
	pointer_value = put_data_into_halfword(1);
	/* 0x1 -- pointer to an HCF instruction */
	addr = get_address_from_link(&(cpu->pr[0]), cpu);
	put_halfword_into_memory(pointer_link_1, addr);
	put_halfword_into_memory(pointer_value, addr + 2);

	/* Put HCF into 0x1 */
	opcode = put_data_into_byte(0);
	next_instruction_pr.pointer_value = pointer_value;
	next_instruction_pr.pointer_link = put_data_into_halfword(0);
	addr = get_address_from_pointer(&next_instruction_pr, cpu);
	put_byte_into_memory(opcode, addr);

	/*  Finally, load a thing into the stack. */
	/* We will put in '5' */
	addr = get_address_from_pointer(&(cpu->pr[13]), cpu);
	push_operand_word(put_data_into_word(-5), cpu);
}

void cpu_ctor(cpu_t *cpu)
{
	memset(cpu, 0, sizeof(*cpu));
}

int main(void)
{
	cpu_t cpu;

	cpu_ctor(&cpu);
	core_memory = (byte_t *)malloc(sizeof(byte_t [1 << 24]));
	if (core_memory == NULL)
	{
		perror("error allocating core_memory");
		exit(EXIT_FAILURE);
	}
	load_basic_program_1(&cpu);
	for (;;)
	{
		raw_address_t instruction = get_address_from_pointer(&(cpu.pr[0]), &cpu);

		byte_t opcode = get_byte_from_memory(instruction);
		execute(opcode, &cpu);
	}
	free(core_memory);
	return EXIT_SUCCESS;
}
