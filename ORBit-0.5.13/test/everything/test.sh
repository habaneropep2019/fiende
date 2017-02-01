#! /bin/sh

# This is a generic script for firing up a server, waiting for it to write
# its stringified IOR to a file, then firing up a server

./server&

until test -s iorfile; do sleep 1; done

if ./client; then
	kill $!
	rm iorfile
else
	kill $!
	rm iorfile
	exit 1
fi
