#pragma once
#include <QtWidgets>
#include <cmath>
#include <QtNumeric>

#define DEBUG

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	QVector <QPoint> vertices;
	QVector <QPoint> transformedVert;

	bool drawActivated = true;

	QPoint lastMousePos;
	bool dragging = false;

	bool areaIsFilled = false;

	QPoint drawLineBegin = QPoint(0, 0);

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawPolygon(QColor color, int algType, int interpType);
	void drawCircle(QColor color);
	void drawCirclePoints(int xc, int yc, int x, int y, QColor color);

	//Clipping functions
	QVector<QPoint> clippingLine();
	QVector<QPoint> clippingPolygon();

	// Additional functions for drawing
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
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

// Filling functions
	void setFilled(bool state) { areaIsFilled = state; };
	bool getFilled() { return areaIsFilled; };
	void scanLine(const QVector <QPoint>& nodes, const QColor& color);
	QColor nearestNeighbor(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2);
	QColor barycentricInterp(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2);
	void scanLineTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType);
	private: void fillBottomTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2);
	private: void fillTopTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2);

public:
	// Vertices access functions
	void clearVertices();
	void push_backVertex(QPoint point);
	QPoint backVertex();
	QPoint firstVertex();
	qsizetype sizeVertex() { return vertices.size(); };
	void initTransfVert() { transformedVert = vertices; transformedVert.detach(); };

	// Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	void clear();

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	void drawCircleBresenham(QPoint center, QPoint end, QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};