#include "grep_job.h"

grep_job::grep_job(task_executor& te, QString path, QString occurency)
    : abstract_job (te),
      start_path(path),
      occurency(occurency)
{}

void grep_job::start() {
    run_subtask(std::shared_ptr<abstract_task>(new grep_task(*this, start_path)));
}

grep_job::grepped_file::grepped_file(QString file,
             size_t line,
             size_t pos,
             QString occurency):
    file(file),
    line(line),
    pos(pos),
    occurency(occurency){}

QString grep_job::grepped_file::to_string() {
    return file + "::"
            + QString::number(line) + ":"
            + QString::number(pos) + "\n...\n"
            + occurency + "\n...\n";

}

QString grep_job::patch_result() {
    std::unique_lock<std::mutex> lgres(res);
    QString ans;
    while(peek < result.size()){
        ans.append(result[peek++].to_string());
    }
    return ans;
}

grep_job::grep_task::grep_task(grep_job& job, QString path)
    :path(path), job(job){}

grep_job::grep_task::~grep_task() {}

void grep_job::grep_task::execute() {
    if (job.is_shutdown())
        return;
    {
        std::unique_lock<std::mutex> lgres(job.res);
        if (job.visited.find(path) != job.visited.end())
            return;
        job.visited.insert(path);
    }
    QDir dir(path);
    if (dir.exists()) {
        std::vector<std::shared_ptr<abstract_task>> subfolders;
        QDirIterator directories(path, QDir::Dirs
                                     | QDir::Files
                                     | QDir::NoSymLinks
                                     | QDir::NoDotAndDotDot
                                     | QDir::Readable,
                                 QDirIterator::Subdirectories);
        while(directories.hasNext()){
            directories.next();
            subfolders.push_back(std::shared_ptr<abstract_task>(new grep_task(job, directories.filePath())));
        }
        job.run_subtasks(subfolders);
    } else {
        std::vector<grepped_file> current_result;
        QFile file(path);
        file.open(QFile::ReadOnly);
        for (size_t line = 0; !file.atEnd(); ++line) {
            if(job.is_shutdown()) return;
            QByteArray bytes = file.readLine();
            int pos = 0;
            while (pos != -1) {
                if(job.is_shutdown()) return;
                pos = bytes.indexOf(job.occurency, pos);
                if (pos != -1) {
                    int occurency_from = pos - grepped_file::appendix_size < 0
                            ? 0
                            : pos - grepped_file::appendix_size;
                    int occurency_to = pos + grepped_file::appendix_size + job.occurency.length() > bytes.length()
                            ? bytes.length()
                            : pos + grepped_file::appendix_size + job.occurency.length();
                    current_result.push_back(grepped_file(
                                                 path,
                                                 line,
                                                 static_cast<size_t>(pos),
                                                 bytes.mid(occurency_from, occurency_to - occurency_from)
                                                 ));
                    pos += job.occurency.size();
                }
            }
        }
        {
            std::unique_lock<std::mutex> lg(job.res);
            for (auto i : current_result)
                job.result.push_back(i);
        }
    }
}
