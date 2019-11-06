#include "grep_job.h"

grep_job::grep_job(task_executor& te, QString path, QString occurency)
    : abstract_job (te),
      occurency(occurency),
      start_path(path){}

void grep_job::start() {
    static_assert (!__SECURE_IMPLEMENT_METHOD_WARNINGS__, "WARNING: Not implemented method");
}

void grep_job::start(std::shared_ptr<grep_job> job) {
    qDebug() << "START GREP: " << job->start_path;
    if (job->occurency.isEmpty()){
        job->errorlog = "ERROR: search for an empty string";
        qDebug() << "ERROR: search for an empty string)";
        return;
    }
    if (!QDir(job->start_path).exists()) {
        if (QFile::exists(job->start_path)) {
            job->run_subtask(std::make_unique<grep_task>(job, job->start_path));
        }
        else {
            job->errorlog = "ERROR: No such file or directory(" + job->start_path + ")";
            qDebug() << "ERROR: No such file or directory(" + job->start_path + ")";
        }
        return;
    }
    std::vector<std::unique_ptr<abstract_task>> subfiles;
    QDirIterator file(job->start_path, QDir::Files
                                 | QDir::NoSymLinks
                                 | QDir::NoDotAndDotDot
                                 | QDir::Readable,
                             QDirIterator::Subdirectories);
    while(file.hasNext()){
        if(job->is_shutdown()) return;
        file.next();
        subfiles.push_back(std::move(std::make_unique<grep_task>(job, file.filePath())));
    }
    job->run_subtasks(std::move(subfiles));
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
    std::lock_guard<std::mutex> lg(res);
    QString ans;
    while(peek < result.size() && ans.size() < patch_limit){
        ans.append(result[peek++].to_string());
    }
    if(!errorlog.isEmpty() && peek == result.size()){
        ans += errorlog;
        errorlog.clear();
    }
    return ans;
}

void grep_job::append_result(const std::vector<grepped_file>& current_result) {
    if(result.size() + current_result.size() > result_limit || is_shutdown()) {
        stop();
        qDebug() << "ERROR: LARGE ANSWER";
        errorlog += "ERROR: LARGE ANSWER";
        throw std::runtime_error("Too large result");
    }
    for (auto i : current_result)
        result.push_back(i);
}

grep_job::grep_task::grep_task(std::shared_ptr<grep_job> job, QString path)
    :path(path), job(job){}

grep_job::grep_task::~grep_task() {}

void grep_job::grep_task::execute() {
    if (!QFile::exists(path)) return;
    std::vector<grepped_file> current_result;
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    for (size_t line = 0; !file.atEnd(); ++line) {
        if(job->is_shutdown()) return;
        QByteArray bytes = file.readLine();
        int pos = 0;
        while (pos != -1) {
            if(job->is_shutdown()) return;
            pos = bytes.indexOf(job->occurency, pos);
            if (pos != -1) {
                QByteArray::size_type occurency_from = pos < grepped_file::appendix_size
                        ? 0
                        : pos - grepped_file::appendix_size;
                QByteArray::size_type occurency_to = pos + grepped_file::appendix_size + job->occurency.length() > bytes.length()
                        ? bytes.length()
                        : pos + grepped_file::appendix_size + job->occurency.length();
                current_result.push_back(grepped_file(
                                             path,
                                             line,
                                             static_cast<size_t>(pos),
                                             bytes.mid(occurency_from, occurency_to - occurency_from)
                                             ));
                pos += job->occurency.size();
            }
        }
    }
    {
        std::lock_guard<std::mutex> lg(job->res);
        if(job->is_shutdown()) return;
        job->append_result(current_result);
    }
}
