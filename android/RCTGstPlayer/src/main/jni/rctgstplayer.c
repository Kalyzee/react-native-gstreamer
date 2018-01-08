#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <gst/gst.h>
#include <gstreamer_backend.h>
#include <pthread.h>

ANativeWindow *native_window;

// Java callbacks
static jmethodID on_player_init_id;
static jmethodID on_state_changed_id;
static jmethodID on_volume_changed_id;
static jmethodID on_uri_changed_id;
static jmethodID on_eos_id;
static jmethodID on_element_error_id;

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
static void detach_current_thread(void *env) {
    GST_DEBUG("Detaching thread %p", g_thread_self());
    (*jvm)->DetachCurrentThread(jvm);
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

// Bindings methods
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
    rct_gst_set_drawable_surface((guintptr)native_window);
}

static void native_rct_gst_set_pipeline_state(JNIEnv* env, jobject thiz, jint state)
{
    (void)env;
    (void)thiz;

    rct_gst_set_pipeline_state((GstState) state);
}

static void native_rct_gst_set_uri(JNIEnv* env, jobject thiz, jstring uri_j)
{
    (void)env;
    (void)thiz;

    gchar *uri = (gchar *)(*env)->GetStringUTFChars(env, uri_j, 0);
    rct_gst_set_uri(uri);
}

static void native_rct_gst_set_audio_level_refresh_rate(JNIEnv* env, jobject thiz, jint audio_level_refresh_rate)
{
    (void)env;
    (void)thiz;
    rct_gst_set_audio_level_refresh_rate(audio_level_refresh_rate);
}

static void native_rct_gst_set_debugging(JNIEnv* env, jobject thiz, jboolean is_debugging)
{
    (void)env;
    (void)thiz;
    rct_gst_set_debugging(is_debugging);
}


void native_on_init()
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_player_init_id);
}

void native_on_state_changed(GstState old_state, GstState new_state)
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_state_changed_id, (jint)old_state, (jint)new_state);
}

void native_on_volume_changed(RctGstAudioLevel* audioLevel)
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_volume_changed_id, audioLevel->rms, audioLevel->peak, audioLevel->decay);
}

void native_on_uri_changed(gchar *_new_uri)
{
    JNIEnv *env = get_jni_env();
    jstring new_uri = (*env)->NewStringUTF(env, _new_uri);
    (*env)->CallVoidMethod(env, app, on_uri_changed_id, new_uri);
}

void native_on_eos()
{
    JNIEnv *env = get_jni_env();
    (*env)->CallVoidMethod(env, app, on_eos_id);
}

void native_on_element_error(gchar *_source, gchar *_message, gchar *_debug_info)
{
    JNIEnv *env = get_jni_env();

    jstring source = (*env)->NewStringUTF(env, _source);
    jstring message = (*env)->NewStringUTF(env, _message);
    jstring debug_info = (*env)->NewStringUTF(env, _debug_info);
    (*env)->CallVoidMethod(env, app, on_element_error_id, source, message, debug_info);
}

static void native_rct_gst_init_and_run(JNIEnv* env, jobject thiz, jobject j_configuration)
{
    RctGstConfiguration* configuration = rct_gst_get_configuration();
    jclass configuration_class = (*env)->GetObjectClass(env, j_configuration);

    // Creating native code internal data gst_app_thread
    app = (*env)->NewGlobalRef(env, thiz);

    // Defining initial drawable surface (ids)
    jfieldID ids_field_id = (*env)->GetFieldID(env, configuration_class, "initialDrawableSurface", "Landroid/view/Surface;");
    jobject surface = (*env)->GetObjectField(env, j_configuration, ids_field_id);
    native_window = ANativeWindow_fromSurface(env, surface);
    configuration->initialDrawableSurface = (guintptr)native_window;

    // Getting all callbacks
    jclass klass = (*env)->FindClass(env, "com/kalyzee/rctgstplayer/RCTGstPlayerController");
    on_player_init_id = (*env)->GetMethodID(env, klass, "onInit", "()V");
    on_state_changed_id = (*env)->GetMethodID(env, klass, "onStateChanged", "(II)V");
    on_volume_changed_id = (*env)->GetMethodID(env, klass, "onVolumeChanged", "(DDD)V");
    on_uri_changed_id = (*env)->GetMethodID(env, klass, "onUriChanged", "(Ljava/lang/String;)V");
    on_eos_id = (*env)->GetMethodID(env, klass, "onEOS", "()V");
    on_element_error_id = (*env)->GetMethodID(env, klass, "onElementError", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");

    configuration->onInit = native_on_init;
    configuration->onStateChanged = native_on_state_changed;
    configuration->onVolumeChanged = native_on_volume_changed;
    configuration->onUriChanged = native_on_uri_changed;
    configuration->onEOS = native_on_eos;
    configuration->onElementError = native_on_element_error;

    rct_gst_init(configuration);

    pthread_create(&gst_app_thread, NULL, (void *)&rct_gst_run_loop, NULL);
}

static JNINativeMethod native_methods[] = {
        { "nativeRCTGstGetGStreamerInfo", "()Ljava/lang/String;", (void *) native_rct_gst_get_gstreamer_info },

        { "nativeRCTGstInitAndRun", "(Lcom/kalyzee/rctgstplayer/utils/RCTGstConfiguration;)V", (void *) native_rct_gst_init_and_run },

        { "nativeRCTGstSetPipelineState", "(I)V", (void *) native_rct_gst_set_pipeline_state },

        { "nativeRCTGstSetDrawableSurface", "(Landroid/view/Surface;)V", (void *) native_rct_gst_set_drawable_surface },
        { "nativeRCTGstSetUri", "(Ljava/lang/String;)V", (void *) native_rct_gst_set_uri },
        { "nativeRCTGstSetAudioLevelRefreshRate", "(I)V", (void *) native_rct_gst_set_audio_level_refresh_rate },
        { "nativeRCTGstSetDebugging", "(Z)V", (void *) native_rct_gst_set_debugging }
};

// Called by JNI
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    // Storing global context
    jvm = vm;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "RCTGstPlayer", "Could not retrieve JNIEnv");
        return 0;
    }
    jclass klass = (*env)->FindClass(env, "com/kalyzee/rctgstplayer/RCTGstPlayerController");
    (*env)->RegisterNatives(env, klass, native_methods, G_N_ELEMENTS(native_methods));

    pthread_key_create(&current_jni_env, detach_current_thread);

    return JNI_VERSION_1_6;
}