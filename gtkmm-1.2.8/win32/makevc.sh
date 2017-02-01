#!/bin/sh

# this hacks the gtkmm dist to make a version usable by Microsoft VC++ 6.0

if test ! -r config.status || test ! -r gtk--config.h ; then
  echo "Package must be configured"
  exit -1
fi

VERSION=`grep GTKMM_VERSION config.status |awk -F% '{print $3}'`
PACKAGE="gtkmm-$VERSION"
TARBALL="gtkmm-$VERSION.tar.gz"

if test ! -r $TARBALL ; then
  echo "Can't find $TARBALL"
  echo -1
fi

echo "Building VC++ dist for $VERSION"
if test -d $PACKAGE ; then
  rm -Rf $PACKAGE
fi

echo "> Unpack distribution"
tar xzvf $TARBALL

cd $PACKAGE 

echo "> Configure"
./configure

echo "> Build code generator"
(cd src/gtkmmproc; make)

echo "> Build sources"
(cd src/build_sources; make)

echo "> Rearrange files"
mv src/gtk-- src/gtk--.h src/glib--.h .
mv gdk-- gdkmm
mv gdkmm/gdk--.h gdkmm/gdk-- .
mv win32/*.dsp win32/*.dsw .

echo "> Rename extensions"
CCFILES=`find $package -name "*.cc"`
for i in $CCFILES ;
do
base=`echo $i | sed 's/\.cc$/.cpp/'`
echo "$i"
mv -f $i $base
done

echo "> Remove excess files"
rm -Rf docs docgen scripts gdkmm src win32
rm -f config* gtkmm-config* gtkmmconvert libtool 
rm -f stamp* *.spec* ac* *.m4
find . -name "Makefile*" | xargs rm

cd ..

echo "> Repacking"
find $PACKAGE | zip -@ gtkmm-visual-$VERSION.zip
rm -Rf $PACKAGE
chmod a+r gtkmm-visual-$VERSION.zip
