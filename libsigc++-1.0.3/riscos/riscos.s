case "$1" in
   clean)
       echo Removing all files generated for RISC OS release
       rm -rf \!LibSig stype,feb
       exit 0
esac

LIBSIGC="!LibSig"

echo Setting up directories...
if test ! -d $LIBSIGC; then
  mkdir $LIBSIGC
  mkdir $LIBSIGC/o
  mkdir $LIBSIGC/cc
  mkdir $LIBSIGC/h
  mkdir $LIBSIGC/doc
fi

echo Inserting RISCOS dependent files...

cat > $LIBSIGC/\!Boot << EOF
Set SigC$Path <Obey\$Dir>.
IconSprites <Obey\$Dir>.!Sprites
EOF

cat > $LIBSIGC/\!Run << EOF
Set SigC\$Path <Obey\$Dir>. 
Filer_OpenDir SigC:h
IconSprites <Obey\$Dir>.!Sprites
EOF

cat > stype,feb << EOF
| Set the type of this file to Obey (FEB) and make sure it's in the same directory as !LibSig, then run it
SetType <Obey\$Dir>.!LibSig.!Boot Obey
SetType <Obey\$Dir>.!LibSig.!Run Obey
EOF

cp sprites $LIBSIGC/\!Sprites && echo 'SetType <Obey$Dir>.!LibSig.!Sprites Sprite' >> stype,feb
cp sprites22 $LIBSIGC/\!Sprites22 && echo 'SetType <Obey$Dir>.!LibSig.!Sprites22 Sprite' >> stype,feb
cp ../doc/riscos $LIBSIGC/README && echo 'SetType <Obey$Dir>.!LibSig.README Text' >> stype,feb
cp ../COPYING $LIBSIGC/GPL && echo 'SetType <Obey$Dir>.!LibSig.GPL Text' >> stype,feb
cp ../COPYING.LIB $LIBSIGC/LGPL && echo 'SetType <Obey$Dir>.!LibSig.LGPL Text' >> stype,feb
cp ../AUTHORS $LIBSIGC/AUTHOR && echo 'SetType <Obey$Dir>.!LibSig.AUTHOR Text' >> stype,feb
cp ../FEATURES $LIBSIGC/FEATURES && echo 'SetType <Obey$Dir>.!LibSig.FEATURES Text' >> stype,feb

cat >> stype,feb << EOF
Repeat SetType <Obey\$Dir>.!LibSig.cc -files text
Repeat SetType <Obey\$Dir>.!LibSig.h -files text
Repeat SetType <Obey\$Dir>.!LibSig.doc -files text
EOF

chmod -R a+x $LIBSIGC/* stype,feb

echo Arranging cc files...
for i in ../sigc++/*.cc; 
do 
   sed '/^#include <sigc++\/.*>/{s/<sigc++\/*/"sigc:/;s/>/"/;}' $i > $LIBSIGC/cc/`basename $i .cc`
done

echo Arranging h files...
for i in ../sigc++/*.h;
do
   sed '/^#include <sigc++.*>/{s/<sigc++\/*/"sigc:/;s/>/"/;}' $i > $LIBSIGC/h/`basename $i .h`
done
cp sigc++config $LIBSIGC/h/config


echo Copying docs \& examples
cp -r ../doc $LIBSIGC/

echo Done.

