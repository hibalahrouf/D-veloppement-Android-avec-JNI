#include <jni.h>
#include <string>
#include <algorithm>
#include <climits>
#include <chrono>
#include <android/log.h>

#define LOG_TAG "JNI_NEXUS"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ──────────────────────────────────────────────
// Implémentations (noms courts, pas de préfixe Java_)
// ──────────────────────────────────────────────

static jstring impl_greetFromNative(JNIEnv* env, jobject) {
    LOGI("[greetFromNative] Invoque depuis la couche native");
    return env->NewStringUTF("Bonjour depuis C++ via JNI !");
}

static jint impl_computeFactorial(JNIEnv* env, jobject, jint inputVal) {
    auto startTime = std::chrono::steady_clock::now();
    if (inputVal < 0) {
        LOGE("[computeFactorial] Valeur negative : %d", inputVal);
        return -1;
    }
    long long result = 1;
    for (int step = 1; step <= inputVal; step++) {
        result *= step;
        if (result > INT_MAX) {
            LOGE("[computeFactorial] Depassement pour %d", inputVal);
            return -2;
        }
    }
    auto endTime = std::chrono::steady_clock::now();
    auto durationNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    LOGI("[computeFactorial] %d! = %lld en %lld ns", inputVal, result, durationNs);
    return static_cast<jint>(result);
}

static jstring impl_flipString(JNIEnv* env, jobject, jstring sourceString) {
    if (sourceString == nullptr) {
        LOGE("[flipString] Chaine nulle");
        return env->NewStringUTF("Erreur : chaine nulle");
    }
    const char* rawChars = env->GetStringUTFChars(sourceString, nullptr);
    if (rawChars == nullptr) {
        LOGE("[flipString] Echec decodage");
        return env->NewStringUTF("Erreur JNI");
    }
    std::string buffer(rawChars);
    env->ReleaseStringUTFChars(sourceString, rawChars);
    std::reverse(buffer.begin(), buffer.end());
    LOGI("[flipString] Resultat = %s", buffer.c_str());
    return env->NewStringUTF(buffer.c_str());
}

static jint impl_accumulateArray(JNIEnv* env, jobject, jintArray intData) {
    if (intData == nullptr) {
        LOGE("[accumulateArray] Tableau null");
        return -1;
    }
    jsize dataLen = env->GetArrayLength(intData);
    jint* dataPtr = env->GetIntArrayElements(intData, nullptr);
    if (dataPtr == nullptr) {
        LOGE("[accumulateArray] Echec acces elements");
        return -2;
    }
    long long accumulator = 0;
    for (jsize idx = 0; idx < dataLen; idx++) {
        accumulator += dataPtr[idx];
    }
    env->ReleaseIntArrayElements(intData, dataPtr, 0);
    if (accumulator > INT_MAX) {
        LOGE("[accumulateArray] Depassement");
        return -3;
    }
    LOGI("[accumulateArray] Total = %lld", accumulator);
    return static_cast<jint>(accumulator);
}

static jintArray impl_multiplyMatrices(JNIEnv* env, jobject, jintArray matA, jintArray matB, jint size) {
    jint* ptrA = env->GetIntArrayElements(matA, nullptr);
    jint* ptrB = env->GetIntArrayElements(matB, nullptr);
    jintArray matResult = env->NewIntArray(size * size);
    jint* ptrResult = env->GetIntArrayElements(matResult, nullptr);
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            long long cellValue = 0;
            for (int k = 0; k < size; k++) {
                cellValue += ptrA[row * size + k] * ptrB[k * size + col];
            }
            ptrResult[row * size + col] = static_cast<jint>(cellValue);
        }
    }
    LOGI("[multiplyMatrices] Multiplication %dx%d terminee", size, size);
    env->ReleaseIntArrayElements(matA, ptrA, 0);
    env->ReleaseIntArrayElements(matB, ptrB, 0);
    env->ReleaseIntArrayElements(matResult, ptrResult, 0);
    return matResult;
}

static jstring impl_detectForbiddenChars(JNIEnv* env, jobject, jstring inputString) {
    if (inputString == nullptr) {
        LOGE("[detectForbiddenChars] Chaine nulle recue");
        return env->NewStringUTF("ERREUR : chaine nulle");
    }
    const char* rawInput = env->GetStringUTFChars(inputString, nullptr);
    if (rawInput == nullptr) {
        LOGE("[detectForbiddenChars] Echec decodage");
        return env->NewStringUTF("ERREUR JNI");
    }
    const char forbiddenList[] = {'@', '#', '$', '%', '!'};
    const int forbiddenCount = 5;
    std::string inputBuf(rawInput);
    env->ReleaseStringUTFChars(inputString, rawInput);
    std::string foundChars = "";
    for (char c : inputBuf) {
        for (int idx = 0; idx < forbiddenCount; idx++) {
            if (c == forbiddenList[idx]) {
                foundChars += c;
                foundChars += " ";
            }
        }
    }
    if (foundChars.empty()) {
        LOGI("[detectForbiddenChars] Chaine propre : %s", inputBuf.c_str());
        return env->NewStringUTF("OK : aucun caractere interdit");
    } else {
        LOGE("[detectForbiddenChars] Caracteres interdits : %s", foundChars.c_str());
        std::string alertMsg = "INTERDIT : " + foundChars;
        return env->NewStringUTF(alertMsg.c_str());
    }
}

// ──────────────────────────────────────────────
// Table RegisterNatives
// Lie chaque nom Java → fonction C++ locale
// ──────────────────────────────────────────────
static JNINativeMethod gMethodTable[] = {
        {"greetFromNative",      "()Ljava/lang/String;",    (void*) impl_greetFromNative},
        {"computeFactorial",     "(I)I",                    (void*) impl_computeFactorial},
        {"flipString",           "(Ljava/lang/String;)Ljava/lang/String;", (void*) impl_flipString},
        {"accumulateArray",      "([I)I",                   (void*) impl_accumulateArray},
        {"multiplyMatrices",     "([I[II)[I",               (void*) impl_multiplyMatrices},
        {"detectForbiddenChars", "(Ljava/lang/String;)Ljava/lang/String;", (void*) impl_detectForbiddenChars},
};

// ──────────────────────────────────────────────
// JNI_OnLoad — appelé automatiquement au loadLibrary
// ──────────────────────────────────────────────
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE("[JNI_OnLoad] Impossible d'obtenir JNIEnv");
        return JNI_ERR;
    }

    jclass targetClass = env->FindClass("com/example/jnidemo/MainActivity");
    if (targetClass == nullptr) {
        LOGE("[JNI_OnLoad] Classe MainActivity introuvable");
        return JNI_ERR;
    }

    int methodCount = sizeof(gMethodTable) / sizeof(gMethodTable[0]);
    if (env->RegisterNatives(targetClass, gMethodTable, methodCount) != 0) {
        LOGE("[JNI_OnLoad] Echec RegisterNatives");
        return JNI_ERR;
    }

    LOGI("[JNI_OnLoad] %d methodes enregistrees avec succes", methodCount);
    return JNI_VERSION_1_6;
}