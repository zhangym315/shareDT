# Linux(Ubuntu) & MacOS Build
### Linux Required package(Ubuntu)
* Build essential package
```
sudo apt-get install build-essential
sudo apt-get install libgtk2.0-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install cmake python
```
* libx11 related package
```
sudo apt-get install "^libxcb.*" libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxkbcommon-x11-dev
sudo apt-get install libxtst-dev
sudo apt-get install nasm
```
### MacOS
* Required package: XCode 13+, python, pkg-config
* Install nasm
```
brew install nasm
brew install pkg-config
```

### CMD To Build
* 1. git clone code
```
git clone https://github.com/zhangym315/shareDT.git
cd shareDT
git submodule init
git submodule update
```
* 2. Build contrib
```
cd contrib && buildc.py
```
* 3. Build source code
```
mkdir && cd shareDT/build/
cmake ../
cmake --build .  --config Release --parallel
```
* 4. Install
```
cmake --install .  --prefix install
```

# Windows Build
### Requirted Package
* 1. Install Visual Studio 2019 community(Select Desktop C++)
* 2. Install msys2 and install to ```C:``` root directory. After finished installation, run following to install package:
```
pacman -S make gcc diffutils python

rename link.exe to others
mv /usr/bin/link.exe /usr/bin/link-origin.exe
```
* 3. Download flex and add bin to path environment, rename ```win_flex.exe``` to ```flex.exe``` and ```win_bison.exe``` to ```bison.exe```
```
https://sourceforge.net/projects/winflexbison/files/win_flex_bison-2.5.5.zip/download
```
* 4. Install ```python3``` and ```Strawberry Perl``` and add executable to ```path``` environment
* 5. Download ```yasm``` from following link and add it to path
```
https://github.com/yasm/yasm/releases/tag/v1.3.0
```
* 6. Download and install Microsoft Visual C++ 2010 Redistributable Package x64


### Build
* 1. Start cmd from VS2019
* 2. Run following to set multi processors
```
set CL=/MP
```
* 3. clone source code
```
git clone https://github.com/zhangym315/shareDT.git
cd shareDT
git submodule init
git submodule update

```
* 3. Build contrib
```
cd contrib
python buildc.py
```
* 4. Build source code
```
mkdir build
cd build/
cmake ../
cmake --build . --config Release --parallel
```
* 5. Install
```
cmake --install . --prefix install
```

### Windows MSI generation
```
candle .\ShareDT.wxs
light -ext WixUIExtension -cultures:en-us .\ShareDT.wixobj -out ShareDT.msi
```
