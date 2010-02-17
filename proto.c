#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t data;
	bool flag;
} byte_t;

typedef struct {
	byte_t low;
	byte_t high;
} half_word_t;
typedef half_word_t hw_t;

typedef struct {
	half_word_t pointer_link;
	half_word_t pointer_value;
} number_format_t;
typedef number_format_t nf_t;

typedef struct {
	hw_t consecutive_storage_link;
	hw_t free_list_link;
} available_space_format_t;
typedef available_space_format_t asf_t;

typedef struct {
	byte_t zero;
	byte_t bounds;
	hw_t start_page;
} base_pointer_t;
	

typedef struct {
	number_format_t pr_0;
	number_format_t pr_1;
	number_format_t pr_2;
	number_format_t pr_3;
	number_format_t pr_4;
	number_format_t pr_5;
	number_format_t pr_6;
	number_format_t pr_7;
	number_format_t pr_8;
	number_format_t pr_9;
	number_format_t pr_10;
	number_format_t pr_11;
	number_format_t pr_12;
	number_format_t pr_13;
	base_pointer_t br_0;
	base_pointer_t br_1;
	base_pointer_t br_2;
	base_pointer_t br_3;
	asf_t pr_14;
} cpu_t;

static void
exec(nf_t instruction_word) {
	return;
}

int main(void) {
	cpu_t cpu;
	for (;;) {
		nf_t current_instruction = cpu.pr_0;
		cpu.pr_0 = mem(cpu.pr_0.pointer_link);
		exec(mem(current_instruction.pointer_value));
	}
	return 0;
}
