# LVGL 在 SigmaStar 平台上的优化

## 快速开始

### 编译
1. 根据项目需求更改 `Makefile` 中 `ALKAID_PROJ` 和 `CC` 的值，例如：

    ```mk
    CC := arm-linux-gnueabihf-gcc
    ALKAID_PROJ := /path/to/alkaid/project
    ```

2. 在项目根目录下执行 `make -j8`

3. 生成 `build` 目录，`build/bin/demo` 即为最终可执行程序。

### 运行
默认情况下，编进去了 3 个官方的 demo，运行方式如下：

注：如果运行demo时发生屏幕撕裂问题，修改 /config/fbdev.ini 缓冲区到2倍的大小
如1024*600时，将FB_BUFFER_LEN修改成4800即可

```sh
./demo a # music
./demo b # benchmark
./demo c # widgets
```

## 旋转开启
修改lv_conf.h下的SSTAR_GFX_ROTATE和SSTAR_GFX_ROTATE_ANGLE，可以开启或者关闭gfx硬件加速旋转以及设置旋转角度。（与lvgl原生旋转冲突，需关闭原生旋转配置！）
注意：
SSTAR_GFX_ROTATE开启为1时，fbdev.ini下的宽高与lvgl的disp_drv.hor_res和disp_drv.ver_res两个参数要对调设置。否则就不要对调设置。
举例：SSTAR_GFX_ROTATE为1时，fbdev.ini设置的是720x1280，则disp_drv.hor_res=1280；disp_drv.ver_res=720。SSTAR_GFX_ROTATE为0时，disp_drv.hor_res=720；disp_drv.ver_res=1280


## 项目结构

```
▸ build/                    // 编译生成的文件
▸ lv_drivers/               // 官方 driver，这里主要使用 indev 部分
▸ lv_porting_sstar/         // 移植与优化相关代码
▸ lvgl/                     // lvgl 库主体代码
▸ squareline_proj/          // 为 squareline studio 准备的目录，存放预设的 makefile
  lv_conf.h                 // lvgl 配置文件
  lv_drv_conf.h             // lv_drivers 配置文件
  main.c                    // 主函数文件
  Makefile                  // Makefile
  README.md                 // README
```

## Squareline Studio 项目导入

1. 使用 squareline studio 导出代码；
2. 把所有代码放到 `squareline_proj` 目录下；
3. 修改 main.c 如下部分：

```diff
#include "benchmark/lv_demo_benchmark.h"
-//#include "ui.h"
+include "ui.h"

void *tick_thread(void * data)

......

} else if (0 == strcmp(argv[1], "d")) {
-   //ui_init();
+   ui_init();
}
```

4. 重新 `make -j8` 编译
5. 运行 `./demo d`

