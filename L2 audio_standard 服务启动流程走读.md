## L2 audio_standard 服务启动流程走读

### AudioPolicy服务启动流程

我以涉及到的文件作为代码走读的主轴

#### audio_policy_server.h

services\include\audio_policy\server\audio_policy_server.h

> ```c++
> class AudioPolicyServer : public SystemAbility, public AudioPolicyManagerStub, public AudioSessionCallback 
> ```



#### audio_policy_server.cpp

AudioPolicy服务继承自SystemAbility；当OnStart()被触发时，AudioPolicy服务开始启动

services\src\audio_policy\server\audio_policy_server.cpp

> ```c++
> void AudioPolicyServer::OnStart()
> {
>     //发布服务，客户端可以访问
>     bool res = Publish(this);
>     if (res) {
>         MEDIA_DEBUG_LOG("AudioPolicyService OnStart res=%d", res);
>     }
> 	
>     //服务初始化
>     mPolicyService.Init();
>     
>     RegisterAudioServerDeathRecipient();
>     return;
> }
> ```



#### audio_policy_service.h

展开AudioPolicy初始化的逻辑

services\src\audio_policy\server\service\include\audio_policy_service.h

> ```c++
> //构造函数会初始化一些后面用到的变量
> AudioPolicyService()
>         : mAudioPolicyManager(AudioPolicyManagerFactory::GetAudioPolicyManager()),
>           mConfigParser(ParserFactory::GetInstance().CreateParser(*this))
>     {
>         mDeviceStatusListener = std::make_unique<DeviceStatusListener>(*this);
>     }
> ```



#### audio_policy_service.cpp

进入 mPolicyService.Init();

services\src\audio_policy\server\service\src\audio_policy_service.cpp

> ```c++
> bool AudioPolicyService::Init(void)
> {
>     //大部分初始化逻辑在此
>     mAudioPolicyManager.Init();
>     if (!mConfigParser.LoadConfiguration()) {
>         MEDIA_ERR_LOG("Audio Config Load Configuration failed");
>         return false;
>     }
>     if (!mConfigParser.Parse()) {
>         MEDIA_ERR_LOG("Audio Config Parse failed");
>         return false;
>     }
> 
>     std::unique_ptr<AudioFocusParser> audioFocusParser;
>     audioFocusParser = make_unique<AudioFocusParser>();
>     std::string AUDIO_FOCUS_CONFIG_FILE = "/etc/audio/audio_interrupt_policy_config.xml";
> 
>     if (audioFocusParser->LoadConfig(focusTable_[0][0])) {
>         MEDIA_ERR_LOG("Audio Interrupt Load Configuration failed");
>         return false;
>     }
> 
>     auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
>     if (samgr == nullptr) {
>         MEDIA_ERR_LOG("[Policy Service] Get samgr failed");
>         return false;
>     }
> 
>     sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
>     if (object == nullptr) {
>         MEDIA_DEBUG_LOG("[Policy Service] audio service remote object is NULL.");
>         return false;
>     }
>     g_sProxy = iface_cast<IStandardAudioService>(object);
>     if (g_sProxy == nullptr) {
>         MEDIA_DEBUG_LOG("[Policy Service] init g_sProxy is NULL.");
>         return false;
>     } else {
>         MEDIA_DEBUG_LOG("[Policy Service] init g_sProxy is assigned.");
>     }
> 
>     if (mDeviceStatusListener->RegisterDeviceStatusListener(nullptr)) {
>         MEDIA_ERR_LOG("[Policy Service] Register for device status events failed");
>         return false;
>     }
> 
>     return true;
> }
> ```



#### audio_adapter_manager.cpp

进入mAudioPolicyManager.Init();  

*tips:我觉得这个变量命名不合理，这里是在连接 pulseaudio，命名为 mAudioAdapterManager更合理*

services\src\audio_policy\server\service\src\manager\audio_adapter_manager.cpp

> ```c++
> bool AudioAdapterManager::Init()
> {
>     std::unique_ptr<AudioAdapterManager> audioAdapterManager(this);
>     std::unique_ptr<PolicyCallbackImpl> policyCallbackImpl = std::make_unique<PolicyCallbackImpl>(audioAdapterManager);
>     mAudioServiceAdapter = AudioServiceAdapter::CreateAudioAdapter(std::move(policyCallbackImpl));
>     //连接 pulseaudio
>     bool result = mAudioServiceAdapter->Connect();
>     if (!result) {
>         MEDIA_ERR_LOG("[AudioAdapterManager] Error in connecting audio adapter");
>         return false;
>     }
>     bool isFirstBoot = false;
>     //初始化音频策略
>     InitAudioPolicyKvStore(isFirstBoot);
>     //初始化音量
>     InitVolumeMap(isFirstBoot);
>     //初始化响铃模式
>     InitRingerMode(isFirstBoot);
> 
>     return true;
> }
> ```



#### pulse_audio_service_adapter_impl.cpp

进入mAudioServiceAdapter->Connect();

frameworks\native\audioadapter\src\pulse_audio_service_adapter_impl.cpp

