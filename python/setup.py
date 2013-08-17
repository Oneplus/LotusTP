#!/usr/bin/env python
"""
setup.py file for SWIG example
"""
from distutils.core import setup, Extension

example_module = Extension("_LTP",
        sources=["ltp.i", "pyltp.cpp", ],
        include_dirs=[
            "../src/segmentor",
            "../src/postagger",
            "../src/ner",
            "../src/parser",
            "../src/utils",
            "../src/utils/math"],
        library_dirs=["../lib"],
        libraries=[
            "segmentor", 
            "postagger", 
            "ner",
            "parser",
            "boost_regex"],
        swig_opts=["-c++"],
        )

setup (name = 'LTP',
        version     = '0.0.1',
        author      = "SWIG Docs",
        description = """Simple swig example from docs""",
        ext_modules = [example_module],
        py_modules = ["LTP"],
        )
