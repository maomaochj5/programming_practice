#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "启动最小Qt应用程序";
    
    QWidget window;
    window.setWindowTitle("最小测试窗口");
    window.resize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(&window);
    QLabel *label = new QLabel("Hello, Qt!", &window);
    QPushButton *button = new QPushButton("测试按钮", &window);
    
    layout->addWidget(label);
    layout->addWidget(button);
    
    QObject::connect(button, &QPushButton::clicked, [&]() {
        qDebug() << "按钮点击";
        label->setText("按钮被点击了！");
    });
    
    window.show();
    
    qDebug() << "窗口已显示";
    
    return app.exec();
}
