#!/usr/bin/perl

$module="gtkmm";
$path="gtk--";
$debug=0;
%object = ();
%namespace = ();

while ($ARGV[0]=~/^-/)
  {
   if ($ARGV[0]=~/--module=(\S+)/) {$module="$1";}
   elsif ($ARGV[0]=~/--debug/) {$debug=1;}
   elsif ($ARGV[0]=~/--path=(\S+)/) {$path="$1";}
   else {print "Error: unknow option $ARGV[0]\n"; exit;}

   shift @ARGV;
  }

while ($ARGV[0])
  {
    
    if ($debug) {warn "Processing file : $ARGV[0]\n";}

    open FILE,$ARGV[0] or die "Couldn't open file $ARGV[0] : $!\n";
    $file=$ARGV[0];
    $file=~s/.*\/([^\/]+)$/$1/;
    $file=~s/\.gen_h//;
    while (<FILE>) 
      {
	if (/CLASS_START\((\w+)\)/)
	  {
	    $tmpnamespace = $1;
	  }
	elsif (/WRAP_CLASS\s*\(/)
	  {
	    $line = $_;
	    while ($line !~ /;/ && ($_ = <FILE>))
	      {
		$line.=$_;
	      }

	    $line=~s/^.*WRAP_CLASS\s*\(//;
	    $line=~s/\s+//g;
	    ($cppname,$cname)=split(/,/,$line);
	    $object {$cppname}=$cname;
	    $namespace {$cppname} = $tmpnamespace;
	  }
      }
    shift @ARGV;
    close(FILE);
  }

print "#include <gtk--/object.h>\n";

# Here we have to be subtle. The Gtk-- objects are easy, they all go
# into the Gtk namespace. But in Gnome--, some go into the Gnome
# namespace (most of them), and some into the Gtk one (because the
# corresponding widget is Gtk-prefixed, e.g. GtkTed, GtkDial, etc...

# First, the Gtk namespace

foreach $i (sort keys %object)
  {
      print "namespace $namespace{$i} {class ${i}_Class { public: static Gtk::Object* wrap_new(GtkObject*); };}\n";

  }


print "void ${module}_wrap_init()\n  {\n";

# Generate module_wrap_init() body
#
foreach $i (sort keys %object)
  {
    print "    Gtk::wrap_register(\"$object{$i}\",\&$namespace{$i}::${i}_Class::wrap_new);\n";
  }
print "  }\n";



