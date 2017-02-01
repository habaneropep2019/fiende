<sect>Tips For Writing Gtk-- Applications (draft)
<!-- ***************************************************************** -->
<p>
This section is simply a gathering of wisdom, general style guidelines
and hints to creating good Gtk-- applications. Currently this section
is very short, but hopefully it will get longer in future editions of
this tutorial.

Use GNU autoconf and automake! They are your friends :) Automake
examines C files, determines how they depend on each other, and
generates a Makefile so the files can be compiled in the correct
order. Autoconf permits automatic configuration of software
installation, handling a large number of system quirks to increase
portability. I am planning to make a quick intro on them here.

When writing C code, use only C comments (beginning with "/*" and
ending with "*/"), and don't use C++-style comments ("//").  Although
many C compilers understand C++ comments, others don't, and the ANSI C
standard does not require that C++-style comments be processed as
comments.

<!-- ***************************************************************** -->
