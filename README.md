![tag](https://img.shields.io/github/v/tag/iq-motion-control/iq-control-center)
![release](https://img.shields.io/github/release/iq-motion-control/iq-control-center/all.svg)

# IQ Control Center

This is all the source code used to create and run the IQ Control Center.

## Getting Started

If you just want to use the IQ Control Center please follow this link to the latest [release](https://github.com/iq-motion-control/iq-control-center/releases) where you will find precompiled versions of the IQ Control Center for Windows and Mac. 
  
If you are looking to play with the source code or compile from source continue reading !

### Prerequisites

- QT 5.15.x
- MacOS sdk 10.14 (minimum)
- Windows 10
- Linux Ubuntu 20.04 (Built and Compiled on)

### v1.2.0 Release Notes
With v1.2.0, we've removed [QtAutoupdaterGui](https://github.com/Skycoder42/QtAutoUpdater/releases/tag/2.1.5-4). We now point directly to the QTInstaller Maintencetool for updating within the App.

Here is the link to download QT creator IDE for free : [QT Creator](https://www.qt.io/download-qt-installer)


### Installing

When you have QT installed, just choose Open Project and select IQControlCenter.pro file !

## Troubleshooting

We've noticed issues with some users connecting to our servers when attempting to update to 1.1.2. If you are experiencing this issue and it's not related to a firewall, please submit an Issue Ticket via Github.

Alternatively, we have provided an offline installer with binaries included. [v1.2.0 installer](https://github.com/iq-motion-control/iq-control-center/releases/tag/v1.2.0) _(*See Note Below)_

**Notes:*  
* *Make sure to transfer any custom resource files you may have added. If you haven't receieved any resource files from IQ Motion Control Engineers, chances are this doesn't apply to you and you can safely delete the root folder*


## Deployment

Deployment of this app can be done on any platform thanks to QT. If you would like more information on how to do this please follow the following link [QT deployement](https://doc.qt.io/qt-5/deployment.html)

Currently there isn't an option for cross-deployment, so you'll need can only deploy on the same system as your target.

|  System | Deployment Tool                                                               |
|:-------:|-------------------------------------------------------------------------------|
| Windows |  [windeployqt](https://doc.qt.io/qt-5/macos-deployment.html)			      |
| Mac     |  [macdeployqt](https://doc.qt.io/qt-5/macos-deployment.html)   			  	  |  
| Linux   |  [linuxdeployqt*](https://github.com/probonopd/linuxdeployqt/releases/tag/5)  |

**Note: We are using version 5 of linuxdeployqt due to compatibility issues with newer linux distros. You can read more about the issue [here](https://github.com/probonopd/linuxdeployqt/issues/340)*


```
Windows Deployment Example:

C:\Qt\5.15.2\mingw81_32\bin> windeployqt \path\to\.exe
```

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/iq-motion-control/iq-control-center/tags) or the [releases on this repository](https://github.com/iq-motion-control/iq-control-center/releases).

## Authors & Maintainers

* **Raphael Van Hoffelen** 
* **Malik B. Parker**

See also the list of [contributors](contributors.md) who participated in this project

## License

The Qt app part of this project is licensed under the GNU GPL-3.0-or-later license  - see the [LICENSE](LICENSE) file for details.  
The IQ_API part of this project is licensed under the GNU LGPL-3.0-or-later license - see the [LICENSE.LESSER](App/IQ_api/LICENSE.LESSER) file for details.
