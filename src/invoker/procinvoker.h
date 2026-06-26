#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QStringList>
#include <QImage>
#include <QVariantList>
#include <QtConcurrent> // 引入 QtConcurrent
#include "reproc++/reproc.hpp"


class MrcImageProvider;
class SvgImageProvider;

class ProcInvoker : public QObject
{
    Q_OBJECT
public:
    explicit ProcInvoker(QObject *parent = nullptr);

    void setImageProvider(MrcImageProvider* provider) { m_imageProvider = provider; }
    void setSvgImageProvider(SvgImageProvider* provider) { m_svgImageProvider = provider; }

    void SetWorkingDir(char* argdir) {
        m_workingDir = m_workingDir / std::filesystem::path(argdir).parent_path(); // 设置工作目录为传入的路径
    }

signals:
    // 信号保持不变
    void dataReceived(const QString& chunk, const QString& program);
    void commandFinished(int exitCode, const QString& program);

public slots:
    // Q_INVOKABLE 依然需要，但返回类型是 void
    Q_INVOKABLE void runCommand(const QString& program, const QStringList& args);
    Q_INVOKABLE void runCommandGlobal(const QString& program, const QStringList& args);
    Q_INVOKABLE void runCommandOnDir(const QString& work_dir, const QString& program, const QStringList& args);
    Q_INVOKABLE void runCommandGlobalOnDir(const QString& work_dir, const QString& program, const QStringList& args);
    Q_INVOKABLE void terminateCommand(const QString &program);
    Q_INVOKABLE QString loadMrcSlice(const QString& filePath, int sliceIndex);
    Q_INVOKABLE int getMrcSliceCount(const QString& filePath);
    Q_INVOKABLE QString getMrcHeader(const QString& filePath);

    // 文件扫描 API
    Q_INVOKABLE QVariantList scanMrcRawtltPairs(const QString& folderPath);

    // 文件检查 API
    Q_INVOKABLE bool fileExists(const QString& filePath);

    // SVG API
    Q_INVOKABLE QString loadSvg(const QString& filePath, int width = 0, int height = 0);
    Q_INVOKABLE QString getSvgInfo(const QString& filePath);

    // 文件夹/文件选择对话框（Qt6.2 没有 FolderDialog/FileDialog，用 C++ QFileDialog 替代）
    Q_INVOKABLE QString selectFolder(const QString& title = QString(), const QString& defaultPath = QString());
    Q_INVOKABLE QString selectFile(const QString& title = QString(), const QString& defaultPath = QString(), const QString& filter = QString());



public: 

    std::filesystem::path m_workingDir; // 工作目录

private:
    void runCommandDirect(std::vector<std::string> command_line, const QString& program = QString(), QString workingdir = QString());
    void logCommand(const std::vector<std::string>& command_line, const QString& workingdir);

    QMap<QString, std::shared_ptr<reproc::process>> m_activeProcesses;
    QMutex m_processMutex;
    MrcImageProvider* m_imageProvider;
    SvgImageProvider* m_svgImageProvider;
    QString m_logFilePath;
};

#endif // BACKEND_H