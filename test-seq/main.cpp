#include <QCoreApplication>
#include <seqlogger.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    SEQAPPNAME("libcurl-test");
    SEQLOGINFO("Test log from libcurl-test");

    return 0;
    //return a.exec();
}
