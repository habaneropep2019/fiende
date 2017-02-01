<!doctype linuxdoc system>

<!-- This is the tutorial marked up in SGML
     (just to show how to write a comment)

dnl  We are going to use M4 to do text manipulation here
dnl  The next lines are directives to M4.
define(EXAMPLE_DIR,../../examples)
include(sgml.macros.m4)
-->

<article>
<title>GTK-- v1.2 Tutorial
<author>
by Guillaume Laurent <htmlurl url="mailto:glaurent@worldnet.fr"
			      name="&lt;glaurent@worldnet.fr&gt;">,
Karl Nelson <htmlurl url="mailto:kenelson@ece.ucdavis.edu"
name="&lt;kenelson@ece.ucdavis.edu&gt;"> and
Michael Ashton <htmlurl url="mailto:data@users.sourceforge.net"
name="&lt;data@users.sourceforge.net&gt;">
Chapter on "Timeouts" contributed by 
Bernhard Rieder
	<htmlurl url="mailto:e9325898@student.tuwien.ac.at"
                     name="&lt;e9325898@student.tuwien.ac.at&gt;">
Adapted from the GTK Tutorial by
Tony Gale <htmlurl url="mailto:gale@gtk.org"
			      name="&lt;gale@gtk.org&gt;">
Ian Main <htmlurl url="mailto:imain@gtk.org"
			      name="&lt;imain@gtk.org&gt;">
<date>August 2000
<abstract>
This is a tutorial on how to use GTK-- (the C++ bindings to GTK).

This tutorial is a work in progress.  Not all of the sections have
been completed; some have not been started.  Some chapters have been
written, but not edited or proofread.  Since they do contain helpful
information, they have been included, and are marked "draft".

We are working hard to make this tutorial
helpful and accurate.  We would very much appreciate any reports of
inaccuracies or other errors in this document.  Contributions are also
most welcome.  Post your suggestions, critiques or addenda to the 
<htmlurl url="http://lists.sourceforge.net/mailman/listinfo/gtkmm-main"
name="GTK-- mailing list">.

-- The GTK-- Development Team

<!-- Table of contents -->
<!-- Older versions of this tutorial did not have a table of contents,
     but the tutorial is now so large that having one is very useful. -->
<toc>

divert(-1)
TODO:

* do a tarball-make function in Makefile (DONE)
* fix screenshots (DONE)
* calendar section

* talk about bind()
* talk about manage()
* delete vs. destroy
* general section on memory management, arg passing in GTK--
* talk about other GTK-- weirdnesses (?)
* talk about strings/STL vs. GTK+ types
* Gnome-- Menu API
* new CList / CTree APIs

Examples
--------

* GtkDial example is broken
* No example for ComboBox
* No example for colorsel (dir is empty)
* rangewidgets doesn't compile (FIXED)
* paned doesn't compile (FIXED)

divert(0)

include(introduction.sgml.m4)

dnl done
include(getting_started.sgml.m4)
include(moving_on.sgml.m4)
include(widget_overview.sgml.m4)
include(button_widgets.sgml.m4)

dnl need to explain bind() somewhere ..
include(adjustments.sgml.m4)

dnl done
include(range_widgets.sgml.m4)

dnl done, except need to add section on calendar widget
include(misc_widgets.sgml.m4)

dnl done
include(container.sgml.m4)
include(singleitem.sgml.m4)
include(packing_widgets.sgml.m4)
include(multiitem.sgml.m4)

dnl working on it, needs to be updated for new API
dnl include(clist_widget.sgml.m4)

dnl working on it - updates from m edwards included
include(tree_widget.sgml.m4)

dnl way incomplete, not updated at all for GTK--
dnl include(menu_widget.sgml.m4)

dnl in review, updates from m edwards included
include(text_widget.sgml.m4)

dnl some incomplete stuff here, probs with big comment at end
dnl include(undocumented.sgml.m4)

dnl way incomplete
dnl include(attributes.sgml.m4)

dnl done
include(timeouts.sgml.m4)

dnl way incomplete (we'll explain "bind" here)
include(advanced_events.sgml.m4)

dnl not updated for GTK--
dnl include(selections.sgml.m4)

dnl need a note in glib about glib vs. STL
dnl include(glib.sgml.m4)

dnl rc_files needs updating for GTK--
dnl include(rc_files.sgml.m4)

dnl not updated for GTK--, this section will be fun :)
dnl include(writing_widgets.sgml.m4)

dnl way incomplete
dnl include(scribble.sgml.m4)

dnl draft
include(memory.sgml.m4)

dnl working on it
include(tips.sgml.m4)

dnl is this stuff still accurate?
include(contributing.sgml.m4)
dnl include(credits.sgml.m4)
include(notice.sgml.m4)

<!-- ***************************************************************** -->
<appendix>
<!-- ***************************************************************** -->

dnl Not up to date for gtk--.
dnl include(gtkmm_signals.sgml.m4)
include(gdk_events.sgml.m4)
include(code_examples.sgml.m4)
dnl include(list_widget.sgml.m4)

</article>
