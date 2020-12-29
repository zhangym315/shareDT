#!/usr/bin/env python
#
# build-contrib.py -- build script for contrib

import sys
import os

try:
    import subprocess
except:
    import py24subprocess as subprocess

build64=None
PWD=os.getcwd() + "/"
INS="/build/install/"
BLD="/build/"

def buildWindowsOpenssl():
    pathINS = PWD + component[0] + INS
    pathBLD = PWD + component[0]
    if not os.path.isdir(pathBLD):
        os.makedirs(pathBLD)
    os.chdir(pathBLD)
    print("change to:" + pathBLD)
    ret=os.system('perl Configure VC-WIN64A no-asm --release --prefix=' + pathINS + ' --openssldir=' + pathINS)
    if ret != 0:
        sys.exit(1)
    ret=os.system('nmake')
    if ret != 0:
        sys.exit(1)
    ret=os.system('nmake install')
    if ret != 0:
        sys.exit(1)

def buildAndInstall(kernel, component):
    if kernel == "windows" and component[0] == "openssl":
        buildWindowsOpenssl()
        return

    if component[1] == "CMAKE" :
        pathINS = PWD + component[0] + INS
        pathBLD = PWD + component[0] + BLD
        if not os.path.isdir(pathBLD):
            os.makedirs(pathBLD)
        os.chdir(pathBLD)
        if kernel == "windows" :
            ret=os.system('cmake -A x64 -DCMAKE_INSTALL_PREFIX=' + pathINS + ' ../')
        else:
            ret=os.system('cmake -DCMAKE_INSTALL_PREFIX=' + pathINS + ' ../')

        if ret != 0:
            sys.exit(1)

        ret=os.system('cmake --build . --target install --config Release')
        if ret != 0:
            sys.exit(1)
    else:
        pathINS = PWD + component[0] + INS
        pathBLD = PWD + component[0]
        if not os.path.isdir(pathINS):
            os.makedirs(pathINS)
        os.chdir(pathBLD)
        ret=os.system( './' + component[1] + ' --prefix=' + pathINS)
        if ret != 0:
            sys.exit(1)
        ret=os.system('make -j 10')
        if ret != 0:
            sys.exit(1)
        ret=os.system('make install')
        if ret != 0:
            sys.exit(1)

def normalize_kernel(k):
    if k.lower() == "solaris" or k.lower() == "sun":
        return "SunOS"		# Special case
    if k.lower() == "hpux":
        return "HP-UX"		# ditto
    for onek in [ "Linux", "FreeBSD", "OpenBSD", "SunOS", "Darwin", "AIX", "windows", "HP-UX" ]:
        if k.lower() == onek.lower():
            return onek
    return k

if build64 is None:
    # Assume that we want a 64-bit build unless --32 was specifically passed in.
    # Note that we could still find ourselves on a 32-bit architecture
    # later; this just controls which type of build we'll do if we're in
    # an environment where either alternative is possible.
    build64 = True
    # The releng scripts always explicitly pass "--32" or "--64" when building
    # windows so there isn't any ambiguity.  However, if no explicit flag was
    # passed, default it based on the OS we seem to be running on.
    if sys.platform == "win32":
        import platform
        if not platform.machine().endswith('64'):
            print("We seem to be building on a 32-bit OS, defaulting to --32")
            build64 = False

if sys.platform == "win32":
    kernel = "windows"
    if build64:
        arch = "amd64"
    else:
        arch = "i386"
else:
    def backtick(arg):
        uresult = os.popen(arg, "r")
        v = uresult.readline()
        uresult.close()
        return v.rstrip('\n')
    kernel = normalize_kernel(backtick("uname -s"))

components = [["SDL2-2.0.12", "CMAKE"], ["libjpeg-turbo-2.0.5", "CMAKE"], ["zlib", "CMAKE"],\
             ["libpng-1.6.37", "CMAKE"], ["lzo-2.10", "CMAKE"], ["x265_3.3/source/", "CMAKE"], ["openssl", "Configure"]]
for component in components :
    buildAndInstall(kernel, component)



