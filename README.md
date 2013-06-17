This is the git repository for the Quake-Catcher Network project.

All files are (c) 2013 Stanford University, School of Earth Sciences

Primary author of all files unless otherwise noted is Carl Christensen, carlgt1@yahoo.com  (github: carlgt1)

This repository is released under the Lesser GNU Public License (LGPL) for academic use and experimentation.

No warrantees etc are given -- use at your own risk!

You will need various dependencies, some of which are included e.g. libcurl etc, but mainly you will need to get
the BOINC libraries cloned parallel to the qcn directory:
cd ..
git clone git://boinc.berkeley.edu/boinc.git

basically the QCN client code is in the subdir "client" and the server code in "server" (duh)

In client there is a subdirectory "qcnlive" for the Qt-based (you'll need to install Qt) QCNLive program - and makefiles
for Linux and visual editor files for XCode (Mac) and Visual Studio (Windows)

precompiled libraries and the visual studio/xcode files are also in win_build & mac_build respectively
