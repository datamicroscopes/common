#!/usr/bin/env python

from distutils.core import setup
from distutils.extension import Extension
from distutils.version import LooseVersion
from distutils.sysconfig import parse_makefile
from Cython.Distutils import build_ext
from Cython.Build import cythonize
from cython import __version__ as cython_version

import numpy
import sys
import os

clang = False
if sys.platform.lower().startswith('darwin'):
    clang = True

# make sure C shared library exists
libmicroscopes_common = 'out/libmicroscopes_common.dylib' if clang else 'out/libmicroscopes_common.so'
if not os.path.isfile(libmicroscopes_common):
    raise ValueError(
        "could not locate libmicroscopes_common shared object. make sure to run `make' first")

# append to library path
os.environ['LIBRARY_PATH'] = os.environ.get('LIBRARY_PATH', '') + ':out'

min_cython_version = '0.20.2b1' if clang else '0.20.1'
if LooseVersion(cython_version) < LooseVersion(min_cython_version):
    raise ValueError(
        'cython support requires cython>={}'.format(min_cython_version))

distributions_inc, distributions_lib, debug_build = None, None, False

def get_config_info(config):
    config = parse_makefile(config)
    distributions_inc = config.get('DISTRIBUTIONS_INC', None)
    distributions_lib = config.get('DISTRIBUTIONS_LIB', None)
    cc = config.get('CC', None)
    cxx = config.get('CXX', None)
    debug_build = config.get('DEBUG', None)
    if debug_build is not None:
        debug_build = debug_build == 1
    return {
        'distributions_inc' : distributions_inc,
        'distributions_lib' : distributions_lib,
	'cc' : cc,
	'cxx' : cxx,
        'debug_build' : debug_build,
    }

for fname in ('../config.mk', 'config.mk'):
    try:
        for k, v in get_config_info(fname).iteritems():
            if v is None:
                continue
            locals()[k] = v
    except IOError:
        pass

if distributions_inc is not None:
    print 'Using distributions_inc:', distributions_inc
if distributions_lib is not None:
    print 'Using distributions_lib:', distributions_lib
if cc is not None:
    print 'Using CC={}'.format(cc)
    os.environ['CC'] = cc
if cxx is not None:
    print 'Using CXX={}'.format(cxx)
    os.environ['CXX'] = cxx
if debug_build:
    print 'Debug build'

extra_compile_args = ['-std=c++0x']
if clang:
    extra_compile_args.extend([
        '-mmacosx-version-min=10.7',  # for anaconda
        '-stdlib=libc++',
    ])
if debug_build:
    extra_compile_args.append('-DDEBUG_MODE')

extra_include_dirs = []
if distributions_inc is not None:
    extra_include_dirs.append(distributions_inc)

extra_link_args = []
if distributions_lib is not None:
    extra_link_args.extend([
        '-L' + distributions_lib,
        '-Wl,-rpath,' + distributions_lib
    ])

def make_extension(module_name):
    sources = [module_name.replace('.', '/') + '.pyx']
    return Extension(
        module_name,
        sources=sources,
        libraries=["microscopes_common", "protobuf", "distributions_shared"],
        language="c++",
        include_dirs=[numpy.get_include(), 'include'] + extra_include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args)

extensions = cythonize([
    make_extension('microscopes.cxx.models'),
    make_extension('microscopes.cxx._models'),
    make_extension('microscopes.cxx.common._dataview'),
    make_extension('microscopes.cxx.common._entity_state'),
    make_extension('microscopes.cxx.common.recarray.dataview'),
    make_extension('microscopes.cxx.common.recarray._dataview'),
    make_extension('microscopes.cxx.common.sparse_ndarray.dataview'),
    make_extension('microscopes.cxx.common.sparse_ndarray._dataview'),
    make_extension('microscopes.cxx.common.rng'),
    make_extension('microscopes.cxx.common._rng'),
    make_extension('microscopes.cxx.common.random'),
    make_extension('microscopes.cxx.common.scalar_functions'),
    make_extension('microscopes.cxx.common._scalar_functions'),
])

setup(ext_modules=extensions)
