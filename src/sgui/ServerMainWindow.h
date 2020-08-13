#ifndef SERVERMAINWINDOW_H
#define SERVERMAINWINDOW_H

#include <QMainWindow>

#include <QWidget>
#include <QPushButton>
#include <QListWidget>

class QLabel;

class LockerButton : public QPushButton
{
    Q_OBJECT
public:
    /// @brief constructor
    explicit LockerButton(QWidget* parent = nullptr);

    /// @brief SetImageLabel
    void SetImageLabel(const QPixmap &pixmap);

    /// @brief SetTextLabel
    void SetTextLabel(QString text);

    /// @brief GetImageHandle
    QLabel* GetImageHandle();

    /// @brief GetImageHandle
    QLabel* GetTextHandle();

private:
    // button icon
    QLabel* m_imageLabel;
    // button text
    QLabel* m_textLabel;
};

class MainServerWindowWidget : public QWidget
{
    Q_OBJECT
public:
    /// @brief constructor
    explicit MainServerWindowWidget(QWidget* parent = nullptr);

private:
    void SetUpUI();
    LockerButton* _monButton;
    QListWidget * _monWidget;
    LockerButton* _winButton;
    QListWidget*  _winWidget;
    LockerButton* _partButton;
    QListWidget*  _partWidget;

    quint8 _monSwitch;
    quint8 _winSwtich;
    quint8 _partSwitch;
};
QT_BEGIN_NAMESPACE
namespace Ui { class ServerMainWindow; }
QT_END_NAMESPACE

class ServerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ServerMainWindow(QWidget *parent = nullptr);
    ~ServerMainWindow();

    void initMenu();
    void initImage();
    void initStartedServer();

private:
    Ui::ServerMainWindow *ui;

private slots:
    void startMonitorCapture();
    void startWindowCapture();
    void startPartialCapture();
    void newServer();
    void closeThis();
    void listDetails();
    void listIcons();
};
#endif // SERVERMAINWINDOW_H
