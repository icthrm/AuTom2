#include "procinvoker.h"
#include "mrcimageprovider.h"
#include "svgimageprovider.h"
#include "mrc2img.h"
#include <QSvgRenderer>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariantMap>
#include "reproc++/reproc.hpp"
#include <vector>

ProcInvoker::ProcInvoker(QObject *parent) :
    QObject(parent),
    m_imageProvider(nullptr),
    m_svgImageProvider(nullptr)
{
    qDebug() << "!!!!!!!! C++ REAL BACKEND IS ALIVE !!!!!!!!!!";
    m_workingDir = std::filesystem::absolute(std::filesystem::current_path()); // 默认工作目录为当前路径

    // 设置日志文件路径
    m_logFilePath = QString::fromStdString(m_workingDir.string()) + "/command_history.log";
    qDebug() << "Command log file:" << m_logFilePath;
}

void ProcInvoker::terminateCommand(const QString &program)
{
    qDebug() << "C++ ProcInvoker: Request to terminate:" << program;
    
    std::shared_ptr<reproc::process> procToStop;

    // 1. 在互斥锁保护下查找进程
    {
        QMutexLocker locker(&m_processMutex);
        if (m_activeProcesses.contains(program)) {
            procToStop = m_activeProcesses.value(program);
        }
    }

    // 2. 执行终止
    if (procToStop) {
        // 根据 reproc++ 官方文档，正确的结构体是 stop_actions
        reproc::stop_actions stop_opts;
        
        // 第一阶段：尝试正常终止 (SIGTERM)
        stop_opts.first.action = reproc::stop::terminate;
        stop_opts.first.timeout = std::chrono::milliseconds(2000); // 等待2秒
        
        // 第二阶段：如果没反应，强制杀死 (SIGKILL)
        stop_opts.second.action = reproc::stop::kill;
        stop_opts.second.timeout = std::chrono::milliseconds(500);

        // 执行停止动作
        procToStop->stop(stop_opts);
        
        qDebug() << "C++ ProcInvoker: Termination sequence initiated for" << program;
    } else {
        qDebug() << "C++ ProcInvoker: No active process found to terminate:" << program;
    }
}
// 返回类型现在是 void
void ProcInvoker::runCommand(const QString &program, const QStringList &args)
{
    qDebug() << "C++ ProcInvoker: Kicking off command via QtConcurrent:" << program << args;

    // 将参数转换为 reproc 需要的格式
    std::vector<std::string> command_line;
    std::string program_str = program.toStdString();
    std::filesystem::path program_path = m_workingDir / program_str.c_str();
    #ifdef _WIN32
        other_program_path.replace_extension(".exe");
    #endif
    command_line.push_back(program_path.string());
    qDebug() << "C++ ProcInvoker: Program path is:" << QString::fromStdString(program_path.string());
    for(const QString& arg : args) {
        command_line.push_back(arg.toStdString());
    }

    runCommandDirect(command_line, program);
}

void ProcInvoker::runCommandGlobal(const QString &program, const QStringList &args)
{
    qDebug() << "C++ ProcInvoker: Kicking off command via QtConcurrent:" << program << args;

    // 将参数转换为 reproc 需要的格式
    std::vector<std::string> command_line;
    std::string program_str = program.toStdString();
    #ifdef _WIN32
        other_program_path.replace_extension(".exe");
    #endif
    command_line.push_back(program_str);
    // qDebug() << "C++ ProcInvoker: Program path is:" << QString::fromStdString(program_path.string());
    for(const QString& arg : args) {
        command_line.push_back(arg.toStdString());
    }

    runCommandDirect(command_line, program);
}

