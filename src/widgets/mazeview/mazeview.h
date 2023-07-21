#ifndef MAZESCENE_H
#define MAZESCENE_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QThread>
#include <QMutex>


typedef QVector<uint8_t> matrixRow;
typedef QVector<matrixRow> matrixTable;


struct Point
{
    Point()
        : r(0), c(0) {}
    Point(int _r, int _c)
        : r(_r), c(_c) {}
    int r;
    int c;
};

struct Node
{
    Node(int _r, int _c, QVector<Point> _points)
        :r(_r), c(_c), points(_points) {}
    int r;
    int c;
    QVector<Point> points;
};


class SearhPathThread: public QThread
{
    Q_OBJECT
public:
    SearhPathThread(QObject *parent = Q_NULLPTR);

protected:
    void run() override;

public:
    void setMatrix(matrixTable *mat, int mWidth, int mHeight);
    void setPoints(Point src, Point dest);
    void stop();

signals:
    void resultReady(QVector<Point> vec);

private:
    bool isRunner;
    matrixTable *mat;
    int matWidth;
    int matHeight;
    Point src;
    Point dest;
    QMutex mMutex;
};


class MazeView: public QGraphicsView
{
    Q_OBJECT

public:
    MazeView(QWidget *parent = Q_NULLPTR);
    ~MazeView();

    void generateMatrix(int mWidth, int mHeight);
    void zoom(qreal factor);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void deleteItemsFromGroup(QGraphicsItemGroup *group);

private Q_SLOTS:
    void drawPath(QVector<Point> vec);

private:
    QGraphicsScene *scene;
    QGraphicsItemGroup *backgroundScene;
    QGraphicsItemGroup *pathScene;
    QGraphicsItemGroup *pointsScene;
    SearhPathThread *searchPathThread;
    matrixTable matrix;
    QSize matrixSize;
    qreal zoomFactor;
    bool isChangedZoom;
    QPoint pointStart;
    QPoint pointEnd;
};


#endif // MAZESCENE_H
