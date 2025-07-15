#include <QApplication>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Starting minimal application...";
    
    QMessageBox::information(nullptr, "Test", "Application started successfully!");
    
    return 0;
}
