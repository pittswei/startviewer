#!/bin/bash

# Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
# Institut de Diagnòstic per la Imatge.
# Girona 2014. All rights reserved.
# http://starviewer.udg.edu
#
# This file is part of the Starviewer (Medical Imaging Software) open source project.
# It is subject to the license terms in the LICENSE file found in the top-level
# directory of this distribution and at http://starviewer.udg.edu/license. No part of
# the Starviewer (Medical Imaging Software) open source project, including this file,
# may be copied, modified, propagated, or distributed except according to the
# terms contained in the LICENSE file.

if [ $# -lt 1 ]
then
  echo "Please specify the Homebrew Qt directory"
  exit 1
fi

# Avoid confusion with QTDIR environment variable
QT_DIR=$1

# Change binaries

# List of Mach-O binaries in the bin dir, i.e. excluding scripts
BINARIES=$(find $QT_DIR/bin -type f -perm +111 -exec bash -c "file '{}' | grep -q 'Mach-O'" \; -print)

for f in $BINARIES
do
  chmod +w $f
  ./install_name_prefix_tool.sh $f @@HOMEBREW_CELLAR@@ /usr/local/Cellar > /dev/null
done

# Change libs

# List of frameworks with the pattern QtFoo.framework (without the prefix path)
FRAMEWORKS=$(find $QT_DIR/lib -type d -name "*.framework" | xargs basename)

for f in $FRAMEWORKS
do
  LIB=${f%.*}
  FULLNAME=$QT_DIR/lib/$LIB.framework/$LIB
  chmod +w $FULLNAME
  ./install_name_prefix_tool.sh $FULLNAME @@HOMEBREW_CELLAR@@ /usr/local/Cellar > /dev/null
  ./install_name_prefix_tool.sh $FULLNAME @@HOMEBREW_PREFIX@@ /usr/local > /dev/null
done
