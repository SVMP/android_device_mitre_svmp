LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := fbset.c

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := fbset

include $(BUILD_EXECUTABLE)
