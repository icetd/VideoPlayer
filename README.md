## build for win

```
mkdir build
cmake ..
MSBuild.exe VideoPlayer.sln -t:Rebuild -p:Configuration=Release
```

App Demo base on imgui.

![](pic/view.png)