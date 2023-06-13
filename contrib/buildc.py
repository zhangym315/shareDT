#!/usr/bin/env python
#
# build-contrib.py -- build script for contrib
import sys
import os
import multiprocessing
import shutil

try:
    import subprocess
except:
    import py24subprocess as subprocess

build64=None
PWD=os.getcwd() + "/"
INS="/build/install/"
BLD="/build/"
DONE_FILE="/.INSTALLED_DONE"
cpuCount = multiprocessing.cpu_count()
cpuToBuild = str(cpuCount-1 if (cpuCount > 2) else cpuCount)


x265Dir = "x265_3.3/source/";
os.environ["PKG_CONFIG_PATH"] = PWD + x265Dir + INS + "/lib/pkgconfig/"
x264Include=PWD + "x264" + INS + "/include/"
x264Lib=PWD + "x264" + INS + "/lib/"
os.environ["PKG_CONFIG_PATH"] += ":" + x264Lib + "/pkgconfig"

def buildQT(k):
    pathBLD = PWD + component[0]

    if not os.path.isdir(pathBLD):
        os.makedirs(pathBLD)
    os.chdir(pathBLD)

    if not os.path.exists('.git'):
        os.mkdir('.git')

    ret=-1
    if k.lower() == "linux":
        ret=os.system('./configure -static -release -prefix ./static-build -xcb-xlib -xcb -recheck-all -nomake examples -nomake tests -skip qtwebengine -confirm-license -opensource')
    elif k.lower() == "darwin":
        ret=os.system('./configure -static -release -prefix ./static-build -recheck-all -nomake examples -nomake tests -skip qtwebengine -confirm-license -opensource ')
    elif k.lower() == "windows":
        ret=os.system('configure -static -release -prefix ./static-build -recheck-all -nomake examples -nomake tests -skip qtwebengine -confirm-license -opensource')
    else:
        print("NO specific platform")

    if ret != 0:
        return 1

    if k.lower() == "windows":
        ret=os.system('nmake')
    else:
        ret=os.system('make -j ' + cpuToBuild)
    if ret != 0:
        return 1

    if k.lower() == "windows":
        ret=os.system('nmake install')
    else:
        ret=os.system('make install')
    if ret != 0:
        return 1
    open(PWD + component[0] + DONE_FILE, 'w')

def buildWindowsOpenssl():
    pathINS = PWD + component[0] + INS
    pathBLD = PWD + component[0]

    if not os.path.isdir(pathBLD):
        os.makedirs(pathBLD)
    os.chdir(pathBLD)

    ret=os.system('perl Configure VC-WIN64A no-asm no-shared -static --release --prefix=' + pathINS + ' --openssldir=' + pathINS)
    if ret != 0:
        return 1

    ret=os.system('nmake')
    if ret != 0:
        return 1

    ret=os.system('nmake install')
    if ret != 0:
        return 1
    open(PWD + component[0] + DONE_FILE, 'w')

def buildBZip2():
    pathINS = PWD + component[0] + INS
    pathBLD = PWD + component[0]

    os.chdir(pathBLD)

    ret=os.system('make -j ' + cpuToBuild)
    if ret != 0:
        return 1

    ret=os.system('make install PREFIX=' + pathINS)
    if ret != 0:
        return 1
    open(PWD + component[0] + DONE_FILE, 'w')

# specific to build on windows msys, for ffmpeg, liblzma and liblzma
def buildOnWinMSYS(component):
    pathINS = PWD + component[0] + INS
    pathBLD = PWD + component[0]

    if not os.path.isdir(pathINS):
        os.makedirs(pathINS)
    os.chdir(pathBLD)

    if component[0] == "ffmpeg":
        cmd = 'unset CL && ./' + component[1] +' --toolchain=msvc --enable-swscale --enable-asm --enable-yasm --target-os=win64 --arch=x86_64 --disable-shared  --disable-avdevice  --disable-doc  --disable-ffplay  --disable-ffprobe  --disable-ffmpeg  --enable-w32threads --disable-amf --disable-ffnvcodec --disable-mediafoundation --prefix="' + pathINS +'"'
        print('Running ffmpeg configure, this may take several mins, please wait...')
        print(cmd)

        ret = os.system(cmd)
        if ret != 0:
            return 1
    else:
        cmd = './' + component[1] +'  --prefix=' + pathINS
        # only copy for windows platform build
        if component[0] == "x264":
            shutil.copy2(PWD+'shared/x264-config.guess', pathBLD + '/config.guess')
            cmd = 'CC=cl ' + cmd

        print ("running: " + cmd)
        ret=os.system(cmd)
        if ret != 0:
            return 1

    MakeCMD = 'make -j ' + cpuToBuild
    print("Running CMD: " + MakeCMD)
    ret=os.system(MakeCMD)

    if ret != 0:
        return 1
    ret=os.system('make install')

    if ret != 0:
        return 1
    open(PWD + component[0] + DONE_FILE, 'w')

    return

def cleanBuild(kernel, component):
    print('Clearning for ' + component[0])

    installFullPath=PWD+component[0]+DONE_FILE
    if os.path.exists(installFullPath):
        os.remove(installFullPath)

    ## Normal cmake build
    if component[1] == "CMAKE" :
        pathBLD = PWD + component[0] + BLD
        ret=os.system('cmake --build ' + pathBLD + ' --target clean ')
        if ret != 0:
            print("Failed to clean " + component[0])
    else :
        pathBLD = PWD + component[0]

        os.chdir(pathBLD)
        MakeCMD = 'make clean'
        if kernel == "windows":
            MakeCMD = 'nmake clean'
        ret=os.system(MakeCMD)
        if ret != 0:
            print("Failed to clean " + component[0])
            return 1

