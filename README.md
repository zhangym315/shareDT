# Dependency
## Linux
* Required package
```
sudo apt install build-essential
sudo apt install libgtk2.0-dev             
sudo apt install libgtk-3-dev
```
* Required xcb-xlib
```
sudo apt-get install "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev
```

## Windows
```Visual Studio 2019```


# Build
## Enivronment(all platform)
```
export Qt5Widgets_DIR=${SHAREDT_SRC}/shareDT/contrib/qt515/static-build/lib/cmake/Qt5Widgets
```

* Linux Environment
```
export CMAKE_MODULE_PATH=${SHAREDT_SRC}/contrib/qt515/static-build/bin
export PATH=${SHAREDT_SRC}/contrib/qt515/static-build/bin
```

## CMD
```
git clone https://github.com/zhangym315/shareDT.git
mkdir && cd shareDT/build/
cmake --build ../
cmake .
```

## Windows Build For contrib/qt515
* Install MinGW
* Under MinGW, checkout the lastest code of win_flex_bison-latest and build
* Suppose shareDT checkout at C:\shareDT\
```
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
SET _ROOT=C:\shareDT\contrib\qt515
SET PATH=%PATH%;C:\MinGW\bin;C:\MinGW\msys\1.0\bin;%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin
REM SET PATH=%_ROOT%\qtrepotools\bin;%PATH%
SET PATH=C:\win_flex_bison-latest\bin\Debug;%PATH%
```
