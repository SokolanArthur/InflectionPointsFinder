#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMessageBox>
#include <QFile>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    Ui::MainWindow *ui;
    QVector<QPair<double, double>> simplifyLinearSegments(const QVector<QPair<double, double>>& points);
    QVector<QPair<double, double>> findInflectionPoints(const QVector<QPair<double, double>>& points);
    QVector<QPair<double, double>> readPointsFromFile(const QString &fileName);

    QVector<double> x, y;
};

#endif // MAINWINDOW_H
