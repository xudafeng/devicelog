#include <stdio.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include "MobileDevice.h"

typedef struct {
  service_conn_t connection;
  CFSocketRef socket;
  CFRunLoopSourceRef source;
} DeviceConnection;

static CFMutableDictionaryRef connections;
static CFStringRef deviceId;

static void writeBuffer(int fd, const char *buffer, size_t length) {
  while (length) {
    ssize_t result = write(fd, buffer, length);
    if (!result) {
      break;
    }
    
    buffer += result;
    length -= result;
  }
}

static void socketCallback(CFSocketRef socket, CFSocketCallBackType type, CFDataRef address, const void *data, void *info) {
  ssize_t length = CFDataGetLength(data);
  const char *buffer = (const char *)CFDataGetBytePtr(data);
  
  while (length) {
    while (*buffer == '\0') {
      buffer++;
      length--;
      if (!length) {
        return;
      }
    }
    size_t extentLength = 0;
    
    while ((buffer[extentLength] != '\0') && extentLength != length) {
      extentLength++;
    }
    
    if (extentLength > 3) {
      writeBuffer(1, buffer, extentLength);
    }
    
    length -= extentLength;
    buffer += extentLength;
  }
}

static void deviceNotificationCallback(am_device_notification_callback_info *info, void *unknown) {
  struct am_device *device = info->dev;
  
  switch (info->msg) {
    case ADNCI_MSG_CONNECTED: {
      if (deviceId) {
        CFStringRef _deviceId = AMDeviceCopyDeviceIdentifier(device);
        Boolean isDevice = CFEqual(_deviceId, deviceId);
        CFRelease(_deviceId);
        if (!isDevice) {
          break;
        }
      }
      if (AMDeviceConnect(device) == MDERR_OK) {
        if (AMDeviceIsPaired(device) && (AMDeviceValidatePairing(device) == MDERR_OK)) {
          if (AMDeviceStartSession(device) == MDERR_OK) {
            service_conn_t connection;
            if (AMDeviceStartService(device, AMSVC_SYSLOG_RELAY, &connection, NULL) == MDERR_OK) {
              CFSocketRef socket = CFSocketCreateWithNative(kCFAllocatorDefault, connection, kCFSocketDataCallBack, socketCallback, NULL);
              if (socket) {
                CFRunLoopSourceRef source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, socket, 0);
                if (source) {
                  CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopCommonModes);
                  AMDeviceRetain(device);
                  DeviceConnection *data = malloc(sizeof *data);
                  data->connection = connection;
                  data->socket = socket;
                  data->source = source;
                  CFDictionarySetValue(connections, device, data);
                  return;
                }
                CFRelease(source);
              }
            }
            AMDeviceStopSession(device);
          }
        }
      }
      AMDeviceDisconnect(device);
      break;
    }
    case ADNCI_MSG_DISCONNECTED: {
      DeviceConnection *data = (DeviceConnection *)CFDictionaryGetValue(connections, device);
      if (data) {
        CFDictionaryRemoveValue(connections, device);
        AMDeviceRelease(device);
        CFRunLoopRemoveSource(CFRunLoopGetMain(), data->source, kCFRunLoopCommonModes);
        CFRelease(data->source);
        CFRelease(data->socket);
        free(data);
        AMDeviceStopSession(device);
        AMDeviceDisconnect(device);
      }
      break;
    }
    default:
      break;
  }
}

int main (int argc, char * const argv[]) {
  if (argv[1]) {
    printf("udid: %s\n", argv[1]);
    if (deviceId) {
      CFRelease(deviceId);
    } else {
      deviceId = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingASCII);
    }
  } else {
    printf("missing device udid\n");
  }
  
  connections = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
  am_device_notification *notification;
  AMDeviceNotificationSubscribe(deviceNotificationCallback, 0, 0, NULL, &notification);
  CFRunLoopRun();
  return 0;
}
