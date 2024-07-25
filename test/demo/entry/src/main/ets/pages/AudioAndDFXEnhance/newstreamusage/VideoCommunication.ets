/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import router from '@ohos.router';
import audio from '@ohos.multimedia.audio'
import fs from '@ohos.file.fs';

@Entry
@Component
struct VideoCommunicationTest {
  private audioRendererOptions = {
    streamInfo: {
      samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_16000,
      channels: audio.AudioChannel.CHANNEL_1,
      sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
      encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    },
    rendererInfo: {
      content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
      usage: audio.StreamUsage.STREAM_USAGE_VIDEO_COMMUNICATION,
      rendererFlags: 0
    }
  }
  private audioRendererFilter = {
    rendererInfo : {
      content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
      usage : audio.StreamUsage.STREAM_USAGE_VIDEO_COMMUNICATION,
      rendererFlags: 0
    }
  }
  private audioRenderer = null
  private audioManager = null
  private audioRoutingManager = null
  @State bufferSize: number = 0
  private writeArr = 0
  @State returnMsg: string = ``
  public path: string = globalThis.pathDir + '/StarWars10s-1C-16000-2SW.wav'
  private outputDeviceList = []
  private selectOutputDeviceList = [{ value: "无数据" }]
  @State selectedOutputIndex: number = 0
  @State outputDeviceListTag: number = 0

  async aboutToAppear() {
    this.audioManager = audio.getAudioManager();
    this.audioRoutingManager = this.audioManager.getRoutingManager();
    if (this.audioManager !== null) {
      this.returnMsg += `getAudioManager : Success\n`
      this.audioRoutingManager = this.audioManager.getRoutingManager();
    } else {
      this.returnMsg += `getAudioManager : FAIL\n`
    }
    this.getSelectOutputDeviceList()
  }

  async getSelectOutputDeviceList() {
    let deviceFlag = audio.DeviceFlag.OUTPUT_DEVICES_FLAG
    try {
      this.outputDeviceList = await this.audioRoutingManager.getDevices(deviceFlag)
      if (this.outputDeviceList.length > 0) {
        this.selectOutputDeviceList = this.getDeviceList(this.outputDeviceList)
        this.outputDeviceListTag = 1
      }
      this.returnMsg = `getOutputDeviceList Success:${JSON.stringify(this.selectOutputDeviceList)}`
    } catch (err) {
      this.returnMsg = `getOutputDeviceList Fail:${JSON.stringify(err)}`
    }
  }

  getDeviceList(deviceDescriptors) {
    let deviceList = [];
    for (let i = 0;i < deviceDescriptors.length; i++) {
      let deviceTypeName = this.getDeviceTypeNameByValue(deviceDescriptors[i].deviceType)
      deviceList.push({ value: deviceTypeName })
    }
    return deviceList;
  }

  getDeviceTypeNameByValue(value) {
    for (let key in audio.DeviceType) {
      if (audio.DeviceType[key] == value) {
        return key;
      }
    }
  }

  async onBackPress() {
    if (this.audioRenderer == null) {
      this.returnMsg += `audioRenderer instance had not created\n`
      return
    }
    await this.audioRenderer.release()
    this.audioRenderer = null
  }

