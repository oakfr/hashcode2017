all: ../build/release/Makefile
	@$(MAKE) -s -C ../build/release

debug: build.debug
profile: build.profile
coverage: build.coverage

build.%: ../build/%/Makefile
	@$(MAKE) -s -C ../build/$* clean
	@$(MAKE) -s -C ../build/$*

../build/%/Makefile:
	mkdir -p ../build
	mkdir -p ../build/$*
	(cd ../build/$* && cmake ../../src -DCMAKE_BUILD_TYPE=$*)

clean: ../build/release/Makefile
	(cd ../build/release && cmake ../../src -DCMAKE_BUILD_TYPE=release)
	@$(MAKE) -s -C ../build/release $@
