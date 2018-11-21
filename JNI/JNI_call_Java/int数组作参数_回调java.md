```
// c++
int32_t a[3] = {0,1,2}; // 创建int数组
jintArray array = env->NewIntArray(3); // 申明jintArray
env->SetIntArrayRegion(array, 0, 3, a); // 复制元素给jintArray

jclass jcl = env->GetObjectClass(ob);
jmethodID mid = env->GetMethodID(jcl, "cToJava", "([III)V"); // 回调java方法
env->CallVoidMethod(ob, mid,array,1,2); // 传递参数

// java
public void cToJava(int[] pixs,int width,int height) { // 定义java方法
    // ...
}
```
