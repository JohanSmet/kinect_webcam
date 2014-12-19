KinectWebcam v0.1
=================
A DirectShow capture filter that turns a Kinect v2 into a webcam.

Features
========
- supports the Kinect v2 and the original Kinect
- head-tracking 
- green screen (basic - more a gimmick than a feature)
- 32bit and 64bit support
- tested with: - Flash on Firefox, Chrome, and WaterFox (64-bit)
               - GraphStudioNext (32-bit and 64-bit)
	       - Yahoo Messenger (yes, really)
- Kinect v2 requires Windows 8+, original Kinect should work on older
  OS-versions (not tested yet).

Known Issues
============
- doesn't work with IE and Skype
	These only seem to detect WDM-drivers, not software DirectShow filter. TBI.

Binaries
========
Will be provided in the future.

Building
========
A CMake build file is provided. Visual Studio 2013 Professional was used for 
development but Visual Studio 2013 Express should probably work. Some C++11
features were used so it might not work with older versions.

Requirements :
- CMake
- The Microsoft DirectShow Base Classes (strmbase)
  http://msdn.microsoft.com/en-us/library/windows/desktop/dd407279%28v=vs.85%29.aspx
- OpenCV (http://opencv.org/)
- The Kinect v2 SDK (usually the latest version)
- The Kinect SDK (v1.8)

License
=======
This Software is provided under the MIT-license. See LICENSE.txt in the main
source directory or http://opensource.org/licenses/MIT for more details.

Contact
=======
Maintainer : Johan Smet <johan.smet@justcode.be>
