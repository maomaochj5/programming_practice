#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

class SimpleMainWindow : public QMainWindow
{
public:
    SimpleMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("简化版 SmartPOS 测试");
        setMinimumSize(800, 600);
        
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        QLabel *titleLabel = new QLabel("智能超市收银系统", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);
        
        QPushButton *testButton = new QPushButton("测试按钮", this);
        layout->addWidget(testButton);
        
        connect(testButton, &QPushButton::clicked, [this]() {
            qDebug() << "测试按钮被点击";
        });
        
        qDebug() << "简化主窗口初始化完成";
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "启动简化测试应用";
    
    SimpleMainWindow window;
    window.show();
    
    return app.exec();
}

#include "simple_test.moc"
