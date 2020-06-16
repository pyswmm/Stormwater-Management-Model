#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from skbuild import setup

VERSION = '5.2.0.dev6'

CLASSIFIERS = ["Development Status :: 5 - Production/Stable",
               "Environment :: Console",
               "Intended Audience :: Science/Research",
               "Intended Audience :: End Users/Desktop",
               "License :: OSI Approved :: MIT License",
               "Operating System :: MacOS :: MacOS X",
               "Operating System :: POSIX :: Linux",
               "Operating System :: Microsoft :: Windows",
               "Programming Language :: C",
               "Topic :: Scientific/Engineering",
               "Topic :: Scientific/Engineering :: Hydrology",
               "Topic :: Software Development :: Libraries"]

DESCR = "OWA Stormwater Management Model "
LONG_DESCR = """SWMM is a dynamic hydrology-hydraulic water quality simulation model. It is 
                used for single event or long-term (continuous) simulation of runoff quantity 
                and quality from primarily urban areas."""

metadata = dict(name='swmm',
                version=VERSION,
                description=DESCR,
                long_description=LONG_DESCR,
                url='https://github.com/OpenWaterAnalytics/Stormwater-Management-Model',
                license='MIT',
                classifiers=CLASSIFIERS,
                keywords='science engineering hydrology drainage',
                )

setup(**metadata)
