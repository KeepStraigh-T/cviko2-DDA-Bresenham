#pragma once
#include <QtWidgets>
#include <cmath>
#include <QtNumeric>

struct ControlPoint
{
	QPoint point;																// main point
	QPoint handle;															// tangent point

	ControlPoint(QPoint p, QPoint h): point(p), handle(h) {
	};
};

struct Vertex
{
	int x, y, z;
	Vertex(int xx, int yy, int zz): x(xx), y(yy), z(zz) {};
};

class ViewerWidget: public QWidget
{
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	QVector <QPoint> vertices;
	QVector <QPoint> transformedVert;

	bool drawActivated = true;
	bool dragging = false;
	bool areaIsFilled = false;

	QPoint lastMousePos;
	QPoint drawLineBegin = QPoint(0, 0);

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	QVector <ControlPoint> curvePoints;

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);
	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawPolygon(QColor color, int algType, int interpType);
	void drawCircle(QColor color);
	void drawCirclePoints(int xc, int yc, int x, int y, QColor color);
	void drawCurve(QColor color, int curveType, int algType);

	// Additional functions for drawing
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() {	return drawLineBegin; }
	void setDrawActivated(bool state) { drawActivated = state; }
	bool getDrawActivated() { return drawActivated; }
	void setDragging(bool state) { dragging = state; }
	bool getDragging() { return dragging; }
	void setLastMousePos(QPoint pos) { lastMousePos = pos; }
	QPoint getLastMousePos() { return lastMousePos; }
	void swapPoints(QPoint& start, QPoint& end);

	// Transformations
	void rotate(double angle);
	void scale(double factorX, double factorY);
	void shear(double factorX);
	void symmetry();
	void translation(QPoint currentPos);

	// Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	void drawCircleBresenham(QPoint center, QPoint end, QColor color);
	QVector<QPoint> clippingLine();
	QVector<QPoint> clippingPolygon();
	void scanLine(const QVector <QPoint>& vertices, const QColor& color);
	void scanLineTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, std::vector<double>* zBuffer = nullptr, double z_const = std::numeric_limits<double>::lowest());
	void fillTopTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2, std::vector<double>* zBuffer = nullptr, double z_const = std::numeric_limits<double>::lowest());
	void fillBottomTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2, std::vector<double>* zBuffer = nullptr, double z_const = std::numeric_limits<double>::lowest());
	QColor nearestNeighbor(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2);
	QColor barycentricInterp(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2);
	void fergusovCubicCurve(QColor color, int algType);
	void bezierCurve(QColor color, int algType);
	void coonsoveCubicBSpline(QColor color, int algType);

	// Vertices functions
	void clearVertices();
	void push_backVertex(QPoint point);
	QPoint backVertex();
	QPoint firstVertex();
	qsizetype sizeVertex() { return vertices.size(); };
	void initTransfVert() { transformedVert = vertices; transformedVert.detach(); };
	void clear();

	// Filling functions
	void setFilled(bool state) { areaIsFilled = state; };
	bool getFilled() { return areaIsFilled; };

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };


public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};