Q_INVOKABLE void ProcInvoker::runCommandOnDir(const QString& work_dir, const QString& program, const QStringList& args) 
{
    qDebug() << "C++ ProcInvoker: Kicking off command via QtConcurrent:" << program << args;

    // 将参数转换为 reproc 需要的格式
    std::vector<std::string> command_line;
    std::string program_str = program.toStdString();
    std::filesystem::path program_path = m_workingDir / program_str.c_str();
    #ifdef _WIN32
        other_program_path.replace_extension(".exe");
    #endif
    command_line.push_back(program_path.string());
    qDebug() << "C++ ProcInvoker: Program path is:" << QString::fromStdString(program_path.string());
    for(const QString& arg : args) {
        command_line.push_back(arg.toStdString());
    }

    runCommandDirect(command_line, program, work_dir);

}

Q_INVOKABLE void ProcInvoker::runCommandGlobalOnDir(const QString& work_dir, const QString& program, const QStringList& args) 
{
    qDebug() << "C++ ProcInvoker: Kicking off command via QtConcurrent:" << program << args;

    // 将参数转换为 reproc 需要的格式
    std::vector<std::string> command_line;
    std::string program_str = program.toStdString();
    std::filesystem::path program_path = program_str.c_str();
    #ifdef _WIN32
        other_program_path.replace_extension(".exe");
    #endif
    command_line.push_back(program_path.string());
    qDebug() << "C++ ProcInvoker: Program path is:" << QString::fromStdString(program_path.string());
    for(const QString& arg : args) {
        program_str = arg.toStdString(); // 可能是 "-n", "some.file", "." 等

        std::filesystem::path program_path2;

        if (program_str == ".") {
            // 情况 1: 刚好是单个 "."
            program_path2 = m_workingDir;
        } 
        else if (program_str.rfind("./", 0) == 0) {
            // 情况 2: 以 "./" 开头 (Linux/Unix 风格)
            program_path2 = m_workingDir / program_str.substr(2);
        } 
        #ifdef _WIN32
        else if (program_str.rfind(".\\", 0) == 0) {
            // 情况 3: 以 ".\" 开头 (Windows 风格)
            program_path2 = m_workingDir / program_str.substr(2);
        }
        #endif
        else {
            // 情况 4: 其他参数（如 -n, -a）或普通文本，保持原样
            program_path2 = program_str;
        }
        command_line.push_back(program_path2.string());
    }

    runCommandDirect(command_line, program, work_dir);

}

void ProcInvoker::runCommandDirect(std::vector<std::string> command_line, const QString& program, QString workingdir) {
    // 如果未指定工作目录，使用默认值
    if(workingdir.isEmpty()) {
        workingdir = QString::fromStdString(m_workingDir.string());
    }

    // 记录命令到日志
    logCommand(command_line, workingdir);

    // *** 核心修改：使用 QtConcurrent::run 将任务放到后台线程 ***
    // 这个 runCommand 函数会立即返回，UI 不会阻塞。
    // lambda 表达式中的代码将在 Qt 的全局线程池中执行。
    auto process = std::make_shared<reproc::process>();
    {
        QMutexLocker locker(&m_processMutex);
        // 如果同名程序已经在运行，可以选择先终止它或直接返回
        m_activeProcesses.insert(program, process);
    }

    qDebug() << "workingdir is " << workingdir;
    (void)QtConcurrent::run([this, command_line, program, process, workingdir]() {
        // --- 这部分代码在后台线程运行 ---

        
        reproc::options options;

        std::string stdWorkDir = workingdir.toStdString();
        
        // 2. 检查目录是否存在（防御性编程）
        if (!std::filesystem::exists(stdWorkDir)) {
            emit dataReceived("Error: Working directory does not exist: " + workingdir, program);
            return;
        }

        // 3. 赋值指针（此时 stdWorkDir 的生命周期覆盖了 start 函数）
        options.working_directory = stdWorkDir.c_str(); 
        

        // options.redirect.merge = true; 

        std::error_code ec = process->start(command_line, options);
        if (ec) {
            emit dataReceived("Error: Failed to start process: " + QString::fromStdString(ec.message()), program);
            emit commandFinished(ec.value(), program);
            return; // 结束后台任务
        }

        char buffer[4096];
        while(true) {
            auto [bytes_read, ec_read] = process->read(reproc::stream::out, reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));

            if (bytes_read > 0) {
                // 从后台线程发射信号。Qt 会保证这个信号被安全地投递到主线程。
                emit dataReceived(QString::fromUtf8(buffer, bytes_read), program);
            }

            if (ec_read == std::errc::broken_pipe) {
                break;
            } else if (ec_read) {
                emit dataReceived("\nError: Failed to read output: " + QString::fromStdString(ec_read.message()), program);
                break;
            }
            
        }

        auto [exit_status, stop_ec] = process->stop(options.stop);
        {
            QMutexLocker locker(&m_processMutex);
            m_activeProcesses.remove(program);
        }
        if (stop_ec) {
            emit dataReceived("\nError: Failed to stop process: " + QString::fromStdString(stop_ec.message()), program);
        }

        // 任务结束，发射最终的 commandFinished 信号
        emit commandFinished(exit_status, program);
        qDebug() << "Background task finished with exit code:" << exit_status;
    });
}

