#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/asset_manager.h>
#include <android/bitmap.h>
#include <android/configuration.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android/obb.h>
#include <android/rect.h>
#include <android/sensor.h>
#include <android/storage_manager.h>
#include <android/tts.h>
#include <android/window.h>
#include <android_native_app_glue.h>

#include"Game.h"
#include"EngineEGL.h"
#include"SysTime.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "games.engines", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "games.engines", __VA_ARGS__))

EngineEGL engineEGL;//嵌入式图形库
SysTime sysTime;//linux时间模块
//传感器及其控制
#define SENSOR_AMOUNT 32
ASensorManager* sensorManager=NULL;
const ASensor* allSensors[SENSOR_AMOUNT]={NULL};
ASensorEventQueue* sensorEventQueue=NULL;

static int32_t engine_handle_input(struct android_app* app,AInputEvent* event){
	int32_t type=AInputEvent_getType(event);
	int32_t deviceID=AInputEvent_getDeviceId(event);
	int32_t source=AInputEvent_getSource(event);
	switch(type){
		case AINPUT_EVENT_TYPE_KEY:{
			int32_t action=AKeyEvent_getAction(event);
			int32_t flags=AKeyEvent_getFlags(event);
			int32_t keyCode=AKeyEvent_getKeyCode(event);
			int32_t scanCode=AKeyEvent_getScanCode(event);
		}break;
		case AINPUT_EVENT_TYPE_MOTION:{
			int32_t action=AMotionEvent_getAction(event);
			switch(action){
				case AMOTION_EVENT_ACTION_DOWN:LOGI("下");break;
				case AMOTION_EVENT_ACTION_UP:LOGI("上");break;
				case AMOTION_EVENT_ACTION_MOVE:LOGI("移动");break;
				case AMOTION_EVENT_ACTION_CANCEL:LOGI("取消");break;
				case AMOTION_EVENT_ACTION_OUTSIDE:LOGI("外部");break;
				case AMOTION_EVENT_ACTION_POINTER_DOWN:LOGI("指针下");break;
				case AMOTION_EVENT_ACTION_POINTER_UP:LOGI("指针上");break;
				case AMOTION_EVENT_ACTION_HOVER_MOVE:LOGI("盘旋移动");break;
				case AMOTION_EVENT_ACTION_SCROLL:LOGI("滚动");break;
				case AMOTION_EVENT_ACTION_HOVER_ENTER:LOGI("盘旋进入");break;
				case AMOTION_EVENT_ACTION_HOVER_EXIT:LOGI("盘旋退出");break;
			}
			size_t pointerCount=AMotionEvent_getPointerCount(event);
			size_t i=0;
			for(;i<pointerCount;++i){
				float x=AMotionEvent_getX(event,i);
				float y=AMotionEvent_getY(event,i);
				LOGI("x,y==%d,%d",(int)x,(int)y);
			}
		}break;
	}
	return 0;//返回0表示系统也进行处理
}

void printSensorInfo(const ASensor *sensor){//打印传感器信息
	if(!sensor)return;
	LOGI("传感器--------");
	LOGI("名称: %s",ASensor_getName(sensor));
	LOGI("供应商: %s",ASensor_getVendor(sensor));
	LOGI("类型: %d",ASensor_getType(sensor));
	LOGI("分辨率: %f",ASensor_getResolution(sensor));
	LOGI("最小延时: %d",ASensor_getMinDelay(sensor));
}
void startupSensor(const ASensor *sensor){//启动传感器
	if(!sensor)return;
	ASensorEventQueue_enableSensor(sensorEventQueue,sensor);
	ASensorEventQueue_setEventRate(sensorEventQueue,sensor,1000000/60);
}
void stopSensor(const ASensor *sensor){//关闭传感器
	if(sensor)ASensorEventQueue_disableSensor(sensorEventQueue,sensor);
}

