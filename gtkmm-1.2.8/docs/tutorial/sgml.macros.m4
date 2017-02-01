define(EXAMPLE_DIR,../../examples)
divert(-1)
changecom()
changequote([[,]])

define([[__tlat_cmds__]],[['s/&/\&amp;/g
s/>/\&gt;/g
s/</\&lt;/g']])
define([[example_include]],[[syscmd(sed __tlat_cmds__ EXAMPLE_DIR/$1)]])

define([[example_incl_src]],
[[<em>Source location: examples/$1</em>
<tscreen><verb>
example_include($1)
</verb></tscreen>
]])

divert(0)dnl
