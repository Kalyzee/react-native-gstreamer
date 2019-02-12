#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <gst/gst.h>
#include <gstreamer_backend.h>
#include <pthread.h>

ANativeWindow *native_window;
static RctGstUserData *userData;

// Java callbacks
static jmethodID on_player_init_id;
static jmethodID on_state_changed_id;
static jmethodID on_playing_progress_id;
static jmethodID on_volume_changed_id;
static jmethodID on_uri_changed_id;
static jmethodID on_eos_id;
static jmethodID on_element_error_id;

// Audio Stucture
static jclass JavaRctGstAudioLevel;
static jfieldID JavaRctGstAudioLevelRms;
static jfieldID JavaRctGstAudioLevelPeak;
static jfieldID JavaRctGstAudioLevelDecay;

// Global context
pthread_t gst_app_thread;
static pthread_key_t current_jni_env;
jobject app;
static JavaVM *jvm;

/* Register this thread with the VM */
static JNIEnv *attach_current_thread(void) {
    JNIEnv *env;
    JavaVMAttachArgs args;

    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if ((*jvm)->AttachCurrentThread(jvm, &env, &args) < 0) {
        GST_ERROR("Failed to attach current thread");
        return NULL;
    }

    return env;
}

/* Unregister this thread from the VM */
static void detach_current_thread(JNIEnv *env) {
    GST_DEBUG("Detaching thread %p", g_thread_self());
    (*jvm)->DetachCurrentThread(jvm);
    (*env)->DeleteGlobalRef(env, JavaRctGstAudioLevel);
}

/* Retrieve the JNI environment for this thread */
static JNIEnv *get_jni_env(void) {
    JNIEnv *env;

    if ((env = pthread_getspecific(current_jni_env)) == NULL) {
        env = attach_current_thread();
        pthread_setspecific(current_jni_env, env);
    }

    return env;
}

static RctGstUserData* get_user_data() {
    if (!userData) {
        __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "Creating user data");
        userData = rct_gst_init_user_data();
    }

    return userData;
}

// Callbacks
void native_on_player_init(void *owner)
{
    __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "native_on_player_init");

    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_player_init_id);
}

void native_on_state_changed(void *owner, GstState old_state, GstState new_state)
{
    __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "native_on_state_changed %s -> %s", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));

    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_state_changed_id, (jint)old_state, (jint)new_state);
}

void native_on_playing_progress(void *owner, gint64 progress, gint64 duration)
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_playing_progress_id, (jlong)progress, (jlong)duration);
}

void native_on_volume_changed(void *owner, RctGstAudioLevel* audioLevel, int nb_channels)
{
    JNIEnv *env = get_jni_env();
    jobjectArray JavaRctGstAudioLevelsArray;
    int current_channel = 0;

    jmethodID constructor = (*env)->GetMethodID(env, JavaRctGstAudioLevel, "<init>", "(DDD)V");
    JavaRctGstAudioLevelsArray = (*env)->NewObjectArray(env, nb_channels, JavaRctGstAudioLevel, NULL);

    // Create volume data array
    for (current_channel = 0; current_channel < nb_channels; current_channel++) {
        jobject audioLevelObject = (*env)->NewObject(env, JavaRctGstAudioLevel, constructor, audioLevel[current_channel].rms, audioLevel[current_channel].peak, audioLevel[current_channel].decay);
        (*env)->SetObjectArrayElement(env, JavaRctGstAudioLevelsArray, current_channel, audioLevelObject);
        (*env)->DeleteLocalRef(env, audioLevelObject);
    }

    (*env)->CallVoidMethod(env, app, on_volume_changed_id , JavaRctGstAudioLevelsArray, (jint)nb_channels);
    (*env)->DeleteLocalRef(env, JavaRctGstAudioLevelsArray);
}

