#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <string>
#include <thread>
#include <mutex>
#include <deque>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
	QTcpServer ser,liveStrXhrSer;
	mutex mt0,mt;
	deque<QTcpSocket *> pending;
	string curr,dres;
private slots:
	void on_tE1_textChanged();
	
	void on_xhrPort_valueChanged(double arg1);
	
	void on_sitePort_valueChanged(double arg1);
	
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
