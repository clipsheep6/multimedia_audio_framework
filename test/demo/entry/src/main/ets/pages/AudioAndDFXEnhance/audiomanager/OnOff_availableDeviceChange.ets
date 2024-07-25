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

@Entry
@Component
struct OnOff_availableDeviceChange {
  @State returnMsg: string = ''
  @State Callback1Status: string = `未注册`
  @State Callback2Status: string = `未注册`
  @State returnCallback1Msg: string = ``
  @State returnCallback2Msg: string = ``
  private audioManager = null;
  private audioRoutingManager = null;

  @State selectedDeviceUsageKey: string = "请选择"
  @State selectedDeviceUsage: number = 2;
  private deviceUsageSelectList = []

  async aboutToAppear() {
    for (let key in audio.DeviceUsage) {
      this.deviceUsageSelectList.push({ value: key })
    }
    try {
      this.audioManager = audio.getAudioManager();
      this.returnMsg = `get audioManager success!`
    } catch (error) {
      this.returnMsg = `getAudioManager failed. error: ${JSON.stringify(error)}`;
    }
    try {
      this.audioRoutingManager = this.audioManager.getRoutingManager();
      this.returnMsg += `get audioRoutingManager success!`
    } catch (error) {
      this.returnMsg = `getRoutingManager failed. error: ${JSON.stringify(error)}`;
    }
  }

  async onBackPress() {
  }

 getDeviceChangeAction(deviceChangeInfo) {
   let str = ``;
   let deviceInfo = deviceChangeInfo;
   str += `deviceChangeType: ${deviceInfo.type}, `;
   for (let i = 0; i < deviceInfo.deviceDescriptors.length; i++) {
     let descriptor = deviceInfo.deviceDescriptors[i];
     str += `第${i}条\nId:${descriptor.id}, Type:${descriptor.deviceType}, Role:${descriptor.deviceRole}, Name:${descriptor.name},
      Address:${descriptor.address}, SampleRates:${descriptor.sampleRates[0]}, ChannelCounts:${descriptor.channelCounts[0]},
      ChannelMask:${descriptor.channelMasks[0]}, EncodingType:${descriptor.encodingTypes[0]}\n`;
   }
   return str;

  }

  callback1 = (callback) => {
    this.returnCallback1Msg = this.getDeviceChangeAction(callback);
    console.log("callback1: ", this.returnCallback1Msg)
  }
  callback2 = (callback) => {
    this.returnCallback2Msg = this.getDeviceChangeAction(callback);
  }

  build() {
    Column() {
      Row() {
        Column() {
          Text("Callback1状态：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 5 }).fontSize(18)
          Text(this.Callback1Status).fontWeight(FontWeight.Bolder).position({ x: 145, y: 5 }).fontSize(18)
          Text("Callback2状态：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 30 }).fontSize(18)
          Text(this.Callback2Status).fontWeight(FontWeight.Bolder).position({ x: 145, y: 30 }).fontSize(18)
        }.width('98%').height(65).backgroundColor(Color.Green).position({ x: '1%' })

        Column() {
          Text("Callback1返回值：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 5 }).fontSize(18)
          Text(this.returnCallback1Msg).position({ x: 10, y: 30 }).fontSize(14)
          Text("Callback2返回值：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 200 }).fontSize(18)
          Text(this.returnCallback2Msg).position({ x: 10, y: 230 }).fontSize(14)
          Text(this.returnMsg).fontWeight(FontWeight.Bolder).position({ x: 10, y: 370 }).fontSize(18)
        }.width('98%').height(410).backgroundColor(Color.Orange).position({ x: '1%', y: 70 })
      }.position({ x: 0, y: 3 }).width('100%').zIndex(999)

      Scroll() {
        Column() {
          Column() {
            Row() {
              Text("DeviceUsage选择:").fontSize(24)
              Select(this.deviceUsageSelectList)
                .value(this.selectedDeviceUsageKey)
                .onSelect((index, value) => {
                  this.selectedDeviceUsage = audio.DeviceUsage[value];
                  console.log(`selectedDeviceUsage: ${this.selectedDeviceUsage}`)
                  this.selectedDeviceUsageKey = value;
                  console.log(`selectedDeviceUsageKey: ${this.selectedDeviceUsageKey}`)
                })
            }
          }.width('100%')

          Row() {
            Button() {
              Text("监听Callback1").fontSize(20).fontColor(Color.White).textAlign(TextAlign.Center)
            }.width('50%').height(40).margin({ top: 10 })
            .onClick(() => {
              if (this.audioRoutingManager == null) {
                this.returnMsg = `audioRoutingManager is null,dont‘t allow to listener device change\n`
                return
              }

              let _this = this;
              try {
                this.audioRoutingManager.on('availableDeviceChange', this.selectedDeviceUsage, this.callback1);
                _this.Callback1Status = 'Callback1监听中.....\n'
              } catch (error) {
                this.returnMsg = `availableDeviceChange on called 1 error: ${JSON.stringify(error)}\n`
              }

            })

            Button() {
              Text("监听Callback2").fontSize(20).fontColor(Color.White).textAlign(TextAlign.Center)
            }.width('50%').height(40).margin({ top: 10 })
            .onClick(() => {
              if (this.audioRoutingManager == null) {
                this.returnMsg = `audioRoutingManager is null,dont‘t allow to listener device change\n`
                return
              }

              let _this = this;
              try {
                this.audioRoutingManager.on('availableDeviceChange', this.selectedDeviceUsage, this.callback2);
                _this.Callback2Status = 'Callback2监听中.....\n'
              } catch (error) {
                this.returnMsg = `availableDeviceChange on called 2 error: ${JSON.stringify(error)}\n`
              }
            })
          }.margin({ top: 5, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          Row() {
            Button() {
              Text("取消Callback1").fontSize(20).fontColor(Color.White).textAlign(TextAlign.Center)
            }.width('50%').height(40).margin({ top: 10 })
            .onClick(() => {
              if (this.audioRoutingManager == null) {
                this.returnMsg = `audioRoutingManager is null,dont‘t allow to listener device change\n`
                return
              }
              let _this = this;
              this.audioRoutingManager.off('availableDeviceChange', this.callback1);
              _this.Callback1Status = '已取消Callback1注册\n'
              _this.returnCallback1Msg = ''
            })

            Button() {
              Text("取消Callback2").fontSize(20).fontColor(Color.White).textAlign(TextAlign.Center)
            }.width('50%').height(40).margin({ top: 10 })
            .onClick(() => {
              if (this.audioRoutingManager == null) {
                this.returnMsg = `audioRoutingManager is null,dont‘t allow to listener device change\n`
                return
              }
              let _this = this;
              this.audioRoutingManager.off('availableDeviceChange', this.callback2);
              _this.Callback2Status = '已取消Callback2注册\n'
              _this.returnCallback2Msg = ''
            })
          }.margin({ top: 5, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)

          Row() {
            Button() {
              Text("取消所有监听").fontSize(20).fontColor(Color.White).textAlign(TextAlign.Center)
            }.width('100%').height(40).margin({ top: 5 })
            .onClick(() => {
              if (this.audioRoutingManager == null) {
                this.returnMsg = `audioRoutingManager is null,dont‘t allow to listener device change\n`
                return
              }
              let _this = this;
              this.audioRoutingManager.off('availableDeviceChange');
              _this.Callback1Status = '未注册\n'
              _this.Callback2Status = '未注册\n'
              _this.returnCallback1Msg = ''
              _this.returnCallback2Msg = ''
            })
          }.width('100%')
        }.width('100%')
      }.margin({ top: 490 }).width('100%')

    }
  }
}