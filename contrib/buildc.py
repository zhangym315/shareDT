#!/usr/bin/env python
#
# build-contrib.py -- build script for contrib

import sys
import os
import tarfile

try:
    import subprocess
except:
    import py24subprocess as subprocess

build64=None
def execute_qt(k):
    os.mkdir('./libjpeg-turbo-2.0.5/build/')
    os.chdir('./libjpeg-turbo-2.0.5/build/')
    ret=os.system('cmake ../')
    if ret != 0:
        sys.exit(1)

    ret=os.system('cmake --build .')
    if ret != 0:
        sys.exit(1)

    os.mkdir('../../zlib/build/')
    os.chdir('../../zlib/build/')
    ret=os.system('cmake ../')
    if ret != 0:
        sys.exit(1)

    ret=os.system('cmake --build .')
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

if kernel == "Linux":
    execute_qt(kernel)
elif kernel == "Darwin":
    execute_qt(kernel)
    print ("darwin")
elif kernel == "windows":
    execute_qt(kernel)
    print ("windows")



