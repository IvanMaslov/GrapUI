#ifndef BACKGROUND_JOB_H
#define BACKGROUND_JOB_H

#include <QObject>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <set>
#include <QDir>
#include <QDirIterator>

class task_processor : public QObject
{
    Q_OBJECT
public:
    task_processor();
    ~task_processor();
    void stop();
    void start(QString home_path, QString grep_string);

    struct grepped_file{
        grepped_file();
        grepped_file(QString, size_t, size_t, QString);

        const static QByteArray::size_type appendix_size = 20;

        QString file;
        size_t line;
        size_t pos;
        QString occurency;

        QString to_string();
    };

    QString patch_result();
private:
    void search_in(QString);

private:
    static const size_t thread_count = 3;
    std::unique_ptr<std::thread> executor[thread_count];
    mutable std::mutex res;
    mutable std::mutex arg;
    std::condition_variable cv;
    std::atomic_bool cancel;
    std::atomic_bool quit;
    QString grep_string;
    std::queue<QString> path;
    mutable std::set<QString> visited;
    std::vector<grepped_file> result;
    size_t peek = 0;
};

#endif // BACKGROUND_JOB_H
