LIST=$(shell find -type d -mindepth 1 -maxdepth 1)

.PHONY:	all clean

all:
	for d in $(LIST); do make -C $$d $@; done

clean:
	for d in $(LIST); do make -C $$d $@; done
