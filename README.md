![tag](https://img.shields.io/github/v/tag/iq-motion-control/iq-control-center)
![release](https://img.shields.io/github/release/iq-motion-control/iq-control-center/all.svg)

# IQ Control Center

This is all the source code used to create and run the IQ Control Center.

## Getting Started

If you just want to use the IQ Control Center please follow this link to the latest [release](https://github.com/iq-motion-control/iq-control-center/releases) where you will find precompiled versions of the IQ Control Center for Windows and Mac. 
  
If you are looking to play with the source code or compile from source continue reading !

### Prerequisites

- QT 5.13
- MacOS sdk 10.14
- Windows 10
- [QtAutoupdaterGui](https://github.com/Skycoder42/QtAutoUpdater/releases/tag/2.1.5-4) v2.1.5-4 (for build qt 5.13)

Here is the link to download QT creator IDE for free : [QT Creator](https://www.qt.io/download?hsCtaTracking=c80600ba-f2ea-45ed-97ef-6949c1c4c236%7C643bd8f4-2c59-4c4c-ba1a-4aaa05b51086)

Here is a [link](https://github.com/Skycoder42/QtAutoUpdater/tree/2.1.5-4) to the right version of QTAutoupdater and the readme explains how to install it (use the maintenance tool).

### Installing

When you have QT installed, just choose Open Project and select IQControlCenter.pro file !

## Running

QT normally compiles and create an executable in a different directory that where the source code is located. You have to make sure to have the Resource folder in the same directory of your executable after you compiled the project on QT Creator. You will still be able to run the Control Center without the Resource Folder in the same directory but the tabs won't populate when you connect a motor.

## Deployment

Deployment of this app can be done on any platform thanks to QT. If you would like more information on how to do this please follow the following link [QT deployement](https://doc.qt.io/qt-5/deployment.html)


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/iq-motion-control/iq-control-center/tags) or the [releases on this repository](https://github.com/iq-motion-control/iq-control-center/releases).

## Authors

* **Raphael Van Hoffelen** 

See also the list of [contributors](contributors.md) who participated in this project

## License

The Qt app part of this project is licensed under the GNU GPL-3.0-or-later license  - see the [LICENSE](LICENSE) file for details.  
The IQ_API part of this project is licensed under the GNU LGPL-3.0-or-later license - see the [LICENSE.LESSER](App/IQ_api/LICENSE.LESSER) file for details.