def buildAndInstall(kernel, component):
    if os.path.exists(PWD + component[0] + DONE_FILE):
        print('Built ' + component[0])
        return

    print('Building ' + component[0] + ' at platform ' + kernel)

    ### build special component first
    if component[0] == "qt515":
        return buildQT(kernel)
    if component[0] == "bzip2":
        return buildBZip2()
    if component[0] == "ffmpeg":
        shutil.copyfile(PWD+"./x265_3.3/source/build/install/include/x265.h", PWD+"./ffmpeg/x265.h")
        shutil.copyfile(PWD+"./x265_3.3/source/build/install/include/x265_config.h", PWD+"./ffmpeg/x265_config.h")

    ## windows build component
    if kernel == "windows" or kernel.startswith("cygwin") or kernel.startswith("msys"):
        if component[0] == "openssl":
            return buildWindowsOpenssl()

        if component[0] == "liblzma" or component[0] == "ffmpeg" or component[0] == "x264":
            if (kernel.startswith("msys")):
                return buildOnWinMSYS(component=component)
            else:
                if os.path.exists('C:\msys64\msys2_shell.cmd'):
                    print('Switching to msys to build windows special component ' + component[0])
                    os.system('C:\msys64\msys2_shell.cmd -msys -use-full-path -here -c ./buildc.py')
                else:
                    print("Cannot find msys64 to build" + component[0] + ", please refer to README.md to setup the environment.")
                    print('Please switch msys to build ' + component[0])
                return 2 ### switching msys64 to build

    ## Normal cmake build
    if component[1] == "CMAKE" :
        pathINS = PWD + component[0] + INS
        pathBLD = PWD + component[0] + BLD
        if not os.path.isdir(pathBLD):
            os.makedirs(pathBLD)
        os.chdir(pathBLD)
        if kernel == "windows" :
            ret=os.system('cmake -A x64 -DCMAKE_INSTALL_PREFIX=' + pathINS + ' ../')
        else:
            ret=os.system('cmake -DCMAKE_INSTALL_PREFIX=' + pathINS + ' ../ ' + component[2])

        if ret != 0:
            return 1

        ret=os.system('cmake --build . --target install --config Release --parallel ' + cpuToBuild)
        if ret != 0:
            return 1
    ## normal Makefile build
    else:
        pathINS = PWD + component[0] + INS
        pathBLD = PWD + component[0]

        if not os.path.isdir(pathINS):
            os.makedirs(pathINS)
        os.chdir(pathBLD)

        ret=os.system( './' + component[1] + ' --prefix=' + pathINS)
        if ret != 0:
            return 1

        MakeCMD = 'make -j ' + cpuToBuild
        print("Running CMD: " + MakeCMD)
        ret=os.system(MakeCMD)
        if ret != 0:
            return 1
        ret=os.system('make install')
        if ret != 0:
            return 1
    open(PWD + component[0] + DONE_FILE, 'w')


def normalize_kernel(k):
    if k.lower() == "solaris" or k.lower() == "sun":
        return "SunOS"		# Special case
    if k.lower() == "hpux":
        return "HP-UX"		# ditto
    for onek in [ "Linux", "FreeBSD", "OpenBSD", "SunOS", "Darwin", "AIX", "windows", "HP-UX" ]:
        if k.lower() == onek.lower():
            return onek
    return k.lower()

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
elif sys.platform == "cygwin*":
    kernel = "cygwin"
else:
    def backtick(arg):
        uresult = os.popen(arg, "r")
        v = uresult.readline()
        uresult.close()
        return v.rstrip('\n')
    kernel = normalize_kernel(backtick("uname -s"))

# 1. components name 2. Build type 3. build options
components = [["SDL2-2.0.12", "CMAKE", "-DHAVE_GCC_WDECLARATION_AFTER_STATEMENT=0"], \
              ["libjpeg-turbo-2.0.5", "CMAKE", ""], \
              ["zlib", "CMAKE", ""], \
              ["libpng-1.6.37", "CMAKE", ""], \
              ["lzo-2.10", "CMAKE", ""], \
              ["vvdec", "CMAKE", ""], \
              ["vvenc", "CMAKE", ""], \
              [x265Dir, "CMAKE", ""], \
              ["openssl", "Configure", ""], \
              ["qt515", "configure", ""], \
              ["x264", "configure --enable-static --disable-asm --disable-cli", ""], \
              ["ffmpeg", "configure --disable-iconv --enable-libx265 --enable-gpl --disable-ffmpeg --disable-ffprobe  --enable-libx264 --extra-ldflags=-L" + x264Lib + " --extra-cflags=-I" + x264Include, ""], \
              #              ["ffmpeg", "configure --disable-iconv --enable-libx265 --enable-gpl --disable-ffmpeg --disable-ffprobe  --enable-libx264 "],\
              ["liblzma", "configure", ""], \
              ["bzip2", "configure", ""] \
              ]

for component in components :
    if len(sys.argv) == 2 and sys.argv[1] == "clean":
        cleanBuild(kernel, component)
    else:
        buildRet=buildAndInstall(kernel, component)
        if buildRet == 1 or buildRet == 2:
            if buildRet == 1:
                print("Failed to build " + component[0])
            break

if (kernel.startswith("msys")):
    os.system("bash")  ## start bash to avoid exit automatically
