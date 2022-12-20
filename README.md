![Release Version](https://img.shields.io/badge/Release-v1.2.1-green)

# IQ Control Center

This is all the source code used to create and run the IQ Control Center.

## Getting Started

If you just want to use the IQ Control Center please follow this link to the latest [release](https://github.com/iq-motion-control/iq-control-center/releases) where you will find precompiled versions of the IQ Control Center for Windows, Mac, and Linux.

### General Installation Instructions
- Download the latest precompiled release from the link above
- Extract the zip archive from your download, and run the application
- Follow the instructions to complete the installation

### Notes on Linux Installation
You must add the libxcb-xinerama0 library
```
sudo apt-get install libxcb-xinerama0
```
In order to run the Control Center, and have access to all features, you must run the application through the command line as sudo
```
cd <Path/to/IQ Control Center/executable>
sudo ./IQ\ Control\ Center
```
  
If you are looking to play with the source code or compile from source continue reading!

## Cloning this Repo

This repo uses a submodule called 'Schmi' (the flashing portion of Control Center) and will return as an empty folder if you clone this repo without the `--recursive` flag. Please run this command when cloning this repo.

Clone the repo and all submodules within it:  

``` shell
git clone --recursive <repo-url>
```

### Already cloned the repo without the recursive flag?

Run the following:  

``` shell
git submodule update --init
```

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

On Windows, we've noticed issues with updating from 1.1.0 and below. Updating from 1.1.2 is working as intended. 

If you are experiencing this issue, please submit an Issue Ticket via Github. If the issue requires immediate action, you can also completely uninstall IQ Control Center by **deleting** the root application folder and downloading the [v1.2.0 installer](https://github.com/iq-motion-control/iq-control-center/releases/tag/v1.2.0) _(*See Note Below)_

**Notes:* 
* *The Root Application Folder "IQ Control Center" is more than likely located in Program Files(x86)*
* *Make sure to save any custom resource files you may have added. If you haven't receieved any resource files from IQ Motion Control Engineers, chances are this doesn't apply to you and you can safely delete the root folder*

## PreDeployment Workflow 

1. Choose a system of preference to build and test on:
    - This should be where they heavy code lifting should be done. I personally prefer linux.
    - Why?: To keep track of code changes and keep a consistent analysis method. Change one variable at a time (Imagine changing systems mid build! Get it working on one system first then move to the next.)
2. Before deploying on the first system, there's a few basics tests that must work (It would be nice to automate these in the future one day):

|                    Test                    |                                                            Details                                                             |
| :----------------------------------------: | ------------------------------------------------------------------------------------------------------------------------------ |
| Connect to each IQ Motor (Public Releases) | -Vertiq 8108 150Kv <br> -Vertiq2306 220Kv <br> -Vertiq2306 2200Kv                                                              |
|      Connects to Installer/Repository      | -On startup, shows message about updates in header <br> - Clicking on Installer->"Check for Updates" launches installer window |
|       Motor is able to be controlled       | -In testing tab, be able to move the motor                                                                                     |
|      Able to flash each motor module       | -Flash software onto the motors and go to testing tab to make sure it works                                                    |

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
