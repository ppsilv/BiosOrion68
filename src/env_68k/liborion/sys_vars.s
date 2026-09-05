| global absolute system variables for BURTE
| declaration of krn_vars

.globl	_hostin, _hostout, _termin, _termout, _io_files
	_io_files =	0x5000	|file **
	_termin	=	0x5004	|file *
	_termout =	0x5008	|file *
	_hostin =	0x500c	|file *
	_hostout =	0x5010	|file *

.globl	_burte_state, _sys_error, _update_state
	_burte_state =	0x5014	| long
	_sys_error =	0x5018	| long
	_update_state =	0x501c	| long

.globl	_main_proc, _num_proc, _current_proc
	_main_proc =	0x5020	| short
	_num_proc =	0x5022	| short
	_current_proc =	0x5024	| short

.globl	_wall_clock, _t2_proc, _t3_proc, _wallclock
	_wall_clock = 	0x5028	| long
	_t2_proc =	0x502c	| PFI
	_t3_proc =	0x5030	| PFI
	_wallclock =    0x5034  | long
