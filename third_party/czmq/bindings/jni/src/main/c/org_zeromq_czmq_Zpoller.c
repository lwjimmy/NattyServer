/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
*/
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include "czmq.h"
#include "org_zeromq_czmq_Zpoller.h"

JNIEXPORT jlong JNICALL
Java_org_zeromq_czmq_Zpoller__1_1new (JNIEnv *env, jclass c, jlong reader)
{
    //  Disable CZMQ signal handling; allow Java to deal with it
    zsys_handler_set (NULL);
    jlong new_ = (jlong) (intptr_t) zpoller_new ((void *) (intptr_t) reader);
    return new_;
}

JNIEXPORT void JNICALL
Java_org_zeromq_czmq_Zpoller__1_1destroy (JNIEnv *env, jclass c, jlong self)
{
    zpoller_destroy ((zpoller_t **) &self);
}

JNIEXPORT jint JNICALL
Java_org_zeromq_czmq_Zpoller__1_1add (JNIEnv *env, jclass c, jlong self, jlong reader)
{
    jint add_ = (jint) zpoller_add ((zpoller_t *) (intptr_t) self, (void *) (intptr_t) reader);
    return add_;
}

JNIEXPORT jint JNICALL
Java_org_zeromq_czmq_Zpoller__1_1remove (JNIEnv *env, jclass c, jlong self, jlong reader)
{
    jint remove_ = (jint) zpoller_remove ((zpoller_t *) (intptr_t) self, (void *) (intptr_t) reader);
    return remove_;
}

JNIEXPORT void JNICALL
Java_org_zeromq_czmq_Zpoller__1_1setNonstop (JNIEnv *env, jclass c, jlong self, jboolean nonstop)
{
    zpoller_set_nonstop ((zpoller_t *) (intptr_t) self, (bool) nonstop);
}

JNIEXPORT jlong JNICALL
Java_org_zeromq_czmq_Zpoller__1_1wait (JNIEnv *env, jclass c, jlong self, jint timeout)
{
    jlong wait_ = (jlong) zpoller_wait ((zpoller_t *) (intptr_t) self, (int) timeout);
    return wait_;
}

JNIEXPORT jboolean JNICALL
Java_org_zeromq_czmq_Zpoller__1_1expired (JNIEnv *env, jclass c, jlong self)
{
    jboolean expired_ = (jboolean) zpoller_expired ((zpoller_t *) (intptr_t) self);
    return expired_;
}

JNIEXPORT jboolean JNICALL
Java_org_zeromq_czmq_Zpoller__1_1terminated (JNIEnv *env, jclass c, jlong self)
{
    jboolean terminated_ = (jboolean) zpoller_terminated ((zpoller_t *) (intptr_t) self);
    return terminated_;
}

JNIEXPORT void JNICALL
Java_org_zeromq_czmq_Zpoller__1_1test (JNIEnv *env, jclass c, jboolean verbose)
{
    zpoller_test ((bool) verbose);
}

