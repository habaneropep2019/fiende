"""This module allows python programs to use GNU gettext message catalogs.

Author: James Henstridge <james@daa.com.au>
(This is loosely based on gettext.pl in the GNU gettext distribution)

The best way to use it is like so:
    import gettext
    gettext.bindtextdomain(PACKAGE, LOCALEDIR)
    gettext.textdomain(PACKAGE)
    _ = gettext.gettext
    print _('Hello World')

where PACKAGE is the domain for this package, and LOCALEDIR is usually
'$prefix/share/locale' where $prefix is the install prefix.

If you have more than one catalog to use, you can directly create catalog
objects.  These objects are created as so:
    import gettext
    cat = gettext.Catalog(PACKAGE, localedir=LOCALEDIR)
    _ = cat.gettext
    print _('Hello World')

The catalog object can also be accessed as a dictionary (ie cat['hello']).

There are also some experimental features.  You can add to the catalog, just
as you would with a normal dictionary.  When you are finished, you can call
its save method, which will create a new .mo file containing all the
translations:
    import gettext
    cat = Catalog()
    cat['Hello'] = 'konichiwa'
    cat.save('./tmp.mo')

Once you have written an internationalized program, you can create a .po file
for it with "xgettext --keyword=_ fillename ...".  Then do the translation and
compile it into a .mo file, ready for use with this module.  Note that you
will have to use C style strings (ie. use double quotes) for proper string
extraction.
"""
import os, string

prefix = '/usr/local'
localedir = prefix + '/share/locale'

_aliases = {}
def _unalias_lang(lang):
    if not _aliases:
	for file in [ '/usr/share/locale/locale.alias',
		      '/usr/local/share/locale/locale.alias',
		      '/usr/lib/X11/locale/locale.alias',
		      '/usr/openwin/lib/locale/locale.alias' ]:
	    try:
		fp = open(file, 'r')
		line = fp.readline()
		while line:
		    line = string.strip(line)
		    if line and line[0] != '#':
			p = string.split(line)
			if len(p) >= 2 and not _aliases.has_key(p[0]):
			    _aliases[p[0]] = p[1]
		    line = fp.readline()
	    except:
		pass
    while _aliases.has_key(lang):
	lang = _aliases[lang]
    return lang

def _expand_lang(locale):
    locale = _unalias_lang(locale)
    COMPONENT_CODESET   = 1 << 0
    COMPONENT_TERRITORY = 1 << 1
    COMPONENT_MODIFIER  = 1 << 2
    # split up the locale into its base components
    mask = 0
    pos = string.find(locale, '@')
    if pos >= 0:
	modifier = locale[pos:]
	locale = locale[:pos]
	mask = mask | COMPONENT_MODIFIER
    else:
	modifier = ''
    pos = string.find(locale, '.')
    if pos >= 0:
	codeset = locale[pos:]
	locale = locale[:pos]
	mask = mask | COMPONENT_CODESET
    else:
	codeset = ''
    pos = string.find(locale, '_')
    if pos >= 0:
	territory = locale[pos:]
	locale = locale[:pos]
	mask = mask | COMPONENT_TERRITORY
    else:
	territory = ''
    language = locale

    ret = []
    for i in range(mask+1):
	if not (i & ~mask):  # if all components for this combo exist ...
	    val = language
	    if i & COMPONENT_TERRITORY: val = val + territory
	    if i & COMPONENT_CODESET:   val = val + codeset
	    if i & COMPONENT_MODIFIER:  val = val + modifier
	    ret.insert(0, val)
    return ret

lang = []
for env in 'LANGUAGE', 'LC_ALL', 'LC_MESSAGES', 'LANG':
	if os.environ.has_key(env):
		lang = string.split(os.environ[env], ':')
		lang = map(_expand_lang, lang)
		lang = reduce(lambda a, b: a + b, lang)
		break
if 'C' not in lang:
	lang.append('C')

# remove duplicates
i = len(lang) - 1
while i >= 0:
    if lang[i] in lang[:i]:
	del lang[i]
    i = i - 1
del i

error = 'gettext.error'

def _lsbStrToInt(str):
    return ord(str[0]) + \
	   (ord(str[1]) << 8) + \
	   (ord(str[2]) << 16) + \
	   (ord(str[3]) << 24)
def _msbStrToInt(str):
    return (ord(str[0]) << 24) + \
	   (ord(str[1]) << 16) + \
	   (ord(str[2]) << 8) + \
	   ord(str[3])
def _intToLsbStr(int):
    return chr(int         & 0xff) + \
	   chr((int >> 8)  & 0xff) + \
	   chr((int >> 16) & 0xff) + \
	   chr((int >> 24) & 0xff)