QString ProcInvoker::loadMrcSlice(const QString& filePath, int sliceIndex)
{
    if (!m_imageProvider) {
        qWarning() << "MrcImageProvider not set!";
        return QString();
    }

    return m_imageProvider->loadMrcSlice(filePath, sliceIndex);
}

int ProcInvoker::getMrcSliceCount(const QString& filePath)
{
    qDebug() << "C++ ProcInvoker: Getting slice count from" << filePath;

    // 创建 MrcStack 对象
    util::MrcStack mrcStack;

    // 打开 MRC 文件
    if (!mrcStack.Open(filePath.toStdString().c_str())) {
        qWarning() << "Failed to open MRC file:" << filePath;
        return -1;
    }

    // 获取切片数量
    int sliceCount = mrcStack.Size();

    // 关闭 MRC 文件
    mrcStack.Close();

    qDebug() << "MRC file contains" << sliceCount << "slices";
    return sliceCount;
}

QString ProcInvoker::getMrcHeader(const QString& filePath)
{
    qDebug() << "C++ ProcInvoker: Getting MRC header from" << filePath;

    // 创建 MrcStack 对象
    util::MrcStack mrcStack;

    // 打开 MRC 文件
    if (!mrcStack.Open(filePath.toStdString().c_str())) {
        qWarning() << "Failed to open MRC file:" << filePath;
        QJsonObject errorObj;
        errorObj["error"] = "Failed to open MRC file";
        return QString(QJsonDocument(errorObj).toJson(QJsonDocument::Compact));
    }

    // 读取 header 详细信息
    MrcHeader header;
    if (!MrcReadHead(&header, filePath.toStdString().c_str())) {
        mrcStack.Close();
        QJsonObject errorObj;
        errorObj["error"] = "Failed to read MRC header";
        return QString(QJsonDocument(errorObj).toJson(QJsonDocument::Compact));
    }

    // 构建 JSON 对象
    QJsonObject headerJson;

    // 基本维度信息
    QJsonObject dimensions;
    dimensions["nx"] = header.nx;
    dimensions["ny"] = header.ny;
    dimensions["nz"] = header.nz;
    headerJson["dimensions"] = dimensions;

    // Map mode
    QJsonObject mapMode;
    mapMode["mapc"] = header.mapc;
    mapMode["mapr"] = header.mapr;
    mapMode["maps"] = header.maps;
    headerJson["mapMode"] = mapMode;

    // Cell dimensions (单位: 埃)
    QJsonObject cellDimensions;
    cellDimensions["xlen"] = header.xlen;
    cellDimensions["ylen"] = header.ylen;
    cellDimensions["zlen"] = header.zlen;
    headerJson["cellDimensions"] = cellDimensions;

    // Pixel size (计算实际像素大小, 单位: 埃)
    float pixelX = (header.nx > 0) ? header.xlen / header.nx : 0.0f;
    float pixelY = (header.ny > 0) ? header.ylen / header.ny : 0.0f;
    float pixelZ = (header.nz > 0) ? header.zlen / header.nz : 0.0f;

    QJsonObject pixelSize;
    pixelSize["x"] = pixelX;
    pixelSize["y"] = pixelY;
    pixelSize["z"] = pixelZ;
    headerJson["pixelSize"] = pixelSize;

    // Data mode
    QString modeStr;
    switch(header.mode) {
        case MRC_MODE_BYTE: modeStr = "byte (unsigned 8-bit)"; break;
        case MRC_MODE_SHORT: modeStr = "short (signed 16-bit)"; break;
        case MRC_MODE_FLOAT: modeStr = "float (32-bit)"; break;
        case MRC_MODE_COMPLEX_SHORT: modeStr = "complex short"; break;
        case MRC_MODE_COMPLEX_FLOAT: modeStr = "complex float"; break;
        case MRC_MODE_USHORT: modeStr = "unsigned short (16-bit)"; break;
        case MRC_MODE_RGB: modeStr = "RGB (3 x 8-bit)"; break;
        default: modeStr = QString("unknown (%1)").arg(header.mode); break;
    }

    QJsonObject dataMode;
    dataMode["code"] = header.mode;
    dataMode["description"] = modeStr;
    headerJson["dataMode"] = dataMode;

    // 密度统计信息
    QJsonObject density;
    density["min"] = header.amin;
    density["max"] = header.amax;
    density["mean"] = header.amean;
    headerJson["density"] = density;

    // Space group
    headerJson["spaceGroup"] = header.ispg;

    // Origin
    QJsonObject origin;
    origin["x"] = header.xorg;
    origin["y"] = header.yorg;
    origin["z"] = header.zorg;
    headerJson["origin"] = origin;

    // 可选：添加其他有用字段
    headerJson["nsymbt"] = header.nsymbt;
    headerJson["next"] = header.next;

    // 关闭文件
    mrcStack.Close();

    qDebug() << "MRC header info retrieved successfully";

    // 转换为 JSON 字符串
    QJsonDocument doc(headerJson);
    return QString(doc.toJson(QJsonDocument::Compact));
}

