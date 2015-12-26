1. Put directory "z_jpg" into path "/sdcard/" on mobile.
2. Copy library from ../../../Lib/Android/IOTCAPIs/libs/armeabi/libIOTCAPIs.so to ./libs/armeabi/
   Copy library from ../../../Lib/Android/AVAPIs/libs/armeabi/libAVAPIs.so to ./libs/armeabi/
   Copy java file from ../../../Lib/Android/IOTCAPIs/src/com/tutk/IOTC/*.java to ./src/com/tutk/IOTC/
   Copy java file from ../../../Lib/Android/AVAPIs/src/com/tutk/IOTC/*.java to ./src/com/tutk/IOTC/
3. Open project "Sample_AVAPIs_Server" and run it on mobile, the AV Server will send some mjpeg photos when client connects it.
4. There are more IOCTRL commands, definitions and structures in user document.