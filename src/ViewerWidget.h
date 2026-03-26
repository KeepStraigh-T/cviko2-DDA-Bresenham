#pragma once
#include <QtWidgets>
#include <cmath>
#include <QtNumeric>

class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	QVector <QPoint> vertices;
	QVector <QPoint> transformedVert;

	bool drawActivated = false;

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
	void drawPolygon(QColor color, int algType);
	void drawCircle(QPoint center, QPoint end, QColor color);
	void drawCirclePoints(int xc, int yc, int x, int y, QColor color);

	//Clipping functions
	QVector<QPoint> clippingLine();
	QVector<QPoint> clippingPolygon();

	// Additional functions for drawing
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawActivated(bool state) { drawActivated = state; }
	bool getDrawActivated() { return drawActivated; }
	void swapPoints(QPoint& start, QPoint& end);

//Transformations
	void rotate(double angle);
	void scale(double factorX, double factorY, QColor color, int algType);
	void shear(double factorX, QColor color, int algType);
	void symmetry(QColor color, int algType);


	void clearVertices();
	void push_backVertex(QPoint point);
	QPoint backVertex();
	QPoint firstVertex();
	qsizetype sizeVertex();
	void initTransfVert() { transformedVert = vertices; transformedVert.detach(); };


	//Get/Set functions
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