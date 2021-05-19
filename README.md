![Release Version](release_badge.svg)

# IQ Control Center

This is all the source code used to create and run the IQ Control Center.

## Getting Started

If you just want to use the IQ Control Center please follow this link to the latest [release](https://github.com/iq-motion-control/iq-control-center/releases) where you will find precompiled versions of the IQ Control Center for Windows and Mac. 
  
If you are looking to play with the source code or compile from source continue reading !

### Prerequisites

- QT 5.15.x
- MacOS sdk 10.14 (minimum)
- Windows 10
- With v1.2.0, we've removed [QtAutoupdaterGui](https://github.com/Skycoder42/QtAutoUpdater/releases/tag/2.1.5-4) as seems like a unsupported repo. We now point directly to the QTInstaller Maintencetool for updating within the App.

Here is the link to download QT creator IDE for free : [QT Creator](https://www.qt.io/download?hsCtaTracking=c80600ba-f2ea-45ed-97ef-6949c1c4c236%7C643bd8f4-2c59-4c4c-ba1a-4aaa05b51086)

~~Here is a [link](https://github.com/Skycoder42/QtAutoUpdater/tree/2.1.5-4) to the right version of QTAutoupdater and the readme explains how to install it (use the maintenance tool)~~

### Installing

When you have QT installed, just choose Open Project and select IQControlCenter.pro file !

## Deployment

Deployment of this app can be done on any platform thanks to QT. If you would like more information on how to do this please follow the following link [QT deployement](https://doc.qt.io/qt-5/deployment.html)

I Recommend using the windeployqt, macdeployqt, and linuxdeployqt tools

```
C:\Qt\5.15.2\mingw81_32\bin> windeployqt \path\to\.exe
```

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
