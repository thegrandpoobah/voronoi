@echo off
pushd
cd PGO
del *.pgc
echo Instrumenting (1/2)...
voronoi.exe -l -c -t 0.3 ..\corpus\phoenix.png phoenix.svg
echo Instrumenting (2/2)...
voronoi.exe -l -n -t 0.3 ..\corpus\fairyeyes.png fairyeyes.svg
popd
