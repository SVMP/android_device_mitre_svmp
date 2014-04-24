LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := fbset.c

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := fbset

LOCAL_CFLAGS:= -DLOG_TAG=\"fbset\"

include $(BUILD_EXECUTABLE)
