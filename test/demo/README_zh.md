## 程序功能

该Demo展示了播放、暂停、从头播放、切换音频源、进度显示、渲染速率设置、音量设置、（取消）静音、显示AudioRenderer和AudioManager信息等功能。

## 使用说明

- OpenHarmony SDK 版本：`API Version 8`。
- DevEco Studio 版本: `3.0.0.900`。

程序运行后，需要将pcm文件发送到到设备，命令如下。（send后的第一个参数为本地文件的位置）

```
hdc_std file send \test.pcm /data/app/el2/100/base/com.example.audioplayerdemo/haps/entry/files/

hdc_std file send \test2.pcm /data/app/el2/100/base/com.example.audioplayerdemo/haps/entry/files/
```