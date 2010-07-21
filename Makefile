default: qvm
qvm: build_qvm
so: build_so

ZIP = zip

build_qvm:
	$(MAKE) -C code build_qvm
build_so:
	$(MAKE) -C code build_so

clean:
	$(MAKE) -C code clean
	rm -f wop/*.so wop/*.pk3 wop/vm/*.qvm

pak: qvm
	cd wop && rm -f *.pk3 && $(ZIP) -r pakxx.pk3 vm/* -x \*~
