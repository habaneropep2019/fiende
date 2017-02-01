#!/bin/sh
cat > /tmp/.$$.html
lynx -dump /tmp/.$$.html | sed -e "s,tmp/.$$,THISFILE,g"
rm -f /tmp/.$$.html
