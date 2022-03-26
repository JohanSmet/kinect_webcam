KinectWebcam v0.1
=================
A DirectShow capture filter that turns a Kinect v2 into a webcam.

Features
========
- supports the Kinect v2 and the original Kinect
- head-tracking 
- green screen (basic - more a gimmick than a feature)
- 32bit and 64bit support
- tested with:
	- Flash on Firefox, Chrome, and WaterFox (64-bit)
    - GraphStudioNext (32-bit and 64-bit)
	- Yahoo Messenger (yes, really)
- Kinect v2 requires Windows 8+, original Kinect should work on older
  OS-versions (not tested).

Known Issues
============
- Doesn't work with IE and Skype
  These only seem to detect WDM-drivers, not software DirectShow filter. TBI.

Binaries
========
Will be provided in the future.

Building
========
A CMake build file is provided and the Conan C/C++ package manager is to fetch dependencies.
Visual Studio 2013 was used for development; newer should fine. Some C++11 features were used
so it might not work with older versions.

Requirements :
- CMake (>= 2.8.12)
- [Conan](https://conan.io)
- The Microsoft DirectShow Base Classes (strmbase). These are not included in this repository because I'm unsure if its license allows this.
  (see http://msdn.microsoft.com/en-us/library/windows/desktop/dd407279%28v=vs.85%29.aspx or https://github.com/microsoft/Windows-classic-samples).
- [OpenCV 2](http://opencv.org/) (provided by Conan)
- Qt 5 (provided by Conan)
- The Kinect v2 SDK (usually the latest version)
- The Kinect SDK (v1.8)

Detailed instructions (assume msys2 or other source of posix utilities are installed):

```bash
# clone kinect webcam
git clone https://github.com/JohanSmet/kinect_webcam.git

# clone DirectShow Base Classes
git clone --depth 1 --filter=blob:none --sparse https://github.com/microsoft/Windows-classic-samples.git
git sparse-checkout set Samples/Win7Samples/multimedia/directshow/baseclasses
cp -a Windows-classic-samples\Samples\Win7Samples\multimedia\directshow\baseclasses kinect_webcam\strmbase

# prepare build directory
cd kinect_webcam
mkdir build && cd build
conan install .. -s build_type=Release
cd ..
cmake -B build -G "Visual Studio 16 2019" -A x64 -DENABLE_KINECT_V2=ON -DENABLE_KINECT_V2=ON

# build
cmake --build build --config Release
cmake --build build --config Release --target install
```

License
=======
This Software is provided under the MIT-license. See LICENSE.txt in the main
source directory or http://opensource.org/licenses/MIT for more details.

Contact
=======
Maintainer : Johan Smet <johan.smet@justcode.be>
