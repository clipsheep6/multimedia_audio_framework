/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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
import {ErrorCallback, AsyncCallback, Callback} from './basic';
import {VideoPlayer, AudioPlayer} from './@ohos.multimedia.media'
import Context from './@ohos.ability';
/**
 * @name audio
 * @since 6
 * @sysCap SystemCapability.Multimedia.Audio
 * @import import audio from '@ohos.Multimedia.audio';
 * @permission
 */
declare namespace audio {

  /**
   * Obtains an AudioManager instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function getAudioManager(): AudioManager;

  /**
   * Obtains an AudioCapturer instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function createAudioCapturer(volumeType: AudioVolumeType): AudioCapturer;

  /**
   * Obtains an AudioRenderer instance.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  function createAudioRenderer(volumeType: AudioVolumeType): AudioRenderer;

  /**
   * Enumerates the rendering states of the current device.
   * @since 8
   */
  enum AudioState {
    /**
     * Invalid state.
     * @since 8
     */
    STATE_INVALID = -1,
    /**
     * Create New instance state.
     * @since 8
     */
    STATE_NEW,
    /**
     * Prepared state.
     * @since 8
     */
    STATE_PREPARED,
    /**
     * Running state.
     * @since 8
     */
    STATE_RUNNING,
    /**
     * Stopped state.
     * @since 8
     */
    STATE_STOPPED,
    /**
     * Released state.
     * @since 8
     */
    STATE_RELEASED,
    /**
     * Paused state.
     * @since 8
     */
    STATE_PAUSED
  }