class Catalog:
    def __init__(self, domain=None, localedir=localedir):
	self.domain = domain
	self.localedir = localedir
	self.cat = {}
	if not domain: return
	for self.lang in lang:
	    if self.lang == 'C':
		return
	    catalog = "%s/%s/LC_MESSAGES/%s.mo" % (localedir,self.lang,domain)
	    try:
		f = open(catalog, "rb")
		buffer = f.read()
		del f
		break
	    except IOError:
		pass
	else:
	    return # assume C locale

	strToInt = _lsbStrToInt
	if strToInt(buffer[:4]) != 0x950412de:
	    # catalog is encoded with MSB offsets.
	    strToInt = _msbStrToInt
	    if strToInt(buffer[:4]) != 0x950412de:
		# magic number doesn't match
		raise error, 'Bad magic number in %s' % (catalog,)

	self.revision = strToInt(buffer[4:8])
	nstrings = strToInt(buffer[8:12])
	origTabOffset  = strToInt(buffer[12:16])
	transTabOffset = strToInt(buffer[16:20])
	for i in range(nstrings):
	    origLength = strToInt(buffer[origTabOffset:
					 origTabOffset+4])
	    origOffset = strToInt(buffer[origTabOffset+4:
					 origTabOffset+8])
	    origTabOffset = origTabOffset + 8
	    origStr = buffer[origOffset:origOffset+origLength]
		
	    transLength = strToInt(buffer[transTabOffset:
					  transTabOffset+4])
	    transOffset = strToInt(buffer[transTabOffset+4:
					  transTabOffset+8])
	    transTabOffset = transTabOffset + 8
	    transStr = buffer[transOffset:transOffset+transLength]
	    self.cat[origStr] = transStr

    def gettext(self, string):
	"""Get the translation of a given string"""
	if self.cat.has_key(string):
	    return self.cat[string]
	else:
	    return string
    # allow catalog access as cat(str) and cat[str] and cat.gettext(str)
    __getitem__ = gettext
    __call__ = gettext

    # this is experimental code for producing mo files from Catalog objects
    def __setitem__(self, string, trans):
	"""Set the translation of a given string"""
	self.cat[string] = trans
    def save(self, file):
	"""Create a .mo file from a Catalog object"""
	try:
	    f = open(file, "wb")
	except IOError:
	    raise error, "can't open " + file + " for writing"
	f.write(_intToLsbStr(0x950412de))    # magic number
	f.write(_intToLsbStr(0))             # revision
	f.write(_intToLsbStr(len(self.cat))) # nstrings

	oIndex = []; oData = ''
	tIndex = []; tData = ''
	for orig, trans in self.cat.items():
	    oIndex.append((len(orig), len(oData)))
	    oData = oData + orig + '\0'
	    tIndex.append((len(trans), len(tData)))
	    tData = tData + trans + '\0'
	oIndexOfs = 20
	tIndexOfs = oIndexOfs + 8 * len(oIndex)
	oDataOfs = tIndexOfs + 8 * len(tIndex)
	tDataOfs = oDataOfs + len(oData)
	f.write(_intToLsbStr(oIndexOfs))
	f.write(_intToLsbStr(tIndexOfs))
	for length, offset in oIndex:
	    f.write(_intToLsbStr(length))
	    f.write(_intToLsbStr(offset + oDataOfs))
	for length, offset in tIndex:
	    f.write(_intToLsbStr(length))
	    f.write(_intToLsbStr(offset + tDataOfs))
	f.write(oData)
	f.write(tData)

_cat = None
_cats = {}

def bindtextdomain(domain, localedir=localedir):
    global _cat
    if not _cats.has_key(domain):
	_cats[domain] = Catalog(domain, localedir)
    if not _cat: _cat = _cats[domain]

def textdomain(domain):
    global _cat
    if not _cats.has_key(domain):
	_cats[domain] = Catalog(domain)
    _cat = _cats[domain]

def gettext(string):
    if _cat == None: raise error, "No catalog loaded"
    return _cat.gettext(string)

_ = gettext

def dgettext(domain, string):
    if domain is None:
	return gettext(string)
    if not _cats.has_key(domain):
	bindtextdomain(domain)
    return _cats[domain].gettext(string)

def test():
    import sys
    global localedir
    if len(sys.argv) not in (2, 3):
	print "Usage: %s DOMAIN [LOCALEDIR]" % (sys.argv[0],)
	sys.exit(1)
    domain = sys.argv[1]
    if len(sys.argv) == 3:
	bindtextdomain(domain, sys.argv[2])
    textdomain(domain)
    info = gettext('')  # this is where special info is often stored
    if info:
	print "Info for domain %s, lang %s." % (domain, _cat.lang)
	print info
    else:
	print "No info given in mo file."

if __name__ == '__main__':
    test()
