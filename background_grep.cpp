#include "background_grep.h"

background_grep::background_grep() :
    cancel(false),
    quit(false)
{
    for(size_t i = 0; i < thread_count; ++i){
        tasks[i] = std::move(
                std::unique_ptr<std::thread>(
                        new std::thread([this] {
            while(true) {
                std::unique_lock<std::mutex> lg(arg);
                cv.wait(lg, [this] {
                    return !path.empty() || quit;
                });
                if (quit)
                    break;
                cancel.store(false);

                QString argument = path.front();
                path.pop();
                lg.unlock();

                search_in(argument);
            }
        })));
    }
}

background_grep::~background_grep() {
    cancel.store(true);
    quit.store(true);
    cv.notify_all();
    for(size_t i = 0; i < thread_count; ++i)
        tasks[i]->join();
}

void background_grep::stop(){
    std::unique_lock<std::mutex> lg(arg);

    cancel.store(true);
    while(!path.empty())
        path.pop();

    cv.notify_all();
}

void background_grep::start(QString home_path, QString grep_string){
    stop();

    {
        std::unique_lock<std::mutex> lgres(res);
        peek = 0;
        result.clear();
    }

    {
        std::unique_lock<std::mutex> lgarg(arg);
        this->grep_string = grep_string;
        visited.clear();
        path.push(home_path);
    }

    cv.notify_all();
}

background_grep::grepped_file::grepped_file() {}
background_grep::grepped_file::grepped_file(QString file,
             size_t line,
             size_t pos,
             QString occurency):
    file(file),
    line(line),
    pos(pos),
    occurency(occurency){}

QString background_grep::grepped_file::to_string() {
    return file + "::"
            + QString::number(line) + ":"
            + QString::number(pos) + "\n...\n"
            + occurency + "\n...\n";

}

QString background_grep::patch_result() {
    std::unique_lock<std::mutex> lgres(res);
    QString ans;
    while(peek < result.size()){
        ans.append(result[peek++].to_string());
    }
    return ans;
}

void background_grep::search_in(QString argument) {
    std::unique_lock<std::mutex> lgres(res);
    std::unique_lock<std::mutex> lgarg(arg);

    if (visited.find(argument) != visited.end())
        return;
    visited.insert(argument);

    QDir dir(argument);
    if (dir.exists()) {
        QDirIterator directories(argument,
                             QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::Subdirectories);
        while(directories.hasNext()){
            directories.next();
            path.push(directories.filePath());
        }
    } else {
        QString occurency = grep_string;
        lgarg.unlock();
        lgres.unlock();
        std::vector<grepped_file> current_result;
        QFile file(argument);
        file.open(QFile::ReadOnly);
        for (size_t line = 0; !file.atEnd(); ++line) {
            if(cancel.load()) return;
            QByteArray bytes = file.readLine();
            int pos = 0;
            while (pos != -1) {
                if(cancel.load()) return;
                pos = bytes.indexOf(occurency, pos);
                if (pos != -1) {
                    int occurency_from = pos - grepped_file::appendix_size < 0
                            ? 0
                            : pos - grepped_file::appendix_size;
                    int occurency_to = pos + grepped_file::appendix_size + occurency.length() > bytes.length()
                            ? bytes.length()
                            : pos + grepped_file::appendix_size + occurency.length();
                    current_result.push_back(grepped_file(
                                                 argument,
                                                 line,
                                                 static_cast<size_t>(pos),
                                                 bytes.mid(occurency_from, occurency_to - occurency_from)
                                                 ));
                    pos += occurency.size();
                }
            }
        }
        lgres.lock();
        for (auto i : current_result)
            result.push_back(i);
    }
}
