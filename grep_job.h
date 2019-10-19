#ifndef GREP_JOB_H
#define GREP_JOB_H

#include <abstract_job.h>

#include <set>
#include <QDir>
#include <QDirIterator>

class grep_job : public abstract_job
{
public:
    class grep_task : public abstract_task
    {
    private:
        QString path;
        grep_job& job;
    public:
        grep_task(grep_job&, QString);
        ~grep_task() override;
        void execute() override;
    };

    struct grepped_file{
        grepped_file(QString, size_t, size_t, QString);

        const static QByteArray::size_type appendix_size = 20;

        QString file;
        size_t line;
        size_t pos;
        QString occurency;

        QString to_string();
    };


    grep_job(task_executor&, QString path, QString occurency);
    void start() override;


    QString patch_result();
private:
    QString start_path;
    const QString occurency;
    std::set<QString> visited;
    std::vector<grepped_file> result;
    size_t peek = 0;
};

#endif // GREP_JOB_H
