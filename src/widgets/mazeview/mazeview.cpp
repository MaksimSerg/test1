#include "mazeview.h"

//#include <time.h>

#include <QGraphicsItemGroup>
#include <QPainter>
#include <QQueue>
#include <QWheelEvent>
#include <QMessageBox>
//#include <QDebug>


#define MIN_ZOOM_FACTOR     0.1
#define MAX_ZOOM_FACTOR     100
#define MATRIX_MIN_SIDE     10
#define MATRIX_MAX_SIDE     150
#define MATRIX_DENSITY      0.3


SearhPathThread::SearhPathThread(QObject *parent):
    QThread(parent),
    isRunner(false), mat(nullptr), matWidth(0), matHeight(0), src(-1,-1), dest(-1,-1), mMutex()
{}

void SearhPathThread::setMatrix(matrixTable *rMat, int mWidth, int mHeight)
{
    mat = rMat;
    matWidth = mWidth;
    matHeight = mHeight;
    src = Point(-1,-1);
    dest = Point(-1,-1);
}

void SearhPathThread::setPoints(Point rSrc, Point rDest)
{
    src = rSrc;
    dest = rDest;
}

void SearhPathThread::stop()
{
    QMutexLocker ml(&mMutex);
    isRunner = false;
    ml.unlock();
    while (isRunning());
}

void SearhPathThread::run()
{
    if (!(matWidth && src.r >-1 && src.r < matHeight && src.c >-1 && src.c < matWidth)) return;
    if (!mat || (*mat)[src.r][src.c] == 1) return;

    isRunner = true;

    int r = src.r;
    int c = src.c;

    QVector<QVector<bool>> notVisited;
    notVisited.resize(matHeight);
    notVisited.fill(QVector<bool>(matWidth));
    for (int r=0; r<matHeight; ++r)
    {
        for (int c=0; c<matWidth; ++c)
        {
            notVisited[r][c] = true;
        }
    }
    notVisited[r][c] = false;

    QVector<Point> startNode;
    startNode.push_back(Point(r, c));

    QQueue<Node> q;
    q.push_back({r, c, startNode});

    int row[] = { -1, 0, 0, 1 };
    int col[] = { 0, -1, 1, 0 };

    // run
//    clock_t tStart = clock();
    while (isRunner && !q.empty())
    {
        // node - current cell
        Node node = q.front();
        q.pop_front();

        r = node.r;
        c = node.c;

        // search
        if (r == dest.r && c == dest.c)
        {
            emit resultReady(node.points);
//            qDebug() << "find, sec" << (double)(clock() - tStart)/CLOCKS_PER_SEC;
            return;
        }

        for (int k = 0; k < 4; k++)
        {
            if (!isRunner) return;
            int nr = r + row[k];
            int nc = c + col[k];

            if (nr >-1 && nr < matHeight && nc >-1 && nc < matWidth && (*mat)[nr][nc]==0 && notVisited[nr][nc])
            {
                notVisited[nr][nc] = false;
                QVector<Point> startNodes = node.points;
                startNodes.push_back(Point(nr, nc));
                q.push_back({nr, nc, startNodes});
            }
        }
    }

    if (isRunner)
    {
        // not found
        emit resultReady({});
    }
}


MazeView::MazeView(QWidget *parent)
    : QGraphicsView(parent),
      scene(nullptr),
      backgroundScene(nullptr), pathScene(nullptr), pointsScene(nullptr),
      searchPathThread(nullptr),
      matrix(), matrixSize(0,0),
      zoomFactor(1), isChangedZoom(false),
      pointStart(-1,-1), pointEnd(-1,-1)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    setCursor(Qt::CrossCursor);

    backgroundScene = new QGraphicsItemGroup();
    scene->addItem(backgroundScene);

    pointsScene = new QGraphicsItemGroup();
    scene->addItem(pointsScene);

    pathScene = new QGraphicsItemGroup();
    scene->addItem(pathScene);

    setMouseTracking(true);

    searchPathThread = new SearhPathThread(this);
    connect(searchPathThread, SIGNAL(resultReady(QVector<Point>)), this, SLOT(drawPath(QVector<Point>)));
}

MazeView::~MazeView()
{
    if (searchPathThread)
    {
        if (searchPathThread->isRunning())
        {
            searchPathThread->stop();
            while(searchPathThread->isRunning());
        }
        searchPathThread->deleteLater();
    }
}

void MazeView::drawPath(QVector<Point> vec)
{
    deleteItemsFromGroup(pathScene);

    QPen pen(Qt::NoPen);
    QBrush brush(Qt::red);

    for (int i=1; i<vec.size(); ++i)
    {
        Point pnt = vec[i];
        QRect rect(pnt.c+0.5, pnt.r+0.5, 1, 1);
        QGraphicsRectItem *pPoint = scene->addRect(rect, pen, brush);
        pathScene->addToGroup(pPoint);
    }
}