void ProcInvoker::logCommand(const std::vector<std::string>& command_line, const QString& workingdir)
{
    QFile logFile(m_logFilePath);

    // 以追加模式打开文件
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open command log file:" << m_logFilePath;
        return;
    }

    QTextStream out(&logFile);

    // 写入时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    out << "[" << timestamp << "] ";

    // 写入工作目录
    out << "WorkDir: " << workingdir << " | ";

    // 写入完整命令
    out << "Command: ";
    for (size_t i = 0; i < command_line.size(); ++i) {
        if (i > 0) out << " ";
        QString arg = QString::fromStdString(command_line[i]);
        // 如果参数包含空格，加引号
        if (arg.contains(' ')) {
            out << "\"" << arg << "\"";
        } else {
            out << arg;
        }
    }
    out << "\n";

    logFile.close();

    qDebug() << "Command logged to:" << m_logFilePath;
}

// ==================== File Scan API Implementation ====================

QVariantList ProcInvoker::scanMrcRawtltPairs(const QString& folderPath)
{
    qDebug() << "C++ ProcInvoker: Scanning folder for .st/.rawtlt pairs:" << folderPath;

    QVariantList result;

    QDir dir(folderPath);
    if (!dir.exists()) {
        qWarning() << "Folder does not exist:" << folderPath;
        return result;
    }

    // 同时收集 .st 和 .mrc 文件
    QStringList mrcFiles = dir.entryList(QStringList() << "*.st" << "*.mrc", QDir::Files);

    for (const QString& mrcFileName : mrcFiles) {
        QFileInfo mrcInfo(dir.filePath(mrcFileName));
        QString basename = mrcInfo.completeBaseName();

        // 查找同名的 .rawtlt 文件
        QString rawtltFileName = basename + ".rawtlt";
        QString rawtltPath = dir.filePath(rawtltFileName);

        if (QFile::exists(rawtltPath)) {
            QVariantMap pair;
            pair["mrcFile"]    = mrcInfo.filePath();
            pair["rawtltFile"] = rawtltPath;
            pair["basename"]   = basename;
            result.append(pair);
            qDebug() << "  Matched pair:" << basename;
        } else {
            qDebug() << "  No .rawtlt found for:" << basename;
        }
    }

    qDebug() << "C++ ProcInvoker: Found" << result.size() << "pairs in" << folderPath;
    return result;
}

