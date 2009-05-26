#!/bin/bash
# Builds a full documentation package in the "dist/" directory

# Delete and recreate directory
rm -rf dist
mkdir dist

# Build platform docs
lua builddoc.lua
cd wb
lua wb_build.lua
cd ..

# Copy the required files to the dist/ directory
for lang in en pt
do
  cp -R $lang/ dist/
done
cp -R wb_img dist/
for f in wb*.html style.css index*.html
do
  echo Copying $f...
  cp $f dist/
done

# Remove all version data from dist
find dist/ -name ".svn" | xargs rm -rf

# Remove unneeded files from base dir
for lang in en pt
do
  rm $lang/arch_platform_*.html
done
rm wb/wb_usr.lua

# All done
echo
echo "DONE! Enjoy your documentation in dist/ :)"