void MazeView::generateMatrix(int mWidth, int mHeight)
{
    if (searchPathThread)
    {
        searchPathThread->stop();
    }

    if (mWidth < MATRIX_MIN_SIDE  || mWidth > MATRIX_MAX_SIDE ||
        mHeight < MATRIX_MIN_SIDE || mHeight > MATRIX_MAX_SIDE
    )
    {
        QMessageBox::warning(this,
            tr("Error"),
            tr("Еhe size of the matrix should be in the range from %1 to %2")
                .arg(MATRIX_MIN_SIDE).arg(MATRIX_MAX_SIDE),
            QMessageBox::StandardButton::Ok,QMessageBox::StandardButton::Ok
        );
        return;
    }

    if (!mWidth || !mHeight) return;

    deleteItemsFromGroup(backgroundScene);
    deleteItemsFromGroup(pointsScene);
    deleteItemsFromGroup(pathScene);

    matrix.clear();
    matrix.resize(mHeight);
    matrix.fill(matrixRow(mWidth));

    pointStart = {-1,-1};
    pointEnd = {-1,-1};

    QPixmap image(mWidth, mHeight);
    QPainter *painter = new QPainter(&image);
    if (painter)
        {
        painter->fillRect(0,0, mWidth,mHeight, Qt::white);
        painter->setPen(Qt::black);

        for(int r=0; r<mHeight; ++r)
        {
            for(int c=0; c<mWidth; ++c)
            {
                uint8_t state = (qrand() % 10) / (10*(1-MATRIX_DENSITY)); // create random matrix. wall, % - MATRIX_DENSITY
                matrix[r][c] = state;
                if (state)
                {
                    painter->drawPoint(c,r);
                }
            }
        }

        delete painter;
    }

    if (!scene || !backgroundScene) return;

    QGraphicsPixmapItem* img = scene->addPixmap( image );
    img->setPos(0, 0);
    backgroundScene->addToGroup(img);

    scene->setSceneRect(0,0, mWidth, mHeight);

    matrixSize = QSize(mWidth, mHeight);

    qreal mx = ((qreal)width()-2)  / (qreal)mWidth;
    qreal my = ((qreal)height()-2) / (qreal)mHeight;
    zoom(qMin(mx,my));

    if (searchPathThread)
    {
        searchPathThread->setMatrix(&matrix, mWidth, mHeight);
    }
}

void MazeView::zoom(qreal factor)
{
    qreal scaleFactor = factor/zoomFactor;

    if (scaleFactor<MIN_ZOOM_FACTOR || scaleFactor>MAX_ZOOM_FACTOR) return;

    scale(scaleFactor, scaleFactor);
    zoomFactor = factor;
}

void MazeView::resizeEvent(QResizeEvent *event)
{
    if (!isChangedZoom && matrixSize.width())
    {
        qreal mx = ((qreal)width()-2)  / (qreal)matrixSize.width();
        qreal my = ((qreal)height()-2) / (qreal)matrixSize.height();
        zoom(qMin(mx,my));
    }

    QGraphicsView::resizeEvent(event);
}

void MazeView::wheelEvent(QWheelEvent *event)
{
    isChangedZoom = true;
    qreal scaleFactor = 1.1;
    const ViewportAnchor anchor = transformationAnchor();
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if(event->delta() > 0)
    {
        zoom(zoomFactor+scaleFactor);
    }
    else
    {
        zoom(zoomFactor-scaleFactor);
    }

    setTransformationAnchor(anchor);

    event->accept();
}

void MazeView::mousePressEvent(QMouseEvent *event)
{
    QPointF pt = mapToScene(event->pos());
    int x = pt.x();
    int y = pt.y();

    if (y>-1 && y<matrixSize.height() && x>-1 && x<matrixSize.width() && matrix[y][x] == 0)
    {
        if (pointsScene)
        {
            QRect rect(x+0.5, y+0.5, 1, 1);

            if (pointStart.x() == -1)
            {
                pointStart = {x,y};
                QGraphicsRectItem *pnt = scene->addRect(rect, QPen(Qt::NoPen), QBrush(Qt::blue));
                pointsScene->addToGroup(pnt);
            }
            else if (pointEnd.x() == -1)
            {
                pointEnd = {x,y};
                QGraphicsRectItem *pnt = scene->addRect(rect, QPen(Qt::NoPen), QBrush(Qt::green));
                pointsScene->addToGroup(pnt);

                if (searchPathThread)
                {
                    Point p1(pointStart.y(), pointStart.x());
                    Point p2(y, x);
                    searchPathThread->stop();
                    searchPathThread->setPoints(p1, p2);
                    searchPathThread->start();
                }
            }
            else
            {
                pointStart = {-1,-1};
                pointEnd = {-1,-1};
                deleteItemsFromGroup(pointsScene);
                deleteItemsFromGroup(pathScene);
            }
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void MazeView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pt = mapToScene(event->pos());
    int x = pt.x();
    int y = pt.y();

    if (y>-1 && y<matrixSize.height() && x>-1 && x<matrixSize.width() && matrix[y][x] == 0)
    {
        if (searchPathThread)
        {
            if (pointStart.x() != -1 && pointEnd.x() == -1)
            {
                Point p1(pointStart.y(), pointStart.x());
                Point p2(y,x);

                searchPathThread->stop();
                searchPathThread->setPoints(p1, p2);
                searchPathThread->start();
            }
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void MazeView::deleteItemsFromGroup(QGraphicsItemGroup *group)
{
    if (!scene || !group) return;

    for( QGraphicsItem *item: scene->items(group->boundingRect())) {
       if(item->group() == group ) {
          delete item;
       }
    }
}
