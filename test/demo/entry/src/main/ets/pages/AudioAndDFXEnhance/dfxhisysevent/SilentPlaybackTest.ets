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
struct SilentPlaybackTest {
  private audioRendererOptions = {
    streamInfo: {
      samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_44100,
      channels: audio.AudioChannel.CHANNEL_2,
      sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
      encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    },
    rendererInfo: {
      content: audio.ContentType.CONTENT_TYPE_MUSIC,
      usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
      rendererFlags: 0
    }
  }
  private audioRenderer = null
  @State bufferSize: number = 0
  private writeArr = 0
  private streamTypeSelectList = []
  @State selectedVolumeTypeKey: string = "请选择"
  @State selectedContentTypeKey: string = "CONTENT_TYPE_MUSIC"
  @State selectedStreamUsageKey: string = "STREAM_USAGE_MEDIA"
  @State returnMsg: string = ``
  public pathValueList = []
  public pathList: string[] = ['/dfxTest_zero_data_59s.pcm', '/dfxTest_zero_data_60s.pcm', '/dfxTest_zero_data_65s.pcm']
  @State selectedPathKey: string = "dfxTest_zero_data_60s.pcm"
  public selectedpath: string = globalThis.pathDir + '/dfxTest_zero_data_60s.pcm'

