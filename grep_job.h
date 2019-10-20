#ifndef GREP_JOB_H
#define GREP_JOB_H

#include <abstract_job.h>

#include <set>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

class grep_job : public abstract_job
{
public:
    class grep_task : public abstract_task
    {
    private:
        QString path;
        std::shared_ptr<grep_job> job;
    public:
        grep_task(std::shared_ptr<grep_job>, QString);
        ~grep_task() override;
        void execute() override;
    };

    struct grepped_file{
        grepped_file(QString, size_t, size_t, QString);

        const static QByteArray::size_type appendix_size = 10;

        QString file;
        size_t line;
        size_t pos;
        QString occurency;

        QString to_string();
    };

    grep_job(task_executor&, QString path, QString occurency);
    void start() override;
    static void start(std::shared_ptr<grep_job>);

    QString patch_result();

protected:
    void append_result(const std::vector<grepped_file>&);
private:
    static const size_t result_limit = 100000;
    static const QByteArray::size_type patch_limit = 100000;

    const QString occurency;
    const QString start_path;

    std::set<QString> visited;
    std::vector<grepped_file> result;
    size_t peek = 0;

};

#endif // GREP_JOB_H
