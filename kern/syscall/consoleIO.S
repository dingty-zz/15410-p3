

.global		getchar
.global		readline
.global		print
.global		set_term_color
.global		get_cursor_pos
.global		set_cursor_pos

.extern		_getchar
.extern		_readline
.extern		_print
.extern		_set_term_color
.extern		_get_cursor_pos
.extern		_set_cursor_pos

getchar:
	pusha
	pushl	%esi
	call _getchar
	popl	%esi
	popa
	iret

