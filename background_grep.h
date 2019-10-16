#ifndef BACKGROUND_JOB_H
#define BACKGROUND_JOB_H

#include <QObject>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <string>
#include <filesystem>
#include <iostream>
#include <dirent.h>
#include <QDir>
#include <QDirIterator>

class background_grep : public QObject
{
    Q_OBJECT
public:
    background_grep();
    ~background_grep();
    void stop();
    void start(std::string home_path);

    struct grepped_file{
        grepped_file();
        grepped_file(std::string, size_t, size_t, std::string, std::string, std::string);

        const static size_t appendix_size = 15;

        std::string file;
        size_t line;
        size_t pos;
        std::string before;
        std::string occurency;
        std::string after;

        QString to_string();
    };

    QString patch_result();
private:
    void search_in(std::string);

private:
    static const size_t thread_count = 2;
    std::unique_ptr<std::thread> tasks[thread_count];
    mutable std::mutex res;
    mutable std::mutex arg;
    std::condition_variable cv;
    std::atomic_bool cancel;
    std::atomic_bool quit;
    std::queue<std::string> path;
    std::vector<grepped_file> result;
    size_t peek = 0;
};

#endif // BACKGROUND_JOB_H
