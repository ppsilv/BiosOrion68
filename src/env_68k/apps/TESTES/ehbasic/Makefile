#all: ehbasic.bin ehbasic.hex ehbasic_even.bin ehbasic_odd.bin
all: ehbasic.bin

ehbasic.hex: ehbasic.run
	srec_cat ehbasic.run -Motorola -output ehbasic.hex -Intel

ehbasic.bin: ehbasic.inc ehbasic.s
	vasmm68k_mot -quiet -nowarn=2028 -m68000 -Felf -L ehbasic.lst -o ehbasic.elf ehbasic.s

ehbasic.run: ehbasic.inc ehbasic.asm
	#vasmm68k_mot -quiet -nowarn=2028 -m68000 -Fsrec -s19 -L ehbasic.lst -o ehbasic.run ehbasic.s
	vasmm68k_mot -quiet -nowarn=2028 -m68000 -Fsrec -s28 -L ehbasic.lst -o ehbasic.run ehbasic.s
	# Use lines below when building for ROM
	srec_cat ehbasic.run -fill 0x00 0xc000 0xffff --crop 0xc000 0xffff -Output_Block_Size 16 -o temp
	mv temp ehbasic.run
	# Use lines below when building for RAM
	#srec_cat ehbasic.run -fill 0x00 0x800 0x3fff --crop 0x0800 0x3fff -Output_Block_Size 16 -o temp
	#mv temp ehbasic.run

ehbasic_even.run: ehbasic.run
	srec_cat ehbasic.run -split 2 0 -offset 0x6000 -o ehbasic_even.run

ehbasic_odd.run: ehbasic.run
	srec_cat ehbasic.run -split 2 1 -offset 0x6000 -o ehbasic_odd.run

upload: ehbasic.run
	/bin/echo -e "LO1\r\n" >/dev/ttyUSB0
	sleep 1
	ascii-xfr -s -l 50 ehbasic.run | sed  's/S5/S9/' > /dev/ttyUSB0
	/bin/echo -e "GO 800\r\n" >/dev/ttyUSB0

ehbasic_odd.bin: ehbasic_odd.run
	srec2bin -o 0xC001 ehbasic_odd.run ehbasic_odd.bin

ehbasic_even.bin: ehbasic_even.run
	srec2bin -o 0xC000 ehbasic_even.run ehbasic_even.bin

# Program EEPROMS using Minipro programmer.
# Adjust device type below to match your actual device.
program_odd: ehbasic_odd.bin
	minipro -S --write=ehbasic_odd.bin --device=AT28HC64

program_even: ehbasic_even.bin
	minipro -S --write=ehbasic_even.bin --device=AT28HC64

clean:
	$(RM) ehbasic.run ehbasic_even.run ehbasic_odd.run ehbasic.lst ehbasic.hex
	$(RM) ehbasic_even.bin ehbasic_odd.bin ehbasic.bin ehbasic.elf
distclean:
	$(RM) ehbasic.run ehbasic_even.run ehbasic_odd.run ehbasic.lst ehbasic.hex
	$(RM) ehbasic_even.bin ehbasic_odd.bin