static void engine_handle_cmd(struct android_app *app,int32_t cmd) {
	switch (cmd) {
		case APP_CMD_INPUT_CHANGED:break;
		case APP_CMD_INIT_WINDOW:// The window is being shown, get it ready.
			if (app->window) {
				engineEGL.initial(app->window);
			}
		break;
		case APP_CMD_TERM_WINDOW:// The window is being hidden or closed, clean it up.
			engineEGL.terminate();
		break;
		case APP_CMD_WINDOW_RESIZED:break;
		case APP_CMD_WINDOW_REDRAW_NEEDED:
			LOGI("Redraw");
			//game->render();
			//engineEGL.swapBuffer();
		break;
		case APP_CMD_CONTENT_RECT_CHANGED:break;
		case APP_CMD_GAINED_FOCUS:{
			//启动所有传感器
			int i=0;
			for(;i<SENSOR_AMOUNT;++i){
				startupSensor(allSensors[i]);
			}
		}break;
		case APP_CMD_LOST_FOCUS:{
			//停止所有传感器
			int i=0;
			for(;i<SENSOR_AMOUNT;++i){
				stopSensor(allSensors[i]);
			}
		}break;
		case APP_CMD_CONFIG_CHANGED:break;
		case APP_CMD_LOW_MEMORY:break;
		case APP_CMD_START:break;
		case APP_CMD_RESUME:break;
		case APP_CMD_SAVE_STATE:// The system has asked us to save our current state.  Do so.
			app->savedState = NULL;
			app->savedStateSize = 0;
		break;
		case APP_CMD_PAUSE:break;
		case APP_CMD_STOP:break;
		case APP_CMD_DESTROY:break;
		default:;
	}
}

extern"C" void android_main(struct android_app* app) {
	LOGI("主函数开始启动");
	app_dummy();//删除此函数可能导致启动失败
	Game *game=Game::newGame();
	app->onAppCmd = engine_handle_cmd;
	app->onInputEvent = engine_handle_input;
	app->userData=game;//以便给回调函数使用
	
	//显示所有传感器信息
	sensorManager = ASensorManager_getInstance();
	int i=0;
	for(;i<SENSOR_AMOUNT;++i){
		allSensors[i]=ASensorManager_getDefaultSensor(sensorManager,i);
		printSensorInfo(allSensors[i]);
	}
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager,app->looper,LOOPER_ID_USER,NULL,NULL);
	LOGI("事件循环启动");
	LOGI("编译时间%s %s",__DATE__,__TIME__);
	int ident=0;
	int events=0;
	sysTime.usecElapsed();
	int timeSlice=0,elapsed=0,redrawInterval=20000;
	struct android_poll_source* source=NULL;
	while (true) {
		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		ident=ALooper_pollAll(0,NULL,&events,(void**)&source);
		for(;ident >= 0;ident=ALooper_pollAll(1,NULL,&events,(void**)&source)){
			//处理事件
			if (source)source->process(app,source);
			//有传感器事件,可以处理
			if (ident == LOOPER_ID_USER) {
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(sensorEventQueue,&event,1) > 0) {
					switch(event.type){
						case ASENSOR_TYPE_ACCELEROMETER:
							//LOGI("accelerometer: x=%f y=%f z=%f",event.acceleration.x, event.acceleration.y,event.acceleration.z);
						break;
						default:;
					}
				}
			}
			
			//需要退出的时候调用此代码
			if (app->destroyRequested != 0) {
				engineEGL.terminate();
				ASensorManager_destroyEventQueue(sensorManager,sensorEventQueue);
				return;
			}
		}
		switch(ident){
			case ALOOPER_POLL_WAKE:LOGI("唤醒");break;
			case ALOOPER_POLL_CALLBACK:LOGI("回调");break;
			case ALOOPER_POLL_TIMEOUT:
				//增加时间片并判断是否重绘
				elapsed=sysTime.usecElapsed();
				game->addTimeSlice(elapsed);
				timeSlice += elapsed;
				if(timeSlice>=redrawInterval){
					timeSlice-=redrawInterval;
					game->render();
					engineEGL.swapBuffer();
				}
			break;
			case ALOOPER_POLL_ERROR:LOGI("错误");break;
			default:;
		}
	}
	delete game;
}