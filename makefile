SUBDIRS = $(wildcard */.) 

$(info $(SUBDIRS))

all: $(SUBDIRS) post

$(SUBDIRS):
	make -C $@		

.PHONY: $(SUBDIRS)
