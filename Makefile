all:
	@echo "choose a valid target"

.PHONY: release
release:
	@echo "Setting up cmake (release)"
	@python ./cmake/print_cmake_command.py Release
	[ -d release ] || (mkdir release && cd release && eval `python ../cmake/print_cmake_command.py Release`)

.PHONY: relwithdebinfo
relwithdebinfo:
	@echo "Setting up cmake (relwithdebinfo)"
	@python ./cmake/print_cmake_command.py RelWithDebInfo
	[ -d relwithdebinfo ] || (mkdir relwithdebinfo && cd relwithdebinfo && eval `python ../cmake/print_cmake_command.py RelWithDebInfo`)

.PHONY: debug
debug:
	@echo "Setting up cmake (debug)"
	@python ./cmake/print_cmake_command.py Debug
	[ -d debug ] || (mkdir debug && cd debug && eval `python ../cmake/print_cmake_command.py Debug`)

.PHONY: test
test:
	(cd test && nosetests --verbose)

.PHONY: travis_install
travis_install:
	make relwithdebinfo
	(cd relwithdebinfo && make && make install)
	pip install .

.PHONY: travis_script
travis_script:
	(cd relwithdebinfo && CTEST_OUTPUT_ON_FAILURE=true make test)
	(cd test && nosetests --verbose)

.PHONY: lint
lint:
	pyflakes test
	pep8 --filename=*.py --ignore=E265 --exclude=*_pb2.py,vendor microscopes test
	pep8 --filename=*.pyx --ignore=E265,E211,E225 microscopes

.PHONY: clean
clean:
	rm -rf release relwithdebinfo debug microscopes_common.egg-info
	rm -f microscopes/io/schema_pb2.py
	find microscopes/ -name '*.cpp' -o -name '*.so' -o -name '*.pyc' -type f -print0 | xargs -0 rm -f --
