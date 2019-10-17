#include "background_grep.h"

background_grep::background_grep() :
    cancel(false),
    quit(false)
{
    for(size_t i = 0; i < thread_count; ++i){
        tasks[i] = std::move(
                std::unique_ptr<std::thread>(
                        new std::thread([this] {
            for (;;) {
                std::unique_lock<std::mutex> lg(arg);
                cv.wait(lg, [this] {
                    return !path.empty() || quit;
                });
                if (quit)
                    break;
                cancel.store(false);

                std::string argument = path.front();
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

void background_grep::start(std::string home_path, QString grep_string){
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
}

background_grep::grepped_file::grepped_file() {}
background_grep::grepped_file::grepped_file(std::string file,
             size_t line,
             size_t pos,
             std::string before,
             std::string occurency,
             std::string after):
    file(file),
    line(line),
    pos(pos),
    before(before),
    occurency(occurency),
    after(after){}

QString background_grep::grepped_file::to_string() {
    return QString::fromStdString(file + "::"
            + std::to_string(line) + ":"
            + std::to_string(pos) + "\n..."
            + before + " " + occurency + " " + after + "...\n");

}

QString background_grep::patch_result() {
    std::unique_lock<std::mutex> lgres(res);
    QString ans;
    while(peek < result.size()){
        ans.append(result[peek++].to_string());
    }
    return ans;
}

void background_grep::search_in(std::string argument) {
    std::unique_lock<std::mutex> lgres(res);
    std::unique_lock<std::mutex> lgarg(arg);

    if (visited.find(argument) != visited.end())
        return;
    visited.insert(argument);
    QDir dir(QString::fromStdString(argument));
    if (dir.exists()) {
        QDirIterator directories(QString::fromStdString(argument),
                             QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable, QDirIterator::Subdirectories);
        while(directories.hasNext()){
            directories.next();
            path.push(directories.filePath().toStdString());
        }
    } else {
        QString occurency = grep_string;
        lgarg.unlock();
        lgres.unlock();
        std::vector<grepped_file> current_result;
        // current_result.push_back(grepped_file(argument, 0, 0, "file:", argument, " \n"));
        QFile file(QString::fromStdString(argument));
        file.open(QFile::ReadOnly);
        for (size_t line = 0; !file.atEnd(); ++line) {
            if(cancel.load()) return;
            QByteArray bytes = file.readLine();
            QString before;
            QString after;
            int pos = 0;
            while (pos != -1) {
                if(cancel.load()) return;
                pos = bytes.indexOf(occurency, pos);
                if (pos != -1) {
                    size_t before_begin = pos;
                    size_t before_size = 0;
                    if (before_begin >= grepped_file::appendix_size){
                        before_size = grepped_file::appendix_size;
                    }
                    else {
                        before_size = pos;
                    }
                    before_begin -= grepped_file::appendix_size;

                    size_t after_begin = pos + occurency.size();
                    size_t after_size = grepped_file::appendix_size;
                    if (after_begin + after_size > bytes.length() ){
                        after_size = bytes.length() - after_begin;
                    }

                    current_result.push_back(grepped_file(
                                                 argument,
                                                 line,
                                                 pos,
                                                 bytes.mid(before_begin, before_size).toStdString(),
                                                 occurency.toStdString(),
                                                 bytes.mid(after_begin, after_size).toStdString()
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