> ```c++
> bool PulseAudioServiceAdapterImpl::Connect()
> {
>     mMainLoop = pa_threaded_mainloop_new();
>     if (!mMainLoop) {
>         MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] MainLoop creation failed");
>         return false;
>     }
> 
>     if (pa_threaded_mainloop_start(mMainLoop) < 0) {
>         MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] Failed to start mainloop");
>         pa_threaded_mainloop_free (mMainLoop);
>         return false;
>     }
> 
>     pa_threaded_mainloop_lock(mMainLoop);
> 
>     while (true) {
>         pa_context_state_t state;
> 
>         if (mContext != NULL) {
>             state = pa_context_get_state(mContext);
>             if (state == PA_CONTEXT_READY)
>                 break;
>             // if pulseaudio is ready, retry connect to pulseaudio. before retry wait for sometime. reduce sleep later
>             usleep(PA_CONNECT_RETRY_SLEEP_IN_MICRO_SECONDS);
>         }
> 		//连接pulseaudio
>         bool result = ConnectToPulseAudio();
>         
>         if (!result || !PA_CONTEXT_IS_GOOD(pa_context_get_state(mContext))) {
>             continue;
>         }
> 
>         MEDIA_DEBUG_LOG("[PulseAudioServiceAdapterImpl] pa context not ready... wait");
> 
>         // Wait for the context to be ready
>         pa_threaded_mainloop_wait(mMainLoop);
>     }
> 
>     pa_threaded_mainloop_unlock(mMainLoop);
> 
>     return true;
> }
> ```



进入 ConnectToPulseAudio(); 

frameworks\native\audioadapter\src\pulse_audio_service_adapter_impl.cpp

> ```c++
> bool PulseAudioServiceAdapterImpl::ConnectToPulseAudio()
> {
>     unique_ptr<UserData> userData = make_unique<UserData>();
>     userData->thiz = this;
> 
>     if (mContext != NULL) {
>         pa_context_disconnect(mContext);
>         pa_context_set_state_callback(mContext, NULL, NULL);
>         pa_context_set_subscribe_callback(mContext, NULL, NULL);
>         pa_context_unref(mContext);
>     }
> 
>     pa_proplist *proplist = pa_proplist_new();
>     pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "PulseAudio Service");
>     pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "com.ohos.pulseaudio.service");
>     mContext = pa_context_new_with_proplist(pa_threaded_mainloop_get_api(mMainLoop), NULL, proplist);
>     pa_proplist_free(proplist);
> 
>     if (mContext == NULL) {
>         MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] creating pa context failed");
>         return false;
>     }
> 
>     pa_context_set_state_callback(mContext,  PulseAudioServiceAdapterImpl::PaContextStateCb, this);
>     //执行pa_context_connect连接pulseaudio，触发 PaContextStateCb 回调
>     if (pa_context_connect(mContext, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
>         if (pa_context_errno(mContext) == PA_ERR_INVALID) {
>             MEDIA_ERR_LOG("[PulseAudioServiceAdapterImpl] pa context connect failed: %{public}s",
>                 pa_strerror(pa_context_errno(mContext)));
>             goto Fail;
>         }
>     }
> 
>     return true;
> 
> Fail:
>     /* Make sure we don't get any further callbacks */
>     pa_context_set_state_callback(mContext, NULL, NULL);
>     pa_context_set_subscribe_callback(mContext, NULL, NULL);
>     pa_context_unref(mContext);
>     return false;
> }
> ```



进入PaContextStateCb 回调

frameworks\native\audioadapter\src\pulse_audio_service_adapter_impl.cpp

> ```c++
> void PulseAudioServiceAdapterImpl::PaContextStateCb(pa_context *c, void *userdata)
> {
>     PulseAudioServiceAdapterImpl *thiz = reinterpret_cast<PulseAudioServiceAdapterImpl*>(userdata);
> 
>     switch (pa_context_get_state(c)) {
>         case PA_CONTEXT_UNCONNECTED:
>         case PA_CONTEXT_CONNECTING:
>         case PA_CONTEXT_AUTHORIZING:
>         case PA_CONTEXT_SETTING_NAME:
>             break;
> 		//PA_CONTEXT_READY 表示已经连上pulseaudio
>         case PA_CONTEXT_READY: {
>             pa_context_set_subscribe_callback(c, PulseAudioServiceAdapterImpl::PaSubscribeCb, thiz);
> 			//注册回调用于监听各种事件-由入参掩码决定
>             pa_operation *operation = pa_context_subscribe(c, (pa_subscription_mask_t)
>                 (PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE |
>                 PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT |
>                 PA_SUBSCRIPTION_MASK_CARD), NULL, NULL);
>             if (operation == NULL) {
>                 pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
>                 return;
>             }
>             pa_operation_unref(operation);
>             pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
>             break;
>         }
> 
>         case PA_CONTEXT_FAILED:
>             pa_threaded_mainloop_signal(thiz->mMainLoop, 0);
>             return;
> 
>         case PA_CONTEXT_TERMINATED:
>         default:
>             return;
>     }
> }
> ```



#### 总结

至此，AudioPolicy服务完成了初始化流程，其中最重要的是

- 连接pulseaudio
- 初始化各类参数
- 发布服务使客户端可以调用

