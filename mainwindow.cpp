#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {

    ui->setupUi(this);

    QVector<QPair<double, double>> points = readPointsFromFile("points.txt");
    if (points.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить точки из файла.");
        return;
    }

    QVector<QPair<double, double>> signal = simplifyLinearSegments(points);
    for (QPair<double, double> point : signal) {
        this->x.push_back(point.first);
        this->y.push_back(point.second);
    }

    QVector<QPair<double, double>> inflectionPoints = findInflectionPoints(signal);

    qDebug() << "Points:";
        for (const auto& point : points) {
            qDebug() << QString("(%1, %2)").arg(point.first).arg(point.second);
        }

        qDebug() << "Signal:";
        for (const auto& point : signal) {
            qDebug() << QString("(%1, %2)").arg(point.first).arg(point.second);
        }

        qDebug() << "Inflection Points:";
        for (const auto& point : inflectionPoints) {
            qDebug() << QString("(%1, %2)").arg(point.first).arg(point.second);
        }
        ui->setupUi(this);

        // Initialize min and max values
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        // Iterate through the points to find min and max
        for (const auto& point : signal) {
            double x = point.first;
            double y = point.second;

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        ui->widget->xAxis->setRange(minX, maxX);
        ui->widget->yAxis->setRange(minY, maxY);

        ui->widget->addGraph();
        ui->widget->graph(0)->addData(x, y);

        QCPGraph *graph = ui->widget->addGraph();

        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10)); // Circle size = 10
        graph->setLineStyle(QCPGraph::lsNone); // No connecting line

        for (const auto& inflectionPoint : inflectionPoints) {
            double inflectionX = inflectionPoint.first;
            double inflectionY = inflectionPoint.second;

            graph->addData(inflectionX, inflectionY);

            QCPItemText *textLabel = new QCPItemText(ui->widget);
            textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
            textLabel->position->setCoords(inflectionX, inflectionY);
            textLabel->setText(QString("(%1, %2)").arg(inflectionX).arg(inflectionY));
            textLabel->setFont(QFont("sans", 8));
            textLabel->setColor(Qt::black);
        }

        ui->widget->replot();
}

QVector<QPair<double, double>> MainWindow::readPointsFromFile(const QString &fileName) {
    QVector<QPair<double, double>> points;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл:" << fileName;
        return points; // Возвращаем пустой QVector
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList values = line.split(',');

        if (values.size() == 2) {
            bool okX, okY;
            double x = values[0].toDouble(&okX);
            double y = values[1].toDouble(&okY);

            if (okX && okY) {
                points.append(QPair<double, double>(x, y));
            } else {
                qDebug() << "Ошибка преобразования значений в строке:" << line;
            }
        }
    }

    file.close();
    return points;
}

MainWindow::~MainWindow() {
    delete ui;
}

QVector<QPair<double, double>> MainWindow::simplifyLinearSegments(const QVector<QPair<double, double>>& points) {
    if (points.size() < 3) {
        return points;
    }

    QVector<QPair<double, double>> simplifiedPoints;
    simplifiedPoints.append(points[0]);

    QVector<double> firstDerivative;
    for (int i = 0; i < points.size() - 1; ++i) {
        double deltaX = points[i + 1].first - points[i].first;
        double deltaY = points[i + 1].second - points[i].second;
        if (std::abs(deltaX) < 1e-9) {
            firstDerivative.append(std::numeric_limits<double>::infinity());
        } else {
            firstDerivative.append(deltaY / deltaX);
        }
    }

    int i = 0;
    while (i < firstDerivative.size()) {
        if (i < firstDerivative.size() - 1 && std::abs(firstDerivative[i] - firstDerivative[i + 1]) < 1e-9) {
            while (i < firstDerivative.size() - 1 && std::abs(firstDerivative[i] - firstDerivative[i + 1]) < 1e-9) {
                ++i;
            }
            simplifiedPoints.append(points[i + 1]);
        } else {
            simplifiedPoints.append(points[i + 1]);
        }
        ++i;
    }

    return simplifiedPoints;
}


QVector<QPair<double, double>> MainWindow::findInflectionPoints(const QVector<QPair<double, double>>& points) {
    // If there are fewer than 3 points, no inflection point is possible
    if (points.size() < 3) {
        return {};
    }

    QVector<double> firstDerivative;
    QVector<double> secondDerivative;

    // Step 1: Compute the first derivative (slopes)
    for (int i = 0; i < points.size() - 1; ++i) {
        double dx = points[i + 1].first - points[i].first;
        double dy = points[i + 1].second - points[i].second;

        if (std::abs(dx) < 1e-9) {
            // Vertical line: assign a large value for the slope
            firstDerivative.append(std::numeric_limits<double>::infinity());
        } else {
            firstDerivative.append(dy / dx);
        }
    }

    // Step 2: Compute the second derivative (differences in slopes)
    for (int i = 0; i < firstDerivative.size() - 1; ++i) {
        secondDerivative.append(firstDerivative[i + 1] - firstDerivative[i]);
    }

    qDebug() << "First Derivative:";
    for (int i = 0; i < firstDerivative.size(); ++i) {
        qDebug() << QString("f'(%1) = %2").arg(points[i].first).arg(firstDerivative[i]);
    }

    qDebug() << "Second Derivative:";
    for (int i = 0; i < secondDerivative.size(); ++i) {
        qDebug() << QString("f''(%1) = %2").arg(points[i].first).arg(secondDerivative[i]);
    }


    QVector<QPair<double, double>> inflectionPoints;

    // Step 3: Detect inflection points
    for (int i = 0; i < secondDerivative.size() - 1; ++i) {
        // Check if second derivative changes sign
        if (secondDerivative[i] * secondDerivative[i + 1] < 0) {
            // Ensure first derivative does not change sign (not an extremum)
            if (firstDerivative[i] * firstDerivative[i + 1] > 0) {
                // Inflection point detected; approximate position
                inflectionPoints.append(points[i + 1]); // Use the midpoint as an approximation
            }
        }
    }

    return inflectionPoints;
}
