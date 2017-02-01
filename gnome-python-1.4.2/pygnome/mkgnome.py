import sys

srcdir = '.'
if len(sys.argv) > 1:
    [srcdir] = sys.argv[1:]

sys.path.insert(0, srcdir + "/../pygtk/generate")
import generate

print "Reading gtk's object and enum definitions ...",
sys.stdout.flush()
generate.TypesParser(srcdir + '/../pygtk/generate/gtk.defs').startParsing()
print "done"

print "Outputing zvt stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/zvt.defs',
			     prefix='zvtmodule',
			     typeprefix='&')
p.addExcludeFile('generate/zvt.ignore')
p.startParsing()
print "done"

print "Outputing xmhtml stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/xmhtml.defs',
			     prefix='gtkxmhtmlmodule',
			     typeprefix='&')
p.addExcludeFile('generate/xmhtml.ignore')
p.startParsing()
print "done"

print "Outputing gnome stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/gnome.defs',
			     prefix='gnomemodule')
p.addExcludeFile('generate/gnome.ignore')
p.startParsing()
print "done"

print "Outputing gnomeui stuff ...",
sys.stdout.flush()
generate.boxed['GdkImlibImage'] = ('PyGdkImlibImage_Type',
				   'PyGdkImlibImage_Get',
				   'PyGdkImlibImage_New')
p = generate.FilteringParser(input=srcdir + '/generate/gnomeui.defs',
			     prefix='gnomeuimodule',
			     typeprefix='&')
p.addExcludeFile('generate/gnomeui.ignore')
p.startParsing()
print "done"

print "Outputing applet stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/applet.defs',
			     prefix='appletmodule',
			     typeprefix='&')
p.startParsing()
print "done"

print "Outputing capplet stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/capplet.defs',
			     prefix='cappletmodule',
			     typeprefix='&')
p.startParsing()
print "done"

print "Outputing gtkhtml stuff ...",
sys.stdout.flush()
p = generate.FilteringParser(input=srcdir + '/generate/gtkhtml.defs',
			     prefix='gtkhtml',
			     typeprefix='&')
p.addExcludeFile('generate/gtkhtml.ignore')
p.startParsing()
print "done"
