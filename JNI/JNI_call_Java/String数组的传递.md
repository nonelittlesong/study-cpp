## Java传递String数组给C++
```
// Java
public class MainActivity extends AppCompatActivity {

    static{
        System.loadLibrary("myndk");
    }

    private TextView textView;

    public native String getStr(String[] oa);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        textView = (TextView)findViewById(R.id.text);
        String[] oa={"你呀","我呀"};
        textView.setText(getStr(oa));
    }
}


// C++ (这里是支持C++11的)
#include <iostream>
#include <string>

JNIEXPORT jstring JNICALL
Java_com_myndk_MainActivity_getStr(JNIEnv *env, jobject instance,jobjectArray oa)
{
    jsize size = env->GetArrayLength(oa); // 获取数组的长度
    for(int i=0;i<size;i++) // 循环获得每个String
    {
        jstring obj = (jstring)env->GetObjectArrayElement(oa,i); // 得到下标I的jobject强制转换为jstring
        std::string sstr = (std::string)env->GetStringUTFChars(obj,NULL);// 将jstring强制转换为std::string得到字符串

    }
    std::string str="NDK";
    return env->NewStringUTF(str.data());
}
```