  async sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  rendererStart() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to start\n`
      return
    }
    let _this = this
    this.audioRenderer.start((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer start : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer start : SUCCESS, state:${_this.audioRenderer.state}\n`
      }
    });
  }

  async writeRenderer() {
    if (this.writeArr == 1) {
      this.returnMsg += `audioRenderer write already clicked`
      return
    }
    if (this.audioRenderer == null) {
      this.returnMsg += `audioRenderer instance had not created, dont‘t allow to read\n`
      return
    }
    if (this.bufferSize == 0) {
      this.bufferSize = await this.audioRenderer.getBufferSize()
    }
    this.writeArr = 1

    let _this = this
    let path = this.path
    try {
      let stat = await fs.stat(path);
      let len = stat.size % this.bufferSize == 0 ? Math.floor(stat.size / this.bufferSize) : Math.floor(stat.size / this.bufferSize + 1);
      let file = await fs.open(path, 0o0);
      let buf = new ArrayBuffer(this.bufferSize);
      this.returnMsg = `audioRenderer  write start.......... \n`;
      while (true) {
        for (let i = 0; i < len; i++) {
          let options = {
            offset: i * this.bufferSize,
            length: this.bufferSize
          }
          let readsize = await fs.read(file.fd, buf, options);
          let writeSize = await new Promise((resolve, reject) => {
            _this.audioRenderer.write(buf, (err, writeSize) => {
              if (err) {
                reject(err)
              } else {
                resolve(writeSize)
              }
            })
          })
        }
      }
    } catch (err) {
      this.returnMsg += `audioRenderer write : Error: ${JSON.stringify(err)}\n`
    }
  }

  stopRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to stop\n`
      return
    }
    let _this = this
    _this.audioRenderer.stop((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer stop : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer stop : SUCCESS, state:${_this.audioRenderer.state}\n`
      }
    })
  }

  pauseRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to pause\n`
      return
    }
    let _this = this
    _this.audioRenderer.pause((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer pause : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer pause : SUCCESS, state:${_this.audioRenderer.state}\n`
      }
    })
  }

  releaseRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to release\n`
      return
    }
    let _this = this
    _this.audioRenderer.release((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer release : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer release SUCCESS, state:${_this.audioRenderer.state}\n`
        _this.audioRenderer = null
        _this.writeArr = 0
      }
    })
  }

  async createRender() {
    try {
      this.audioRenderer = await audio.createAudioRenderer(this.audioRendererOptions)
      this.returnMsg += `audioRenderer create success\n`
    } catch (err) {
      this.returnMsg += `audioRenderer create : Error: ${JSON.stringify(err)}\n`
      return
    }
  }

  getRendererInfo() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to pause\n`
      return
    }
    let _this = this
    _this.audioRenderer.getRendererInfo((err, renderInfo) => {
      if (err) {
        _this.returnMsg += `getRendererInfo : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg += `getRendererInfo : Success, renderInfo.usage is: ${renderInfo.usage}\n`
      }
    });
  }

  getCurrentOutputDevices() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer instance had not created, dont‘t allow to pause\n`
      return
    }
    let _this = this

    _this.audioRenderer.getCurrentOutputDevices((err, outputDeviceDescription) => {
      if (err) {
        _this.returnMsg += `getCurrentOutputDevices : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg += `getCurrentOutputDevices : Success: ${JSON.stringify(outputDeviceDescription)}\n`
      }
    });
  }

  selectOutputDeviceByFilter() {
    let _this = this;
    if (this.outputDeviceList.length == 0) {
      _this.returnMsg = `没有输出设备类`
      return
    }
    let outputAudioDeviceDescriptor = this.outputDeviceList.filter((value, index) => {
      return index == _this.selectedOutputIndex
    })
    if (outputAudioDeviceDescriptor.length == 0) {
      _this.returnMsg = `尚未选择输出设备`
      return
    }
    this.audioRoutingManager.selectOutputDeviceByFilter(this.audioRendererFilter, outputAudioDeviceDescriptor)
      .then(() => {
        _this.returnMsg = `selectOutputDeviceByFilter promise Success, 所选的输出设备为: \n`
        _this.returnMsg += `${JSON.stringify(this.getDeviceList(outputAudioDeviceDescriptor))} \n`
      }).catch(err => {
      _this.returnMsg = `selectOutputDeviceByFilter promise Failed err:${JSON.stringify(err)}`
    })
  }

  build() {
    Column() {
      Row() {
        Column() {
          Text("返回数据：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 5 }).fontSize(18)
          Text(this.returnMsg).position({ x: 10, y: 30 }).fontSize(14)
        }.width('98%').height(220).backgroundColor(Color.Orange).position({ x: '1%' })
      }.position({ x: 0, y: 0 }).width('100%').zIndex(999)

      Scroll() {
        Column() {
          Text(`usage：${this.audioRendererOptions.rendererInfo.usage}`).fontSize(24)
          Row() {
            Button() {
              Text("创建AudioRender").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).onClick(() => this.createRender()).backgroundColor(Color.Pink)

            Button() {
              Text("start").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => {
              this.rendererStart()
            })
          }.margin({ top: 20, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          Row() {
            Button() {
              Text("write").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => {
              this.writeRenderer()
            })

            Button() {
              Text("pause").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => {
              this.pauseRenderer()
            })
          }.margin({ top: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          Row() {
            Button() {
              Text("stop").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => this.stopRenderer())

            Button() {
              Text("release").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => this.releaseRenderer())
          }.margin({ top: 10, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          Row() {
            Button() {
              Text("getRendererInfo").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => this.getRendererInfo())

            Button() {
              Text("getCurrentOutputDevices").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => this.getCurrentOutputDevices())
          }.margin({ top: 10, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          if (this.outputDeviceListTag == 1) {
            Row() {
              Text("输出设备").fontSize(20).margin({ left: 10 })
              Select(this.selectOutputDeviceList).value(this.selectOutputDeviceList[this.selectedOutputIndex].value)
                .onSelect((index, value) => {
                  this.selectedOutputIndex = index;
                })
            }.margin({ top: 10 }).width('100%')
          }

          Row() {
            Button() {
              Text("selectOutputDeviceByFilter").fontSize(22).fontColor(Color.Blue)
            }.width('49%').height(60).backgroundColor(Color.Pink).onClick(() => this.selectOutputDeviceByFilter())

          }.margin({ top: 10, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

        }.width('100%')
      }.margin({ top: 230 }).width('100%')
    }
  }
}