void native_on_uri_changed(void *owner, gchar *_new_uri)
{
    __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "native_on_uri_changed : %s", _new_uri);

    JNIEnv *env = get_jni_env();
    jstring new_uri = (*env)->NewStringUTF(env, _new_uri);

    (*env)->CallVoidMethod(env, app, on_uri_changed_id, new_uri);
}

void native_on_eos(void *owner)
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_eos_id);
}

void native_on_element_error(void *owner, gchar *_source, gchar *_message, gchar *_debug_info)
{
    __android_log_print(ANDROID_LOG_ERROR, "RCTGstPlayer", "native_on_element_error from %s with message : %s (%s)", _source, _message, _debug_info);

    JNIEnv *env = get_jni_env();

    jstring source = (*env)->NewStringUTF(env, _source);
    jstring message = (*env)->NewStringUTF(env, _message);
    jstring debug_info = (*env)->NewStringUTF(env, _debug_info);
    (*env)->CallVoidMethod(env, app, on_element_error_id, source, message, debug_info);
}

// Bindings methods
static void native_rct_gst_init(JNIEnv* env, jobject thiz)
{
    jclass klass = (*env)->FindClass(env, "com/kalyzee/rctgstplayer/RCTGstPlayerController");

    rct_gst_init(get_user_data());

    // onPlayerInit
    on_player_init_id = (*env)->GetMethodID(env, klass, "onPlayerInit", "()V");
    get_user_data()->configuration->onPlayerInit = native_on_player_init;

    // onStateChanged
    on_state_changed_id = (*env)->GetMethodID(env, klass, "onStateChanged", "(II)V");
    get_user_data()->configuration->onStateChanged = native_on_state_changed;

    // onPlayingProgress
    on_playing_progress_id = (*env)->GetMethodID(env, klass, "onPlayingProgress", "(JJ)V");
    get_user_data()->configuration->onPlayingProgress = native_on_playing_progress;

    // onElementError
    on_element_error_id = (*env)->GetMethodID(env, klass, "onElementError", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    get_user_data()->configuration->onElementError = native_on_element_error;

    // onUriChanged
    on_uri_changed_id = (*env)->GetMethodID(env, klass, "onUriChanged", "(Ljava/lang/String;)V");
    get_user_data()->configuration->onUriChanged = native_on_uri_changed;

    // onVolumeChanged
    on_volume_changed_id = (*env)->GetMethodID(env, klass, "onVolumeChanged", "([Lcom/kalyzee/rctgstplayer/utils/RCTGstAudioLevel;I)V");
    get_user_data()->configuration->onVolumeChanged = native_on_volume_changed;

    // Creating native code internal data gst_app_thread
    app = (*env)->NewGlobalRef(env, thiz);

    __android_log_print(ANDROID_LOG_DEBUG, "RCTGstPlayer", "Running Thread ...");
    if (pthread_create(&gst_app_thread, NULL, (void *)&rct_gst_run_loop, get_user_data()) == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "RCTGstPlayer", "Thread started...");
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "RCTGstPlayer", "Thread Failed to start...");
    }
}

static void native_rct_gst_terminate(JNIEnv *env, jobject thiz)
{
    (void)env;
    (void)thiz;

    rct_gst_terminate(get_user_data());
}

static jstring native_rct_gst_get_gstreamer_info(JNIEnv* env, jobject thiz)
{
    (void)env;
    (void)thiz;

    char *version_utf8 = rct_gst_get_info();
    jstring *version_jstring = (*env)->NewStringUTF(env, version_utf8);
    g_free (version_utf8);
    return version_jstring;
}

static void native_rct_gst_set_drawable_surface(JNIEnv* env, jobject thiz, jobject surface)
{
    (void)env;
    (void)thiz;

    native_window = ANativeWindow_fromSurface(env, surface);
    GstState currentState;
    gst_element_get_state(get_user_data()->playbin, &currentState, NULL, NULL);

    __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "Received surface %p (native window %p)", surface, native_window);

    rct_gst_set_drawable_surface(get_user_data(), (guintptr)native_window);
}

