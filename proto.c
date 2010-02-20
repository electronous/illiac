#include <stdint.h>
#include <stdbool.h>

byte_t *memory;

typedef struct
{
	uint8_t data;
	bool flag;
} byte_t;

typedef struct
{
	byte_t low;
	byte_t high;
} half_word_t;
typedef half_word_t hw_t;

typedef struct
{
	half_word_t pointer_link;
	half_word_t pointer_value;
} number_format_t;
typedef number_format_t nf_t;

typedef struct
{
	hw_t consecutive_storage_link;
	hw_t free_list_link;
} available_space_format_t;
typedef available_space_format_t asf_t;

typedef struct
{
	byte_t zero;
	byte_t bounds;
	hw_t start_page;
} base_pointer_t;

typedef int raw_address_t;

typedef struct
{
	number_format_t pr[14];
	base_pointer_t br[6];
	asf_t pr_14;
} cpu_t;

static void
exec(nf_t instruction_word)
{
	return;
}

raw_address_t get_address_common(number_format_t *p, cpu_t *cpu)
{
	uint8_t br_num;
	br_num = (p->pointer_link.low.flag << 2);
	br_num += (p->pointer_value.high.flag << 1);
	br_num += (p->pointer_value.low.flag);

	return (raw_address_t)(cpu->br[br_num].start_page.data << 8);

}

raw_address_t get_address_from_pointer(number_format_t *p, cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret += (p->pointer_value.high.data << 8);
	ret += (p->pointer_value.low.data);

	return ret;
}

raw_address_t get_address_from_link(number_format_t *p, cpu_t *cpu)
{
	raw_address_t ret;
	ret = get_address_common(p, cpu);
	ret += (p->pointer_link.high.data << 8);
	ret += (p->pointer_link.low.data);

	return ret;
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
		raw_address_t next;
		nf_t current_instruction = cpu.pr[0];
		if ((next = get_address_from_link(&(cpu.pr[0]), &cpu) == -1))
		{
			printf("Segfault.");
		}
		cpu.pr[0] = memory[next];
		exec(mem(current_instruction.pointer_value));
	}
	return 0;
}
