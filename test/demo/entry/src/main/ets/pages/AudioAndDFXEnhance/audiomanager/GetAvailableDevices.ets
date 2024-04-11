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
struct GetAvailableDevices {
  @State returnMsg: string = ''
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

  getInfoArray(deviceInfo) {
    let str = ``;
    for (let i = 0; i < deviceInfo.length; i++) {
      let descriptor = deviceInfo[i];
      str += `第${i}条\nId:${descriptor.id}, Type:${descriptor.deviceType}, Role:${descriptor.deviceRole}, Name:${descriptor.name},
      Address:${descriptor.address}, SampleRates:${descriptor.sampleRates[0]}, ChannelCounts:${descriptor.channelCounts[0]},
      ChannelMask:${descriptor.channelMasks[0]}, EncodingType:${descriptor.encodingTypes[0]}\n`;
    }
    return str;
  }

  getAvailableDevices() {
    try {
      let audioDeviceDescriptors = this.audioRoutingManager.getAvailableDevices(this.selectedDeviceUsage);
      this.returnMsg = `getAvailableDevices DeviceUsage(${this.selectedDeviceUsage}) success, return: ${this.getInfoArray(audioDeviceDescriptors)}`
    } catch (error) {
      this.returnMsg = `getAvailableDevices failed. error: ${JSON.stringify(error)}`;
    }
  }

  build() {
    Row() {
      Row() {
        Column() {
          Text("【获取可用设备】返回数据：").fontWeight(FontWeight.Bolder).position({ x: 10, y: 5 }).fontSize(18)
          Text(this.returnMsg).position({ x: 10, y: 30 }).fontSize(14)
        }.width('98%').height(550).backgroundColor(Color.Orange).position({ x: '1%' })
      }.position({ x: 0, y: 0 }).width('100%').zIndex(999)

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

          Divider().strokeWidth(10).color(Color.Blue).margin({ top: 20 })

          Column() {
            Row() {
              Row() {
                Button() {
                  Text("getAvailableDevices同步接口").fontSize(22).fontColor(Color.Blue)
                }.width('98%').height(60).onClick(() => this.getAvailableDevices()).backgroundColor(Color.Pink)
              }.margin({ top: 20, bottom: 10 }).width('100%').justifyContent(FlexAlign.SpaceBetween)
            }.width('100%')

          }.width('100%').margin({ top: 20 })
        }
      }.width('100%').margin({ top: 550 })
    }.height('100%').width('100%');

  }
}