bool ProcInvoker::fileExists(const QString& filePath)
{
    return QFile::exists(filePath);
}

// ==================== SVG API Implementation ====================

QString ProcInvoker::loadSvg(const QString& filePath, int width, int height)
{
    if (!m_svgImageProvider) {
        qWarning() << "SvgImageProvider not set!";
        return QString();
    }

    return m_svgImageProvider->loadSvg(filePath, width, height);
}

QString ProcInvoker::getSvgInfo(const QString& filePath)
{
    qDebug() << "C++ ProcInvoker: Getting SVG info from" << filePath;

    // Check if file exists
    if (!QFile::exists(filePath)) {
        qWarning() << "SVG file does not exist:" << filePath;
        QJsonObject errorObj;
        errorObj["error"] = "SVG file does not exist";
        return QString(QJsonDocument(errorObj).toJson(QJsonDocument::Compact));
    }

    // Create SVG renderer
    QSvgRenderer renderer(filePath);
    if (!renderer.isValid()) {
        qWarning() << "Failed to load SVG file:" << filePath;
        QJsonObject errorObj;
        errorObj["error"] = "Failed to load SVG file";
        return QString(QJsonDocument(errorObj).toJson(QJsonDocument::Compact));
    }

    // Build JSON object with SVG info
    QJsonObject svgInfo;

    // Get default size
    QSize defaultSize = renderer.defaultSize();
    QJsonObject sizeObj;
    sizeObj["width"] = defaultSize.width();
    sizeObj["height"] = defaultSize.height();
    svgInfo["defaultSize"] = sizeObj;

    // Get view box
    QRectF viewBox = renderer.viewBoxF();
    QJsonObject viewBoxObj;
    viewBoxObj["x"] = viewBox.x();
    viewBoxObj["y"] = viewBox.y();
    viewBoxObj["width"] = viewBox.width();
    viewBoxObj["height"] = viewBox.height();
    svgInfo["viewBox"] = viewBoxObj;

    // Calculate aspect ratio
    if (defaultSize.height() > 0) {
        double aspectRatio = static_cast<double>(defaultSize.width()) / defaultSize.height();
        svgInfo["aspectRatio"] = aspectRatio;
    }

    // File info
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        svgInfo["fileSize"] = file.size();
        file.close();
    }

    svgInfo["filePath"] = filePath;
    svgInfo["isValid"] = renderer.isValid();

    qDebug() << "SVG info retrieved successfully";

    // Convert to JSON string
    QJsonDocument doc(svgInfo);
    return QString(doc.toJson(QJsonDocument::Compact));
}

#include <QFileDialog>

QString ProcInvoker::selectFolder(const QString& title, const QString& defaultPath)
{
    QString dir = QFileDialog::getExistingDirectory(nullptr, title, defaultPath);
    qDebug() << "selectFolder returned:" << dir;
    return dir;
}

QString ProcInvoker::selectFile(const QString& title, const QString& defaultPath, const QString& filter)
{
    QString file = QFileDialog::getOpenFileName(nullptr, title, defaultPath, filter);
    qDebug() << "selectFile returned:" << file;
    return file;
}

