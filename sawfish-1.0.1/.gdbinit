# $Id: .gdbinit,v 1.1 1999/07/25 15:04:00 john Exp $

# prints $$ to standard output
define v
call rep_print_val(Fstdout_file(), $)
call rep_stream_putc(Fstdout_file(), '\n')
end

# prints the lisp backtrace
define lbt
call Fbacktrace(Fstdout_file())
call rep_stream_putc(Fstdout_file(), '\n')
end
