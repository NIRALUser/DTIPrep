#!/bin/bash
current_path=`pwd`
dir=`echo $current_path/$0 | sed 's/AppleCreateLinkLibs.sh//'`
# If using Apple platform, Please launch this script

# Create link for the Slicer's library folder
ln -sf $dir/../../../lib $dir/../lib

# Create link for the Slicer's Frameworks folder
ln -sf $dir/../../../Frameworks $dir/../Frameworks

echo "done"