  private renderInfo = {
    'RINGTONE': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_RINGTONE,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_RINGTONE'
      }
    },
    'MEDIA': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_MEDIA'
      }
    },
    'VOICE_CALL': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_VOICE_COMMUNICATION'
      }
    },
    'VOICE_ASSISTANT': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_VOICE_ASSISTANT,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_VOICE_ASSISTANT'
      }
    },
    'ALL': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_SPEECH,
        usage: audio.StreamUsage.STREAM_USAGE_MEDIA,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_SPEECH',
        selectedStreamUsageKey: 'STREAM_USAGE_MEDIA'
      }
    },
    'ALARM': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_ALARM,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_ALARM'
      }
    },
    'ACCESSIBILITY': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_ACCESSIBILITY,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_ACCESSIBILITY'
      }
    },
    'ULTRASONIC': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_ULTRASONIC,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_ULTRASONIC'
      }
    },
    '_MUSIC': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_MUSIC,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_MUSIC'
      }
    },
    '_MESSAGE': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_VOICE_MESSAGE,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_VOICE_MESSAGE'
      }
    },
    '_NOTIFICATION': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_NOTIFICATION,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_NOTIFICATION'
      }
    }
  ,
    '_SYSTEM': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_SYSTEM,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_SYSTEM'
      }
    },
    '_MOVIE': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_MOVIE,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_MOVIE'
      }
    },
    '_UNKNOWN': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_UNKNOWN,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_UNKNOWN'
      }
    }
  ,
    '_GAME': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_GAME,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_GAME'
      }
    },
    '_AUDIOBOOK': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_AUDIOBOOK,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_AUDIOBOOK'
      }
    },
    '_NAVIGATION': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_NAVIGATION,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_NAVIGATION'
      }
    }
  ,
    '_DTMF': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_DTMF,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_DTMF'
      }
    },
    '_ENFORCED_TONE': {
      main: {
        content: audio.ContentType.CONTENT_TYPE_UNKNOWN,
        usage: audio.StreamUsage.STREAM_USAGE_ENFORCED_TONE,
        rendererFlags: 0
      },
      info: {
        selectedContentTypeKey: 'CONTENT_TYPE_UNKNOWN',
        selectedStreamUsageKey: 'STREAM_USAGE_ENFORCED_TONE'
      }
    }
  }

  async aboutToAppear() {
    for (let key in audio.AudioVolumeType) {
      this.streamTypeSelectList.push({ value: key })
    }
    for (var i = 0; i < this.pathList.length; i++) {
      var key = this.pathList[i].toString();
      this.pathValueList.push({ value: key });
    }
    this.streamTypeSelectList.push({ value: '_MUSIC' })
    this.streamTypeSelectList.push({ value: '_MESSAGE' })
    this.streamTypeSelectList.push({ value: '_NOTIFICATION' })
    this.streamTypeSelectList.push({ value: '_SYSTEM' })
    this.streamTypeSelectList.push({ value: '_MOVIE' })
    this.streamTypeSelectList.push({ value: '_UNKNOWN' })
    this.streamTypeSelectList.push({ value: '_GAME' })
    this.streamTypeSelectList.push({ value: '_AUDIOBOOK' })
    this.streamTypeSelectList.push({ value: '_NAVIGATION' })
    this.streamTypeSelectList.push({ value: '_DTMF' })
    this.streamTypeSelectList.push({ value: '_ENFORCED_TONE' })
  };

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
      this.returnMsg = `audioRenderer  instance had not created,dont‘t allow to start\n`
      return
    }
    let _this = this
    this.audioRenderer.start((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer  start : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer  start : SUCCESS,state:${_this.audioRenderer.state}\n`
      }
    });
  }

  async writeRenderer() {
    if (this.writeArr == 1) {
      this.returnMsg += `audioRenderer  write already clicked`
      return
    }
    if (this.audioRenderer == null) {
      this.returnMsg += `audioRenderer  instance had not  ,dont‘t allow to read\n`
      return
    }
    if (this.bufferSize == 0) {
      this.bufferSize = await this.audioRenderer.getBufferSize()
    }
    this.writeArr = 1

    let _this = this
    let path = this.selectedpath
    try {
      let stat = await fs.stat(path);
      let len = stat.size % this.bufferSize == 0 ? Math.floor(stat.size / this.bufferSize) : Math.floor(stat.size / this.bufferSize + 1);
      let file = await fs.open(path, 0o0);
      let buf = new ArrayBuffer(this.bufferSize);
      this.returnMsg = `audioRenderer  write start.......... \n`;
      for (let i = 0;i < len; i++) {
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
      _this.returnMsg += `audioRenderer  write end, state:${_this.audioRenderer.state}\n`
    } catch (err) {
      this.returnMsg += `audioRenderer  write : Error: ${JSON.stringify(err)}\n`
    }
  }

  stopRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer  instance had not created,dont‘t allow to stop\n`
      return
    }
    let _this = this
    _this.audioRenderer.stop((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer  stop : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer  stop : SUCCESS,state:${_this.audioRenderer.state}\n`
      }
    })
  }

  pauseRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer  instance had not created,dont‘t allow to pause\n`
      return
    }
    let _this = this
    _this.audioRenderer.pause((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer pause : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer pause : SUCCESS,state:${_this.audioRenderer.state}\n`
      }
    })
  }

  releaseRenderer() {
    if (this.audioRenderer == null) {
      this.returnMsg = `audioRenderer  instance had not created,dont‘t allow to release\n`
      return
    }
    let _this = this
    _this.audioRenderer.release((err) => {
      if (err) {
        _this.returnMsg = `audioRenderer  release : Error: ${JSON.stringify(err)}\n`
      } else {
        _this.returnMsg = `audioRenderer release SUCCESS,state:${_this.audioRenderer.state}\n`
        _this.audioRenderer = null
        _this.writeArr = 0
      }
    })
  }

  async createRender() {
    try {
      this.audioRenderer = await audio.createAudioRenderer(this.audioRendererOptions)
      this.returnMsg += `audioRenderer create success \n`
    } catch (err) {
      this.returnMsg += `audioRenderer create : Error: ${JSON.stringify(err)}\n`
      return
    }
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
          Row() {
            Text("音频流选择:").fontSize(24)
            Select(this.streamTypeSelectList)
              .value(this.selectedVolumeTypeKey)
              .onSelect(async (index, value) => {
                this.audioRendererOptions.rendererInfo = this.renderInfo[this.selectedVolumeTypeKey].main
                this.selectedStreamUsageKey = this.renderInfo[this.selectedVolumeTypeKey].info.selectedStreamUsageKey
                this.selectedContentTypeKey = this.renderInfo[this.selectedVolumeTypeKey].info.selectedContentTypeKey
                if (this.audioRenderer !== null) {
                  await this.audioRenderer.release()
                  this.audioRenderer = null
                }
              })
          }

          Row() {
            Text("无声片源选择:").fontSize(24)
            Select(this.pathValueList).value(this.selectedPathKey)
              .onSelect((index, value) => {
                this.selectedPathKey = value
                console.log(`selectedPathKey: ${value}`)
                this.selectedpath = globalThis.pathDir + this.pathList[index]
                console.log(`path: ${this.selectedpath}`)
              }).font({ size: 22 })
          }.margin({ top: 20, bottom: 10 }).width('80%').justifyContent(FlexAlign.SpaceBetween)

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
              Text("write 无声数据").fontSize(22).fontColor(Color.Blue)
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

        }.width('100%')
      }.margin({ top: 230 }).width('100%')
    }
  }
}