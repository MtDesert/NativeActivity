LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := GamesAndroid
LOCAL_SRC_FILES := main.cpp\
../../libRenderer/EngineEGL.cpp\
../../libRenderer/EngineGLES.cpp\
$(wildcard ../libGamesEngines/*.cpp)\
$(wildcard ../libAdvanceWars/*.cpp)\
$(wildcard ../libAdvanceWars/GameScenes/*.cpp)\

LOCAL_C_INCLUDES :=\
../libGamesEngines\
../libAdvanceWars\
../libAdvanceWars/GameScenes\
../../libRenderer\
../../libCpp11\

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
