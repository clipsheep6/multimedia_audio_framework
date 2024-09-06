/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import audio from '@ohos.multimedia.audio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'


describe("AudioVolumeManagerJsUnitTest", function () {
    let audioManager = audio.getAudioManager();
    let audioVolumeManager = audioManager.getVolumeManager();

    beforeAll(async function () {

        // input testsuit setup step，setup invoked before all testcases
        console.info('AudioVolumeManagerJsUnitTest:beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('AudioVolumeManagerJsUnitTest:afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('AudioVolumeManagerJsUnitTest:beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('AudioVolumeManagerJsUnitTest:afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_MANAGER_SYNC_001
     * @tc.desc:getVolumeGroupManagerSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_MANAGER_SYNC_001", 0, async function (done) {
        let groupid = audio.DEFAULT_VOLUME_GROUP_ID;
        try {
            let value = audioVolumeManager.getVolumeGroupManagerSync(groupid);
            console.info(`SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_MANAGER_SYNC_001 SUCCESS: ${value}.`);
            expect(typeof value).assertEqual('object');
            done();
        } catch (err) {
            console.error(`SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_MANAGER_SYNC_001 ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_INFOS_SYNC_001
     * @tc.desc:getVolumeGroupInfosSync success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_INFOS_SYNC_001", 0, async function (done) {
        try {
            let value = audioVolumeManager.getVolumeGroupInfosSync(audio.LOCAL_NETWORK_ID);
            console.info(`SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_INFOS_SYNC_001 SUCCESS: ${value}.`);
            expect(value.length).assertLarger(0);
            done();
        } catch (err) {
            console.error(`SUB_AUDIO_VOLUME_MANAGER_GET_VOLUME_GROUP_INFOS_SYNC_001 ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /**
     *@tc.number    : SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_ONOFF_VOLUMECHANGE_0100
     *@tc.name      : OnVolumeChange - setVolume - MEDIA
     *@tc.desc      : OnVolumeChange - setVolume - MEDIA
     *@tc.size      : MEDIUM
     *@tc.type      : Function
     *@tc.level     : Level 3
     */
    it('SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_ONOFF_VOLUMECHANGE_0100', 3, async function (done) {
        let audioVolumeManager = audioManager.getVolumeManager();
        let result = false;
        let maxVolume = await audioManager.getMaxVolume(audio.AudioVolumeType.MEDIA)
        let minVolume = await audioManager.getMinVolume(audio.AudioVolumeType.MEDIA)
        await audioManager.setVolume(audio.AudioVolumeType.MEDIA, maxVolume)
        audioVolumeManager.on('volumeChange', (VolumeEvent) => {
            console.info(`SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_ONOFF_VOLUMECHANGE_0100 VolumeEvent.volumeType:
                ${VolumeEvent.volumeType}, VolumeEvent.volume:${VolumeEvent.volume}, VolumeEvent.updateUi:${VolumeEvent.updateUi}`);
            expect(VolumeEvent.volumeType).assertEqual(audio.AudioVolumeType.MEDIA);
            expect(VolumeEvent.volume).assertEqual(minVolume);
            expect(VolumeEvent.updateUi).assertEqual(false);
            result = !result;
            done();
        });
        await audioManager.setVolume(audio.AudioVolumeType.MEDIA, minVolume)
        audioVolumeManager.off('volumeChange');
        await audioManager.setVolume(audio.AudioVolumeType.MEDIA, maxVolume)
        expect(result).assertTrue();
        done();
    })
   
    /**
     *@tc.number    : SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0100
     *@tc.name      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.desc      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.size      : MEDIUM
     *@tc.type      : Function
     *@tc.level     : Level 3
     */
    it('SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0100', 0, async function (done) {
        let audioVolumeManager = audioManager.getVolumeManager();
        try {
            audioVolumeManager.off(-1, (data) => {
                console.info(` off-VolumeChange Success! test fail!`);
                expect(false).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`off-VolumeChange input parameter type -1: ${error.message}, ${error.code}`);
            expect(Number(error.code)).assertEqual(401);
            done();
        }
    })
   
    /**
     *@tc.number    : SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0200
     *@tc.name      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.desc      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.size      : MEDIUM
     *@tc.type      : Function
     *@tc.level     : Level 3
     */
    it('SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0200', 0, async function (done) {
        let audioVolumeManager = audioManager.getVolumeManager();
        try {
            audioVolumeManager.off('123', (data) => {
                console.info(` off-VolumeChange Success! test fail!`);
                expect(false).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`off-VolumeChange invalid parameter type '123': ${error.message}, ${error.code}`);
            expect(Number(error.code)).assertEqual(6800101);
            done();
        }
    })
   
    /**
     *@tc.number    : SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0300
     *@tc.name      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.desc      : OffVolumeChange - off-ERROR-INPUT_NUMDER_PARAMETER
     *@tc.size      : MEDIUM
     *@tc.type      : Function
     *@tc.level     : Level 3
     */
    it('SUB_MULTIMEDIA_AUDIO_VOLUME_MANAGER_OFF_VOLUMECHANGE_0300', 3, async function (done) {
        let audioVolumeManager = audioManager.getVolumeManager();
        try {
            audioVolumeManager.off((data) => {
                console.info(` off-VolumeChange Success! test fail!`);
                expect(false).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`off-VolumeChange no input parameter: ${error.message}, ${error.code}`);
            expect(Number(error.code)).assertEqual(401);
            done();
        }
    })
})