  /**
   * Enumerates audio stream types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioVolumeType {
    /**
     * Audio streams for voice calls
     */
    VOICE_CALL = 0,
/**
     * Audio streams for ring tones
     */
    RINGTONE = 2,
    /**
     * Audio streams for media purpose
     */
    MEDIA = 3,
    /**
     * Audio stream for voice assistant
     */
    VOICE_ASSISTANT = 5,
  }

  /**
   * Enumerates audio device flags.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceFlag {
    /**
     * Output devices
     */
    OUTPUT_DEVICES_FLAG = 1,
    /**
     * Input devices
     */
    INPUT_DEVICES_FLAG = 2,
    /**
     * All devices
     */
    ALL_DEVICES_FLAG = 3,
  }
  /**
   * Enumerates device roles.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceRole {
    /**
     * Input role
     */
    INPUT_DEVICE = 1,
    /**
     * Output role
     */
    OUTPUT_DEVICE = 2,
  }
  /**
   * Enumerates device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum DeviceType {
    /**
     * Invalid device
     */
    INVALID = 0,
    /**
     * earpiece
     */
    EARPIECE = 1,
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Wired headset
     */
    WIRED_HEADSET = 3,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
    /**
     * Bluetooth device using advanced audio distribution profile (A2DP)
     */
    BLUETOOTH_A2DP = 8,
    /**
     * Microphone
     */
    MIC = 15,
  }
  /**
   * Enumerates Active device types.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
   enum ActiveDeviceType {
    /**
     * Speaker
     */
    SPEAKER = 2,
    /**
     * Bluetooth device using the synchronous connection oriented link (SCO)
     */
    BLUETOOTH_SCO = 7,
  }
  /**
   * Enumerates Audio Ringer modes
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  enum AudioRingMode {
    /**
     * Silent mode
     */
    RINGER_MODE_SILENT = 0,
    /**
     * Vibration mode
     */
    RINGER_MODE_VIBRATE,
	 /**
     * Normal mode
     */
    RINGER_MODE_NORMAL,
  }

  /**
   * Enumerates the sample format.
   */
  enum AudioSampleFormat {
    SAMPLE_U8 = 1,
    SAMPLE_S16LE = 2,
    SAMPLE_S24LE = 3,
    SAMPLE_S32LE = 4,
    INVALID_WIDTH = -1
  }

  /**
   * Enumerates the audio channel.
   */
  enum AudioChannel {
    MONO = 1,
    STEREO
  }

  /**
   * Enumerates the audio sampling rate.
   */
  enum AudioSamplingRate {
    SAMPLE_RATE_8000 = 8000,
    SAMPLE_RATE_11025 = 11025,
    SAMPLE_RATE_12000 = 12000,
    SAMPLE_RATE_16000 = 16000,
    SAMPLE_RATE_22050 = 22050,
    SAMPLE_RATE_24000 = 24000,
    SAMPLE_RATE_32000 = 32000,
    SAMPLE_RATE_44100 = 44100,
    SAMPLE_RATE_48000 = 48000,
    SAMPLE_RATE_64000 = 64000,
    SAMPLE_RATE_96000 = 96000
  }

  /**
   * Enumerates the audio encoding type.
   */
  enum AudioEncodingType {
    ENCODING_PCM = 0,
    ENCODING_INVALID
  }

  /**
   * Enumerates the audio content type.
   */
  enum ContentType {
    CONTENT_TYPE_UNKNOWN = 0,
    CONTENT_TYPE_SPEECH = 1,
    CONTENT_TYPE_MUSIC = 2,
    CONTENT_TYPE_MOVIE = 3,
    CONTENT_TYPE_SONIFICATION = 4,
    CONTENT_TYPE_RINGTONE = 5,
  }

  /**
   * Enumerates the stream usage.
   */
  enum StreamUsage {
    STREAM_USAGE_UNKNOWN = 0,
    STREAM_USAGE_MEDIA = 1,
    STREAM_USAGE_VOICE_COMMUNICATION = 2,
    STREAM_USAGE_NOTIFICATION_RINGTONE = 3,
  }

  /**
   * Interface for audio renderer info
   */
  interface AudioRendererInfo {
    /**
     * Audio content type
     */
    content: ContentType;
    /**
     * Audio stream usage
     */
    usage: StreamUsage;
    /**
     * Audio renderer flags
     */
    rendererFlags: number;
  }


  /**
   * Enumerates the audio render rate.
   */
  enum AudioRendererRate {
    RENDER_RATE_NORMAL = 0,
    RENDER_RATE_DOUBLE = 1,
    RENDER_RATE_HALF = 2,
  }

  /**
   * Enumerates audio interruption event types.
   * @devices phone, tablet, tv, wearable, car
   * @since 7
   * @SysCap SystemCapability.Multimedia.Audio
   */
  enum InterruptType {
    /**
     * An audio interruption event starts.
     */
    INTERRUPT_TYPE_BEGIN = 1,

    /**
     * An audio interruption event ends.
     */
    INTERRUPT_TYPE_END = 2
  }

  /**
   * Enumerates the types of hints for audio interruption.
   * @devices phone, tablet, tv, wearable, car
   * @since 7
   * @SysCap SystemCapability.Multimedia.Audio
   */
  enum InterruptHint {
    INTERRUPT_HINT_NONE = 0,
    /**
     * Audio resumed.
     */
    INTERRUPT_HINT_RESUME = 1,

    /**
     * Audio paused.
     */
    INTERRUPT_HINT_PAUSE = 2,

    /**
     * Audio stopped.
     */
    INTERRUPT_HINT_STOP = 3,

    /**
     * Audio ducking. (In ducking, the audio volume is reduced, but not silenced.)
     */
    INTERRUPT_HINT_DUCK = 4,

    /**
     * Audio unducking.
     */
    INTERRUPT_HINT_UNDUCK = 5,
  }

  /**
   * Interrupt force type.
   * @since 8
   */
  enum InterruptForceType {
    /**
     * Force type, system change audio state.
     */
    INTERRUPT_FORCE = 0,
    /**
     * Share type, application change audio state.
     */
    INTERRUPT_SHARE
  }

  interface InterruptEvent {
    /**
     * Interrupt event type, begin or end
     */
    eventType: InterruptType;

    /**
     * Interrupt force type, force or share
     */
    forceType: InterruptForceType;

    /**
     * Interrupt hint type. In force type, the audio state already changed,
     * but in share mode, only provide a hint for application to decide.
     */
    hintType: InterruptHint;
  }

  enum DeviceChangeType {
    CONNECT = 0,
    DISCONNECT = 1,
  }

  /**
   * Enumerates audio scenes.
   * @since 8
  */
  enum AudioScene {
    /**
     * Default audio scene
     * @since 8
     */
    AUDIO_SCENE_DEFAULT,
    /**
     * Ringing audio scene
     * Only available for system api.
     * @since 8
     */
    AUDIO_SCENE_RINGING,
    /**
     * Phone call audio scene
     * Only available for system api.
     * @since 8
     */
    AUDIO_SCENE_PHONE_CALL,
    /**
     * Voice chat audio scene
     * @since 8
     */
    AUDIO_SCENE_VOICE_CHAT
  }

  /**
   * Manages audio volume and audio device information.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioManager {
    /**
     * Sets volume for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number, callback: AsyncCallback<void>): void;
    /**
     * Sets volume for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setVolume(volumeType: AudioVolumeType, volume: number): Promise<void>;
    /**
     * Obtains volume of a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the volume of a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the minimum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMinVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType, callback: AsyncCallback<number>): void;
    /**
     * Obtains the maximum volume allowed for a stream. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getMaxVolume(volumeType: AudioVolumeType): Promise<number>;
    /**
     * Sets the stream to mute. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    mute(volumeType: AudioVolumeType, mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Sets the stream to mute. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    mute(volumeType: AudioVolumeType, mute: boolean): Promise<void>;
	  /**
     * Checks whether the stream is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMute(volumeType: AudioVolumeType): Promise<boolean>;
    /**
     * Checks whether the stream is active. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the stream is active. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isActive(volumeType: AudioVolumeType): Promise<boolean>;
	  /**
     * Mute/Unmutes the microphone. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean, callback: AsyncCallback<void>): void;
    /**
     * Mute/Unmutes the microphone. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setMicrophoneMute(mute: boolean): Promise<void>;
    /**
     * Checks whether the microphone is muted. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the microphone is muted. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    isMicrophoneMute(): Promise<boolean>;
	  /**
     * Sets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode, callback: AsyncCallback<void>): void;
    /**
     * Sets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRingerMode(mode: AudioRingMode): Promise<void>;
	  /**
     * Gets the ringer mode. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(callback: AsyncCallback<AudioRingMode>): void;
    /**
     * Gets the ringer mode. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRingerMode(): Promise<AudioRingMode>;
	  /**
     * Sets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string, callback: AsyncCallback<void>): void;
    /**
     * Sets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioParameter(key: string, value: string): Promise<void>;
	  /**
     * Gets the audio parameter. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string, callback: AsyncCallback<string>): void;
    /**
     * Gets the audio parameter. This method uses a promise to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioParameter(key: string): Promise<string>;
    /**
     * Obtains the audio devices of a specified flag. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag, callback: AsyncCallback<AudioDeviceDescriptors>): void;
    /**
     * Obtains the audio devices with a specified flag. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getDevices(deviceFlag: DeviceFlag): Promise<AudioDeviceDescriptors>;
	  /**
     * Activates the device. This method uses an asynchronous callback to return the execution result.
	   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean, callback: AsyncCallback<void>): void;
    /**
     * Activates the device. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    setDeviceActive(deviceType: ActiveDeviceType, active: boolean): Promise<void>;
    /**
     * Checks whether the device is active. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType, callback: AsyncCallback<boolean>): void;
    /**
     * Checks whether the device is active. This method uses a promise to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    isDeviceActive(deviceType: ActiveDeviceType): Promise<boolean>;
    /**
     * Subscribes volume change event callback, only for system
     * @return VolumeEvent callback.
     * @sysCap SystemCapability.Multimedia.Audio
     * @since 8
     * @devices
     */
    on(type: 'volumeChange', callback: Callback<VolumeEvent>): void;
    /**
     * Monitors ringer mode change
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    on(type: 'ringerModeChange', callback: Callback<AudioRingMode>): void;
    /**
     * Sets the audio scene mode to change audio strategy.
     * This method uses an asynchronous callback to return the execution result.
   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioScene(scene: AudioScene, callback: AsyncCallback<void> ): void;
    /**
     * Sets the audio scene mode to change audio strategy. This method uses a promise to return the execution result.
   * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setAudioScene(scene: AudioScene): Promise<void>;
    /**
     * Obtains the system audio scene mode. This method uses an asynchronous callback to return the execution result.
     * @sysCap SystemCapability.Multimedia.Audio
     * @devices
     */
    getAudioScene(callback: AsyncCallback<AudioScene> ): void;
     /**
      * Obtains the system audio scene mode. This method uses a promise to return the execution result.
      * @devices
      * @sysCap SystemCapability.Multimedia.Audio
      */
    getAudioScene(): Promise<AudioScene>;
    /**
    * Monitors device changes
    * @sysCap SystemCapability.Multimedia.Audio
    * @devices
    */
    on(type: 'deviceChange', callback: Callback<DeviceChangeAction>): void;
  }

  /**
   * Describes an audio device.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioDeviceDescriptor {
    /**
     * Audio device role
     * @devices
     */
    readonly deviceRole: DeviceRole;
    /**
     * Audio device type
     * @devices
     */
    readonly deviceType: DeviceType;
  }

    /**
     * Audio volume event
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
  interface VolumeEvent {
    /**
     * volumeType of current stream
     * @devices
     */
    volumeType: AudioVolumeType;
    /**
     * volume level
     * @devices
     */
    volume: number;
    /**
     * updateUi show volume change in Ui
     * @devices
     */
    updateUi: boolean;
  }

  interface DeviceChangeAction {
    type: DeviceChangeType;
    deviceDescriptors: AudioDeviceDescriptors;
  }

  /**
   * Provides functions for applications for audio playback.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioRenderer {
    /**
     * Gets audio state.
     * @since 8
     */
    readonly state: AudioState;
    /**
     * Sets audio render parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters, callback: AsyncCallback<void>): void;
    /**
     * Sets audio render parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters): Promise<void>;

    /**
     * Obtains audio render parameters. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(callback: AsyncCallback<AudioParameters>): void;
    /**
     * Obtains audio render parameters. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(): Promise<AudioParameters>;

    /**
     * Starts audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(callback: AsyncCallback<boolean>): void;
    /**
     * Starts audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(): Promise<boolean>;

    /**
     * Render audio data. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    write(buffer: ArrayBuffer, callback: AsyncCallback<number>): void;
    /**
     * Render audio data. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    write(buffer: ArrayBuffer): Promise<number>;

    /**
     * Obtains the current timestamp. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(callback: AsyncCallback<number>): void;
    /**
     * Obtains the current timestamp. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(): Promise<number>;

    /**
     * Drain renderer buffer. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    drain(callback: AsyncCallback<boolean>): void;
    /**
     * Drain renderer buffer. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    drain(): Promise<boolean>;

    /**
     * Pauses audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    pause(callback: AsyncCallback<boolean>): void;
    /**
     * Pauses audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    pause(): Promise<boolean>;

    /**
     * Stops audio rendering. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(callback: AsyncCallback<boolean>): void;
    /**
     * Stops audio rendering. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(): Promise<boolean>;

    /**
     * Releases resources. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(callback: AsyncCallback<boolean>): void;
    /**
     * Releases resources. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(): Promise<boolean>;

    /**
     * Obtains a reasonable minimum buffer size for renderer, however, the renderer can
     * accept other read sizes as well. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(callback: AsyncCallback<number>): void;
    /**
     * Obtains a reasonable minimum buffer size for renderer, however, the renderer can
     * accept other read sizes as well. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(): Promise<number>;

    /**
     * Set the render rate. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRenderRate(rate: AudioRendererRate, callback: AsyncCallback<number>): void;
    /**
     * Set the render rate. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setRenderRate(rate: AudioRendererRate): Promise<number>;

    /**
     * Obtains the current render rate. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRenderRate(callback: AsyncCallback<AudioRendererRate>): void;
    /**
     * Obtains the current render rate. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getRenderRate(): Promise<AudioRendererRate>;
    /**
     * Subscribes mark reach event callback.
     * @param type Event type.
     * @param frame Mark reach frame count.
     * @return Mark reach event callback.
     * @since 8
     */
    on(type: "interrupt", callback: Callback<InterruptEvent>): void;
  }

  /**
   * Provides functions for applications to manage audio capturing.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioCapturer {
    /**
     * Sets audio capture parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters, callback: AsyncCallback<void>): void;
    /**
     * Sets audio capture parameters.
     * If set parameters is not called explicitly, then 16Khz sampling rate, mono channel and PCM_S16_LE format will
     * be set by default. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    setParams(params: AudioParameters): Promise<void>;

    /**
     * Obtains audio capture parameters. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(callback: AsyncCallback<AudioParameters>): void;
    /**
     * Obtains audio capture parameters. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getParams(): Promise<AudioParameters>;

    /**
     * Starts audio capturing. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(callback: AsyncCallback<boolean>): void;
    /**
     * Starts audio capturing. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    start(): Promise<boolean>;

    /**
     * Capture audio data. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    read(size: number, isBlockingRead: boolean, callback: AsyncCallback<ArrayBuffer>): void;
    /**
     * Capture audio data. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    read(size: number, isBlockingRead: boolean): Promise<ArrayBuffer>;

    /**
     * Obtains the current timestamp. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(callback: AsyncCallback<number>): void;
    /**
     * Obtains the current timestamp. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getAudioTime(): Promise<number>;

    /**
     * Stops audio capturing. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(callback: AsyncCallback<boolean>): void;
    /**
     * Stops audio capturing. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    stop(): Promise<boolean>;

    /**
     * Releases a capture resources. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(callback: AsyncCallback<boolean>): void;
    /**
     * Releases a capture resources. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    release(): Promise<boolean>;

    /**
     * Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     * accept other read sizes as well. This method uses an asynchronous callback to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(callback: AsyncCallback<number>): void;
    /**
     * Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     * accept other read sizes as well. This method uses a promise to return the execution result.
     * @devices
     * @sysCap SystemCapability.Multimedia.Audio
     */
    getBufferSize(): Promise<number>;
  }

  /**
   * Structure for audio parameters
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  interface AudioParameters {
    /**
     * Audio sample format
     * @devices
     */
    format: AudioSampleFormat;
    /**
     * Audio channels
     * @devices
     */
    channels: AudioChannel;
    /**
     * Audio sampling rate
     * @devices
     */
    samplingRate: AudioSamplingRate;
    /**
     * Audio encoding type
     * @devices
     */
    encoding: AudioEncodingType;
    /**
     * Audio content type
     * @devices
     */
    contentType: ContentType;
    /**
     * Audio stream usage
     * @devices
     */
    usage: StreamUsage;
    /**
     * Audio device role
     * @devices
     */
    deviceRole: DeviceRole;
    /**
     * Audio device type
     * @devices
     */
    deviceType: DeviceType;
  }

  /**
   * A queue of AudioDeviceDescriptor, which is read-only.
   * @devices
   * @sysCap SystemCapability.Multimedia.Audio
   */
  type AudioDeviceDescriptors = Array<Readonly<AudioDeviceDescriptor>>;

  enum RingtoneType {
    /**Default type */
    RINGTONE_TYPE_DEFAULT = 0,
    /**Multi-sim type */
    RINGTONE_TYPE_MULTISIM
  }
  interface RingtoneOptions {
    volume: number;
    loop: boolean;
  }
  interface RingtonePlayer {
    /**
     * Gets render state of ringtone.
     * @sysCap SystemCapability.Multimedia.Audio
     */
    readonly state: AudioState;

    /**
     * Gets the title of ringtone.
     * @since 1.0
     */
    getTitle(callback: AsyncCallback<string>): void;
    getTitle(): Promise<string>;

    /**
     * Gets audio renderer info.
     * @return AudioRendererInfo value
     * @since 1.0
     */
    getAudioRendererInfo(callback: AsyncCallback<AudioRendererInfo>): void;
    getAudioRendererInfo(): Promise<AudioRendererInfo>;

    /**
     * Sets ringtone parameters.
     * @param option Set RingtoneOption for ringtone like volume & loop
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    configure(option: RingtoneOptions, callback: AsyncCallback<void>): void;
    configure(option: RingtoneOptions): Promise<void>;
    /**
     * Starts playing ringtone.
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    start(callback: AsyncCallback<void>): void;
    start(): Promise<void>;
    /**
     * Stops playing ringtone.
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    stop(callback: AsyncCallback<void>): void;
    stop(): Promise<void>;
    /**
     * Release ringtone player resource.
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    release(callback: AsyncCallback<void>): void;
    release(): Promise<void>;
  }
  function getSystemSoundManager(): SystemSoundManager;
  interface SystemSoundManager {
    /**
     * Sets the ringtone uri.
     * @param context Indicates the Context object on OHOS
     * @param uri Indicated which uri to be set for the tone type
     * @param type Indicats the type of the tone
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    setSystemRingtoneUri(context: Context, uri: string, type: RingtoneType, callback: AsyncCallback<void>): void;
    setSystemRingtoneUri(context: Context, uri: string, type: RingtoneType): Promise<void>;
    /**
     * Sets the ringtone uri.
     * @param context Indicates the Context object on OHOS
     * @param type Indicats the type of the tone
     * @param callback Callback object to be passed along with request
     * @return Returns uri of the ringtone
     * @since 1.0
     * @version 1.0
     */
    getSystemRingtoneUri(context: Context, type: RingtoneType, callback: AsyncCallback<string>): void;
    getSystemRingtoneUri(context: Context, type: RingtoneType): Promise<string>;
    /**
     * Gets the ringtone player.
     * @param context Indicates the Context object on OHOS
     * @param type Indicats the type of the tone
     * @param callback Callback object to be passed along with request
     * @return Returns ringtone player object
     * @since 1.0
     * @version 1.0
     */
    getSystemRingtonePlayer(context: Context, type: RingtoneType, callback: AsyncCallback<RingtonePlayer>): void;
    getSystemRingtonePlayer(context: Context, type: RingtoneType): Promise<RingtonePlayer>;
    /**
     * Sets the notification uri.
     * @param context Indicates the Context object on OHOS
     * @param uri Indicats the uri of the notification
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    setSystemNotificationUri(context: Context, uri: string, callback: AsyncCallback<void>): void;
    setSystemNotificationUri(context: Context, uri: string): Promise<void>;
    /**
     * Gets the notification uri.
     * @param context Indicates the Context object on OHOS
     * @param callback Callback object to be passed along with request
     * @return Returns the uri of the notification
     * @since 1.0
     * @version 1.0
     */
    getSystemNotificationUri(context: Context, callback: AsyncCallback<string>): void;
    getSystemNotificationUri(context: Context): Promise<string>;
    /**
     * Sets the alarm uri.
     * @param context Indicates the Context object on OHOS
     * @param uri Indicats the uri of the alarm
     * @param callback Callback object to be passed along with request
     * @since 1.0
     * @version 1.0
     */
    setSystemAlarmUri(context: Context, uri: string, callback: AsyncCallback<void>): void;
    setSystemAlarmUri(context: Context, uri: string): Promise<void>;
    /**
     * Gets the alarm uri.
     * @param context Indicates the Context object on OHOS
     * @param callback Callback object to be passed along with request
     * @return Returns the uri of the alarm
     * @since 1.0
     * @version 1.0
     */
    getSystemAlarmUri(context: Context, callback: AsyncCallback<string>): void;
    getSystemAlarmUri(context: Context): Promise<string>;
  }
}
export default audio;