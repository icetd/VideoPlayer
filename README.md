## build for win

ENV: Visual Studio 2022

```
mkdir build && cd build
cmake ..
MSBuild.exe VideoPlayer.sln -t:Rebuild -p:Configuration=Release
```