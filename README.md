nautilus
----------------------------------
A game we wrote for Stanford CS478.

![Screenshot](http://nicdahlquist.com/nautilus/img/screenshot0.png)

The 'android', 'ios and 'linux' directories contain build files for their
respective platforms, which call the code in the 'common' directory through
the 'common.h' api.

Based on source code from the Android Open Source Project, the Stanford CS148
starter project, and Eigen.


###Build instructions:

####Android:
```
cd android
ndk-build
ant debug install
```

####Linux:
```
cd linux
make
```

####iOS:
Some black magic with X-Code.
