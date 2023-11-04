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


describe("AudioSpatializationManagerJsUnitTest", function () {
    let audioManager = audio.getAudioManager();
    let audioSpatializationManager = audioManager.getSpatializationManager();
    const ERROR_INPUT_INVALID = '401';
    const ERROR_INVALID_PARAM = '6800101';

    beforeAll(async function () {

        // input testsuit setup step，setup invoked before all testcases
        console.info('beforeAll called')
    })

    afterAll(function () {

        // input testsuit teardown step，teardown invoked after all testcases
        console.info('afterAll called')
    })

    beforeEach(function () {

        // input testcase setup step，setup invoked before each testcases
        console.info('beforeEach called')
    })

    afterEach(function () {

        // input testcase teardown step，teardown invoked after each testcases
        console.info('afterEach called')
    })

    /*
     * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001
     * @tc.desc:isSpatializationEnabled success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001", 0, async function (done) {
        try {
            let isEnabled = audioSpatializationManager.isSpatializationEnabled();
            console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001 SUCCESS: ${isEnabled}.`);
            expect(isEnabled).assertTrue();
            done();
        } catch (err) {
            console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_SPATIALIZATION_ENABLED_001 ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })

    /*
     * @tc.name:SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001
     * @tc.desc:isSpatializationEnabled success
     * @tc.type: FUNC
     * @tc.require: I7V04L
     */
    it("SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001", 0, async function (done) {
        try {
            let isEnabled = audioSpatializationManager.isHeadTrackingEnabled();
            console.info(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001 SUCCESS: ${isEnabled}.`);
            expect(isEnabled).assertTrue();
            done();
        } catch (err) {
            console.error(`SUB_AUDIO_SPATIALIZATION_MANAGER_IS_HEAD_TRACKING_ENABLED_001 ERROR: ${err}`);
            expect(false).assertTrue();
            done();
        }
    })
})