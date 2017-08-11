/*
 * =====================================================================================
 *
 *       Filename:  Hello.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/10/2017 02:04:30 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  first name surname (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <jni.h>
#include <iostream>
#include <pthread.h>
#include "Hello.h"

using namespace std;

void *socket_create(void*) {
	cout << "Connected" << endl;	
}

JNIEXPORT void JNICALL Java_Hello_sayHello (JNIEnv *e, jobject obj) {
    // cout << "Hello Java" << endl;

	pthread_t tid;
	
	pthread_create(&tid, NULL, socket_create, NULL);

	pthread_join(tid, NULL);

    return;
}