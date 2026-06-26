#include <QApplication>
#include <QQmlApplicationEngine>
#include <QLocale>
#include <QTranslator>
#include <QUrl>
#include <QString>
#include <QQmlContext> // 必须包含这个头文件

#include "environment.h"

#include "procinvoker.h"
#include "mrcimageprovider.h"
#include "svgimageprovider.h"

int main(int argc, char *argv[])
{


    // QGuiApplication 是 Qt Quick 应用的基础，比 QApplication 更轻量
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc, argv);

    // QML 引擎，负责加载和执行 QML 代码
    QQmlApplicationEngine engine;
    const QUrl url(mainQmlFile); 

    // ----- [可选，但推荐] 添加国际化支持 -----
//    QTranslator translator;
//    // 获取系统区域设置
//    const QStringList uiLanguages = QLocale::system().uiLanguages();
//    for (const QString &locale : uiLanguages) {
//        const QString baseName = "MyFirstQmlApp_" + QLocale(locale).name();
//        // 尝试加载翻译文件
//        if (translator.load(":/i18n/" + baseName)) {
//            app.installTranslator(&translator);
//            break;
//        }
//    }
    // -----------------------------------------

    // --- 核心步骤在这里 ---

    // 1. 创建你的C++ Backend类的实例
    ProcInvoker realBackend;
    realBackend.SetWorkingDir(argv[0]);
    qDebug() << "C++ Backend working directory set to:" << realBackend.m_workingDir;

    // 2. 创建并注册 MRC ImageProvider
    MrcImageProvider *mrcProvider = new MrcImageProvider();
    realBackend.setImageProvider(mrcProvider);

    // 2.1 创建并注册 SVG ImageProvider
    SvgImageProvider *svgProvider = new SvgImageProvider();
    realBackend.setSvgImageProvider(svgProvider);

    // 3. 获取QML引擎的根上下文(root context)
    QQmlContext *rootContext = engine.rootContext();

    // 4. 将C++对象实例注册到上下文中
    //    第一个参数 "backend" 是在QML中访问这个对象时使用的名字
    //    第二个参数 &realBackend 是C++对象的内存地址
    rootContext->setContextProperty("inReal", true);
    rootContext->setContextProperty("realBackend", &realBackend);

    // 5. 注册 ImageProvider
    engine.addImageProvider("mrc", mrcProvider);
    engine.addImageProvider("svg", svgProvider);

    // --- 注册完成 ---
    



    // 连接一个信号，以确保 QML 文件加载成功
    // 如果 QML 文件有语法错误，这能帮助我们优雅地退出程序
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1); // 加载失败，退出
        }, Qt::QueuedConnection);

    // 引擎加载 QML 文件
    // engine.load(url);
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath("qrc:/qt/qml");
    engine.addImportPath(":/");
    engine.load(url);
    set_qt_environment(); // 设置 Qt 环境变量

    if(engine.rootObjects().isEmpty()) {
        qWarning() << "Failed to load QML file:" << url.toString();
        return -1; // 如果没有加载到根对象，返回错误码
    }

    // 启动 Qt 事件循环
    return app.exec();
}