static void native_rct_gst_set_pipeline_state(JNIEnv* env, jobject thiz, jint state)
{
    (void)env;
    (void)thiz;

    __android_log_print(ANDROID_LOG_INFO, "RCTGstPlayer", "new state requested : %i", state);

    rct_gst_set_playbin_state(get_user_data(), (GstState) state);
}

static void native_rct_gst_set_uri(JNIEnv* env, jobject thiz, jstring uri_j)
{
    (void)env;
    (void)thiz;

    gchar *uri = (gchar *)(*env)->GetStringUTFChars(env, uri_j, 0);
    rct_gst_set_uri(get_user_data(), uri);
}

static void native_rct_gst_set_volume(JNIEnv *env, jobject thiz, gdouble volume)
{
    (void)env;
    (void)thiz;

    rct_gst_set_volume(get_user_data(), (gdouble)volume);
}

static void native_rct_gst_set_ui_refresh_rate(JNIEnv* env, jobject thiz, jint audio_level_refresh_rate)
{
    (void)env;
    (void)thiz;

    rct_gst_set_ui_refresh_rate(get_user_data(), audio_level_refresh_rate);
}

static void native_rct_gst_seek(JNIEnv* env, jobject thiz, jint progress) {
    (void)env;
    (void)thiz;

    rct_gst_seek(get_user_data(), progress);
}

static void native_rct_on_player_init(JNIEnv* env, jobject thiz) {
    rct_gst_set_uri(get_user_data(), get_user_data()->configuration->uri);
    rct_gst_set_ui_refresh_rate(get_user_data(), get_user_data()->configuration->uiRefreshRate);
    rct_gst_set_volume(get_user_data(), get_user_data()->configuration->volume);
}

static JNINativeMethod native_methods[] = {
    { "nativeRCTGstInit", "()V", (void *) native_rct_gst_init },
    { "nativeRCTGstTerminate", "()V", (void *) native_rct_gst_terminate },
    { "nativeRCTGstGetGStreamerInfo", "()Ljava/lang/String;", (void *) native_rct_gst_get_gstreamer_info },
    { "nativeRCTGstSetPipelineState", "(I)V", (void *) native_rct_gst_set_pipeline_state },
    { "nativeRCTGstSetDrawableSurface", "(Landroid/view/Surface;)V", (void *) native_rct_gst_set_drawable_surface },
    { "nativeRCTGstSetUri", "(Ljava/lang/String;)V", (void *) native_rct_gst_set_uri },
    { "nativeRCTGstSetVolume", "(D)V", (void *) native_rct_gst_set_volume },
    { "nativeRCTGstSetUiRefreshRate", "(I)V", (void *) native_rct_gst_set_ui_refresh_rate },
    { "nativeRCTGstSeek", "(J)V", (void *) native_rct_gst_seek },
    { "nativeRCTOnPlayerInit", "()V", (void *) native_rct_on_player_init }
};

// Called by JNI
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    // Storing global context
    jvm = vm;
    JNIEnv *env = NULL;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "RCTGstPlayer", "Could not retrieve JNIEnv");
        return 0;
    }
    jclass klass = (*env)->FindClass(env, "com/kalyzee/rctgstplayer/RCTGstPlayerController");
    (*env)->RegisterNatives(env, klass, native_methods, G_N_ELEMENTS(native_methods));

    // Audio structure
    // JavaRctGstAudioLevel = (*env)->FindClass(env, "com/kalyzee/rctgstplayer/utils/RCTGstAudioLevel");
    JavaRctGstAudioLevel = (*env)->NewGlobalRef(
            env,
            (*env)->FindClass(env, "com/kalyzee/rctgstplayer/utils/RCTGstAudioLevel")
    );

    pthread_key_create(&current_jni_env, detach_current_thread);

    return JNI_VERSION_1_6;
}
