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
                arg.unlock();

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

void background_grep::start(std::string home_path){
    stop();
    std::unique_lock<std::mutex> lgarg(arg);

    {
        std::unique_lock<std::mutex> lgres(res);
        peek = 0;
        result.clear();
    }

    path.push(home_path);
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
    after(after)
{

}
QString background_grep::grepped_file::to_string() {
    return QString::fromStdString(file + "::"
            + std::to_string(line) + ":"
            + std::to_string(pos) + "\n..."
            + before + " **" + occurency + "** " + after + "...\n");

}

QString background_grep::patch_result() {
    QString ans;
    while(peek < result.size()){
        ans.append(result[peek++].to_string());
    }
    return ans;
}

void background_grep::search_in(std::string argument) {
    std::unique_lock<std::mutex> lgres(res);
    std::unique_lock<std::mutex> lgarg(arg);
/*
    std::filesystem::path argument_path(argument);
    if (std::filesystem::is_directory(argument)) {
        std::cerr << argument;
        for(auto p : std::filesystem::directory_iterator(argument)){
            std::cerr << p.path();
            path.push(p.path());
        }
        result.push_back(grepped_file(argument, 1, 2, "directory ", argument, " end"));
    } else {
        result.push_back(grepped_file(argument, 3, 4, "file ", argument, " end"));
    }
*/
    QDir dir(QString::fromStdString(argument));
    if (dir.exists()) {
        QDirIterator directories(QString::fromStdString(argument),
                             QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while(directories.hasNext()){
            directories.next();
            if(directories.filePath().toStdString() == argument)
                continue;
            std::cerr << directories.filePath().toStdString() << std::endl;
            path.push(directories.filePath().toStdString());
        }
        result.push_back(grepped_file(argument, 1, 2, "directory ", argument, " end"));
    } else {
        result.push_back(grepped_file(argument, 3, 4, "file ", argument, " end"));
    }
}
