/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <i2c.h>
#include <axp209.h>


static int do_halt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts ("shutdown ...\n");

	udelay (50000);				/* wait 50 ms */

	disable_interrupts();
	axp209_poweroff();
	return 0;
}

U_BOOT_CMD(
	halt, 1, 0,	do_halt,
	"AXP209 PMU Power Off",
	""
);

// mdd size seek addr
static int do_mdd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong offset = 0;
	ulong seek = 64;
	ulong size = 0;
	ulong block = 0;
	char *s;
	char cmd[64];

	/* pre-set offset from CONFIG_SYS_LOAD_ADDR */
	offset = CONFIG_SYS_LOAD_ADDR;

	/* pre-set offset from $loadaddr */
	if ((s = getenv("loadaddr")) != NULL) {
		offset = simple_strtoul(s, NULL, 16);
	}
	if (argc >= 4) {
		offset = simple_strtoul(argv[3], NULL, 16);
	}
	if (argc >= 3) {
		seek = simple_strtoul(argv[3], NULL, 10);
	}
	if (argc >= 2) {
		size = simple_strtoul(argv[1], NULL, 10);
	}

	if (size == 0)
		return CMD_RET_USAGE;

	block = (size + 511) >> 9;
	printf("dump info:\n  size: %d byte %d blk, seek: %d blk, addr: 0x%x\n", size, block, seek, offset);

	// ..mmc..
	sprintf(cmd, "mmc write %x %x %x", offset, seek, block);
	printf("run: %s\n", cmd);
	if (run_command(cmd, flag) < 0){
		return 1;
	}else {
		return 0;
	}
}

U_BOOT_CMD(
	mdd, 4, 0,	do_mdd,
	"Write Memory to MMC",
	"<size> [seek = 64] [addr = $env->loadaddr]"
);



int axp_write(ulong reg, u8 val)
{
	return i2c_write(0x34, reg, 1, &val, 1);
}

int axp_read(ulong reg, u8 *val)
{
	return i2c_read(0x34, reg, 1, val, 1);
}

static int do_axp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong	reg, val;
	u8		value;
	int		ret;
	char	bin[9];
	int		i;

	if (argc < 3)
		return CMD_RET_USAGE;

	reg = simple_strtoul(argv[2],NULL,16);
	if (strcmp(argv[1], "write") == 0){
		if (argc < 4)
			return CMD_RET_USAGE;

		val = simple_strtoul(argv[3],NULL,16);
		value = val & 0xF;

		ret = axp_write(reg, value);
		printf("axp write 0x%02x << 0x%x (Result: %x)\n", reg, value, ret);
	}else {
		ret = axp_read(reg, &value);
		for (i=0; i<8; i++){
			bin[7-i] = ((1 << i) & value) ? '1' : '0';
		}
		bin[8] = NULL;
		printf("axp read 0x%02x = 0x%x [%s] (Result: %x)\n", reg, value, bin, ret);
	}
	return 0;
}

U_BOOT_CMD(
	axp, 4, 0,	do_axp,
	"AXP209 PMU IO Function",
	"<op> <reg> [value]\n"
		"  op: read / write\n"
		"  reg: HEX Value\n"
		"  value: HEX 8bit Value"
);
