#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

const uint8_t assign_program[] = {
		0x00, 0x01, // assign
		0x21, 0x00, // R2
		0x11, 0x01, 0xDE, 0x00, 0xAD, 0x00, // R1
		0x64, 0x00 // HCF
};

int main(void)
{
	FILE *fd = fopen("assign.illiac", "w");
	fwrite(assign_program, sizeof(assign_program), sizeof(uint8_t), fd);
	fclose(fd);
	return 0;
}
