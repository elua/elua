#!/bin/bash
# Builds a full documentation package in the "dist/" directory

# Delete and recreate directory
rm -rf dist
mkdir dist

# Build platform docs
lua builddoc.lua || exit 1
cd wb
lua wb_build.lua || exit 1
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
cp dist/index_en.html dist/index.html

# Remove all version data from dist
find dist/ -name ".svn" | xargs rm -rf

if [ "$1" != "noclean" ]
then
  # Remove unneeded files from base dir
  echo
  echo "Cleaning up..."
  for lang in en pt
  do
    rm -f $lang/arch_platform_*.html
    rm -f $lang/refman_gen_*.html
    rm -f $lang/refman_ps_lm3s_*.html
  done
  rm -f index_*.html wb/wb_usr.lua ssSearch*.html wb_bar_*.html
  rm -f wb_search*.txt wb_title*.html wb_tree*.html
else
  echo
  echo "NOT cleaning base directory!"
fi

# All done
echo
echo "DONE! Enjoy your documentation in dist/ :)"

